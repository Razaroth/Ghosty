#include "tray.h"

#include "controller.h"
#include "icon.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSlider>
#include <QSystemTrayIcon>
#include <QTimer>

namespace {

QSlider *makeSlider(int orientation)
{
    auto *s = new QSlider(static_cast<Qt::Orientation>(orientation));
    s->setRange(0, 100);
    s->setValue(100);
    return s;
}

QLabel *pctLabel()
{
    auto *l = new QLabel(QStringLiteral("100%"));
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    l->setMinimumWidth(44);
    return l;
}

// The popup panel. KWin on Wayland ignores client-side positioning, so a
// QMenu/Qt::Popup surfaced from a windowless tray app never maps; a frameless
// toplevel does (KWin centers it). Hide on focus loss or Escape.
class Panel : public QWidget
{
public:
    explicit Panel(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_hideTimer(new QTimer(this))
    {
        setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setObjectName(QStringLiteral("panel"));
        setStyleSheet(QStringLiteral(
            "QWidget#panel { background: %1; border: 1px solid %2; border-radius: 8px; }")
                .arg(palette().color(QPalette::Window).name(), QLatin1String("#8a8a92")));
        m_hideTimer->setSingleShot(true);
        m_hideTimer->setInterval(400);
        connect(m_hideTimer, &QTimer::timeout, this, &QWidget::hide);
    }

    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::WindowDeactivate) {
            // Ignore brief focus switches (e.g. the combo dropdown) and only
            // dismiss when we truly stay deactivated.
            if (!m_hideTimer->isActive()) {
                m_hideTimer->start();
            }
            return true;
        }
        if (e->type() == QEvent::WindowActivate || e->type() == QEvent::ActivationChange) {
            m_hideTimer->stop();
        }
        if (e->type() == QEvent::KeyPress) {
            auto *ke = static_cast<QKeyEvent *>(e);
            if (ke->key() == Qt::Key_Escape) {
                m_hideTimer->stop();
                hide();
                return true;
            }
        }
        return QWidget::event(e);
    }

private:
    QTimer *m_hideTimer;
};

} // namespace

Tray::Tray(Controller *controller, QObject *parent)
    : QObject(parent)
    , m_controller(controller)
    , m_applyTimer(new QTimer(this))
{
    m_applyTimer->setSingleShot(true);
    m_applyTimer->setInterval(150);
    connect(m_applyTimer, &QTimer::timeout, this, &Tray::applyPendingGlobal);

    buildUi();
    buildContextMenu();

    m_tray = new QSystemTrayIcon(makeAppIcon(), this);
    m_tray->setToolTip(QStringLiteral("Ghosty"));
    m_tray->setContextMenu(m_menu);
    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            togglePanel();
        }
    });
    m_tray->show();

    connect(m_controller, &Controller::windowsChanged, this, &Tray::refreshWindows);
    refreshWindows();
    refreshOverrideUi();
}

void Tray::buildUi()
{
    m_panel = new Panel;
    m_panel->setMinimumWidth(300);

    auto *v = new QVBoxLayout(m_panel);
    v->setContentsMargins(10, 10, 10, 10);
    v->setSpacing(8);

    // ---- Title row ----
    auto *titleRow = new QHBoxLayout;
    auto *title = new QLabel(QStringLiteral("Ghosty"));
    QFont f = title->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 1);
    title->setFont(f);
    titleRow->addWidget(title);
    titleRow->addStretch(1);
    auto *closeBtn = new QPushButton(QStringLiteral("\u2715"));
    closeBtn->setToolTip(QStringLiteral("Close"));
    closeBtn->setFixedSize(24, 24);
    titleRow->addWidget(closeBtn);
    v->addLayout(titleRow);

    // ---- All windows ----
    auto *globalTitle = new QLabel(QStringLiteral("All windows"));
    auto *globalRow = new QHBoxLayout;
    globalRow->setSpacing(8);
    m_globalSlider = makeSlider(Qt::Horizontal);
    m_globalSlider->setValue(m_controller->globalOpacity());
    m_globalValue = pctLabel();
    m_globalValue->setText(QStringLiteral("%1%").arg(m_controller->globalOpacity()));
    globalRow->addWidget(m_globalSlider, 1);
    globalRow->addWidget(m_globalValue);
    v->addWidget(globalTitle);
    v->addLayout(globalRow);

    connect(m_globalSlider, &QSlider::valueChanged, this, [this](int value) {
        m_globalValue->setText(QStringLiteral("%1%").arg(value));
        scheduleApply();
    });

    // ---- Selected window ----
    auto *winTitle = new QLabel(QStringLiteral("One window"));
    v->addWidget(winTitle);

    auto *comboRow = new QHBoxLayout;
    comboRow->setSpacing(8);
    m_windows = new QComboBox;
    m_windows->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    comboRow->addWidget(m_windows, 1);
    auto *resetBtn = new QPushButton(QStringLiteral("Reset"));
    resetBtn->setEnabled(false);
    comboRow->addWidget(resetBtn);
    v->addLayout(comboRow);

    auto *winRow = new QHBoxLayout;
    winRow->setSpacing(8);
    m_winSlider = makeSlider(Qt::Horizontal);
    m_winSlider->setEnabled(false);
    m_winValue = pctLabel();
    winRow->addWidget(m_winSlider, 1);
    winRow->addWidget(m_winValue);
    v->addLayout(winRow);

    connect(m_windows, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int) {
                m_currentWindow = m_windows->currentText();
                m_winSlider->blockSignals(true);
                const int ov = m_controller->overrideFor(m_currentWindow);
                if (ov >= 0) {
                    m_winSlider->setValue(ov);
                    m_winSlider->setEnabled(true);
                    m_winValue->setText(QStringLiteral("%1%").arg(ov));
                } else {
                    m_winSlider->setValue(m_controller->globalOpacity());
                    m_winSlider->setEnabled(true);
                    m_winValue->setText(QStringLiteral("follows global (%1%)").arg(m_controller->globalOpacity()));
                }
                m_winSlider->blockSignals(false);
            });

    connect(m_winSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_currentWindow.isEmpty()) {
            return;
        }
        m_winValue->setText(QStringLiteral("%1%").arg(value));
        m_controller->SetOverride(m_currentWindow, value);
    });

    connect(resetBtn, &QPushButton::clicked, this, [this] {
        if (m_currentWindow.isEmpty()) {
            return;
        }
        m_controller->ClearOverride(m_currentWindow);
        refreshOverrideUi();
    });

    // ---- Footer ----
    auto *footer = new QHBoxLayout;
    footer->setSpacing(8);
    auto *clearAll = new QPushButton(QStringLiteral("Clear all"));
    clearAll->setToolTip(QStringLiteral("Remove all per-window overrides"));
    footer->addWidget(clearAll);
    footer->addStretch(1);
    auto *quit = new QPushButton(QStringLiteral("Quit"));
    footer->addWidget(quit);
    v->addLayout(footer);

    connect(closeBtn, &QPushButton::clicked, m_panel, &QWidget::hide);
    connect(clearAll, &QPushButton::clicked, this, [this] {
        m_controller->ClearOverrides();
        refreshOverrideUi();
    });
    connect(quit, &QPushButton::clicked, qApp, &QApplication::quit);
}

void Tray::buildContextMenu()
{
    m_menu = new QMenu;
    auto *open = m_menu->addAction(QStringLiteral("Opacity panel"));
    connect(open, &QAction::triggered, this, [this] {
        togglePanel();
    });
    m_menu->addSeparator();
    auto *clearAll = m_menu->addAction(QStringLiteral("Clear all overrides"));
    connect(clearAll, &QAction::triggered, this, [this] {
        m_controller->ClearOverrides();
        refreshOverrideUi();
    });
    m_menu->addSeparator();
    auto *quit = m_menu->addAction(QStringLiteral("Quit"));
    connect(quit, &QAction::triggered, qApp, &QApplication::quit);
}

void Tray::togglePanel()
{
    if (m_panel->isVisible()) {
        m_panel->hide();
        return;
    }
    refreshOverrideUi();
    m_panel->show();
    m_panel->raise();
    m_panel->activateWindow();
    m_panel->setFocus();
}

void Tray::refreshWindows()
{
    const QString current = m_windows->currentText();
    m_windows->blockSignals(true);
    m_windows->clear();
    m_windows->addItems(m_controller->windows());
    if (!current.isEmpty() && m_controller->windows().contains(current)) {
        m_windows->setCurrentText(current);
    } else {
        m_currentWindow = m_windows->currentText();
    }
    m_windows->blockSignals(false);
    refreshOverrideUi();
}

void Tray::refreshOverrideUi()
{
    if (m_currentWindow.isEmpty()) {
        m_winSlider->setEnabled(false);
        m_winValue->setText(QStringLiteral("select a window"));
        return;
    }
    const int ov = m_controller->overrideFor(m_currentWindow);
    m_winSlider->blockSignals(true);
    if (ov >= 0) {
        m_winSlider->setValue(ov);
        m_winValue->setText(QStringLiteral("%1%").arg(ov));
    } else {
        m_winSlider->setValue(m_controller->globalOpacity());
        m_winValue->setText(QStringLiteral("follows global (%1%)").arg(m_controller->globalOpacity()));
    }
    m_winSlider->blockSignals(false);
    m_winSlider->setEnabled(true);
}

void Tray::scheduleApply()
{
    m_applyTimer->start();
}

void Tray::applyPendingGlobal()
{
    m_controller->SetGlobalOpacity(m_globalSlider->value());
    refreshOverrideUi();
}