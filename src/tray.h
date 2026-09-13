#pragma once

#include <QObject>

class Controller;
class QSystemTrayIcon;
class QMenu;
class QWidget;
class QSlider;
class QComboBox;
class QLabel;
class QTimer;

class Tray : public QObject
{
    Q_OBJECT

public:
    explicit Tray(Controller *controller, QObject *parent = nullptr);

private:
    void buildUi();
    void buildContextMenu();
    void togglePanel();
    void refreshWindows();
    void refreshOverrideUi();
    void scheduleApply();
    void applyPendingGlobal();

    Controller *m_controller;
    QSystemTrayIcon *m_tray;
    QMenu *m_menu;
    QWidget *m_panel;
    QSlider *m_globalSlider;
    QLabel *m_globalValue;
    QComboBox *m_windows;
    QSlider *m_winSlider;
    QLabel *m_winValue;
    QTimer *m_applyTimer;
    QString m_currentWindow;
};