#include "controller.h"

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>

namespace {
const QString kKWinService = QStringLiteral("org.kde.KWin");
const QString kAppService = QStringLiteral("org.kde.opacity-slider");
const QString kEffectId = QStringLiteral("opacity-slider");
const QString kScriptId = QStringLiteral("opacity-windows");
const QString kStateDir = QStringLiteral("opacity-slider");
} // namespace

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    loadState();
}

bool Controller::registerService()
{
    const bool ok = QDBusConnection::sessionBus().registerService(kAppService);
    if (ok) {
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllSlots);
    }
    return ok;
}

QString Controller::assetsDir() const
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/assets");
}

QString Controller::installedEffectDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QStringLiteral("/kwin/effects/") + kEffectId;
}

QString Controller::installedScriptDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QStringLiteral("/kwin/scripts/") + kScriptId;
}

bool Controller::writeFile(const QString &path, const QByteArray &data) const
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(data);
    return file.commit();
}

QString Controller::readAsset(const QString &name) const
{
    QFile f(assetsDir() + QLatin1Char('/') + name);
    if (!f.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromUtf8(f.readAll());
}

bool Controller::installEffect()
{
    if (!writeFile(installedEffectDir() + QStringLiteral("/metadata.json"), readAsset(QStringLiteral("effect-metadata.json")).toUtf8())) {
        return false;
    }
    // Seed the effect with the current state so it is valid before first apply().
    const QByteArray js = readAsset(QStringLiteral("effect-template.js")).replace(QLatin1String("__STATE__"), stateJson().toUtf8()).toUtf8();
    return writeFile(installedEffectDir() + QStringLiteral("/contents/code/main.js"), js);
}

bool Controller::installScript()
{
    if (!writeFile(installedScriptDir() + QStringLiteral("/metadata.json"), readAsset(QStringLiteral("script-metadata.json")).toUtf8())) {
        return false;
    }
    return writeFile(installedScriptDir() + QStringLiteral("/contents/code/main.js"),
                     readAsset(QStringLiteral("watcher-script.js")).toUtf8());
}

bool Controller::ensureInstalled()
{
    const bool ok = installEffect() && installScript();
    if (ok) {
        enablePlugins();
    }
    return ok;
}

void Controller::enablePlugins()
{
    static const QStringList keys = { kEffectId + QStringLiteral("Enabled"),
                                      kScriptId + QStringLiteral("Enabled") };

    const QString kwrite = QStandardPaths::findExecutable(QStringLiteral("kwriteconfig6"));
    if (!kwrite.isEmpty()) {
        for (const QString &key : keys) {
            QProcess::execute(kwrite, { QStringLiteral("--file"), QStringLiteral("kwinrc"),
                                        QStringLiteral("--group"), QStringLiteral("Plugins"),
                                        QStringLiteral("--key"), key, QStringLiteral("true") });
        }
    } else {
        QSettings kwin(QStringLiteral("kwinrc"), QSettings::IniFormat);
        kwin.beginGroup(QStringLiteral("Plugins"));
        for (const QString &key : keys) {
            kwin.setValue(key, true);
        }
        kwin.sync();
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(kKWinService, QStringLiteral("/KWin"),
                                                      QStringLiteral("org.kde.KWin"), QStringLiteral("reconfigure"));
    QDBusConnection::sessionBus().asyncCall(msg);
}

bool Controller::isEffectLoaded(const QString &id) const
{
    QDBusInterface iface(kKWinService, QStringLiteral("/Effects"), QStringLiteral("org.kde.kwin.Effects"),
                         QDBusConnection::sessionBus());
    QDBusReply<bool> reply = iface.call(QStringLiteral("isEffectLoaded"), id);
    return reply.isValid() && reply.value();
}

bool Controller::loadEffect(const QString &id)
{
    QDBusInterface iface(kKWinService, QStringLiteral("/Effects"), QStringLiteral("org.kde.kwin.Effects"),
                         QDBusConnection::sessionBus());
    QDBusReply<bool> reply = iface.call(QStringLiteral("loadEffect"), id);
    return reply.isValid() && reply.value();
}

bool Controller::unloadEffect(const QString &id)
{
    QDBusInterface iface(kKWinService, QStringLiteral("/Effects"), QStringLiteral("org.kde.kwin.Effects"),
                         QDBusConnection::sessionBus());
    QDBusReply<void> reply = iface.call(QStringLiteral("unloadEffect"), id);
    return reply.isValid();
}

void Controller::rearmWatcher()
{
    QDBusInterface scripting(kKWinService, QStringLiteral("/Scripting"), QStringLiteral("org.kde.kwin.Scripting"),
                             QDBusConnection::sessionBus());
    scripting.call(QStringLiteral("unloadScript"), kScriptId);

    const QString path = installedScriptDir() + QStringLiteral("/contents/code/main.js");
    QDBusReply<int> reply = scripting.call(QStringLiteral("loadScript"), path, kScriptId);
    if (!reply.isValid()) {
        return;
    }
    QDBusInterface script(kKWinService, QStringLiteral("/Scripting/Script%1").arg(reply.value()),
                          QStringLiteral("org.kde.kwin.Script"), QDBusConnection::sessionBus());
    script.call(QStringLiteral("run"));
}

QString Controller::stateJson() const
{
    QJsonObject overrides;
    for (auto it = m_overrides.constBegin(); it != m_overrides.constEnd(); ++it) {
        overrides.insert(it.key(), it.value());
    }
    QJsonObject root;
    root.insert(QStringLiteral("opacity"), m_globalOpacity);
    root.insert(QStringLiteral("overrides"), overrides);
    root.insert(QStringLiteral("transition"), m_transitionMs);
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

bool Controller::apply()
{
    if (!installEffect()) {
        return false;
    }

    unloadEffect(kEffectId);
    const bool loaded = loadEffect(kEffectId);
    if (!loaded) {
        // Plugin might not be known to KWin yet (first run). Make sure it is
        // enabled and give KWin a moment to notice, then retry once.
        enablePlugins();
        return loadEffect(kEffectId);
    }
    emit stateChanged();
    return true;
}

void Controller::ApplyWindows(const QVariantList &captions)
{
    QStringList names;
    names.reserve(captions.size());
    for (const QVariant &v : captions) {
        const QString s = v.toString();
        if (!s.isEmpty()) {
            names.append(s);
        }
    }
    if (names != m_windows) {
        m_windows = names;
        emit windowsChanged(m_windows);
    }
}

void Controller::saveState()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + kStateDir;
    QDir().mkpath(dir);
    QSaveFile f(dir + QStringLiteral("/state.json"));
    if (!f.open(QIODevice::WriteOnly)) {
        return;
    }
    f.write(stateJson().toUtf8());
    f.commit();
}

void Controller::loadState()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + kStateDir;
    QFile f(dir + QStringLiteral("/state.json"));
    if (!f.exists()) {
        return;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        return;
    }
    const QJsonObject root = doc.object();
    m_globalOpacity = qBound(0, root.value(QLatin1String("opacity")).toInt(100), 100);
    m_transitionMs = qBound(0, root.value(QLatin1String("transition")).toInt(250), 1000);
    const QJsonObject overrides = root.value(QLatin1String("overrides")).toObject();
    m_overrides.clear();
    for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it) {
        m_overrides.insert(it.key(), qBound(0, it.value().toInt(100), 100));
    }
}

void Controller::setGlobalOpacity(int pct)
{
    m_globalOpacity = qBound(0, pct, 100);
    saveState();
    apply();
}

void Controller::setOverride(const QString &caption, int pct)
{
    if (caption.isEmpty()) {
        return;
    }
    m_overrides.insert(caption, qBound(0, pct, 100));
    saveState();
    apply();
}

void Controller::clearOverride(const QString &caption)
{
    if (m_overrides.remove(caption)) {
        saveState();
        apply();
    }
}

void Controller::clearAllOverrides()
{
    if (m_overrides.isEmpty()) {
        return;
    }
    m_overrides.clear();
    saveState();
    apply();
}