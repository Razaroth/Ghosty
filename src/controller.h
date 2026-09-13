#pragma once

#include <QDBusConnection>
#include <QMap>
#include <QObject>
#include <QStringList>

class Controller : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.opacityslider")

public:
    explicit Controller(QObject *parent = nullptr);

    bool registerService();
    bool ensureInstalled();
    void rearmWatcher();
    bool apply();

    int globalOpacity() const { return m_globalOpacity; }
    QStringList windows() const { return m_windows; }
    bool hasOverride(const QString &caption) const { return m_overrides.contains(caption); }
    int overrideFor(const QString &caption) const { return m_overrides.value(caption, -1); }

    QString stateJson() const;

public slots:
    void ApplyWindows(const QVariantList &captions);
    QStringList Windows() const { return m_windows; }
    int GlobalOpacity() const { return m_globalOpacity; }
    int Transition() const { return m_transitionMs; }
    void SetGlobalOpacity(int pct) { setGlobalOpacity(pct); }
    void SetOverride(const QString &caption, int pct) { setOverride(caption, pct); }
    void ClearOverride(const QString &caption) { clearOverride(caption); }
    void ClearOverrides() { clearAllOverrides(); }

signals:
    void windowsChanged(const QStringList &windows);
    void stateChanged();

private:
    void setGlobalOpacity(int pct);
    void setOverride(const QString &caption, int pct);
    void clearOverride(const QString &caption);
    void clearAllOverrides();

private:
    QString installedEffectDir() const;
    QString installedScriptDir() const;
    QString assetsDir() const;

    bool installEffect();
    bool installScript();
    bool writeFile(const QString &path, const QByteArray &data) const;
    QString readAsset(const QString &name) const;

    void enablePlugins();
    bool loadEffect(const QString &id);
    bool unloadEffect(const QString &id);
    bool isEffectLoaded(const QString &id) const;

    void saveState();
    void loadState();

    QMap<QString, int> m_overrides; // caption -> opacity (0..100)
    int m_globalOpacity = 100;
    int m_transitionMs = 250;
    QStringList m_windows;
};