#pragma once

#include <QList>
#include <QString>
#include <QJsonObject>
#include <QPluginLoader>

class IPlugin;

class PluginManager {
public:
    static PluginManager& instance();

    struct PluginInfo {
        QString name;
        QString version;
        QString description;
        QString author;
        IPlugin* instance = nullptr;
    };

    void loadPlugins(const QString& pluginsDir);
    const QList<PluginInfo>& loadedPlugins() const;

private:
    PluginManager() = default;
    QList<PluginInfo> m_plugins;
};
