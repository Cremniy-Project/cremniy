#include "PluginManager.h"
#include "IPlugin.h"

#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QCoreApplication>
#include <QLibrary>

PluginManager& PluginManager::instance() {
    static PluginManager inst;
    return inst;
}

void PluginManager::loadPlugins(const QString& pluginsDir) {
    QDir dir(pluginsDir);
    if (!dir.exists()) {
        qDebug() << "Plugin directory does not exist:" << pluginsDir;
        return;
    }

    const auto entryList = dir.entryList(QDir::Files);
    for (const QString &fileName : entryList) {
        if (!QLibrary::isLibrary(fileName)) {
            continue;
        }

        QString absolutePath = dir.absoluteFilePath(fileName);
        QPluginLoader loader(absolutePath);
        
        QObject *pluginObj = loader.instance();
        if (pluginObj) {
            IPlugin *plugin = qobject_cast<IPlugin*>(pluginObj);
            if (plugin) {
                // Read embedded metadata 
                QJsonObject metaData = loader.metaData();
                QJsonObject metaDataObj = metaData.value("MetaData").toObject();
                
                PluginInfo info;
                info.instance = plugin;
                info.name = metaDataObj.value("name").toString(fileName);
                info.version = metaDataObj.value("version").toString("1.0.0");
                info.description = metaDataObj.value("description").toString("");
                info.author = metaDataObj.value("author").toString("");

                // Initialize the plugin
                plugin->initialize();
                
                m_plugins.append(info);
                qDebug() << "Loaded plugin:" << info.name << "v" << info.version;
            } else {
                qDebug() << "Plugin loaded but it is not an IPlugin:" << absolutePath;
                loader.unload();
            }
        } else {
            qDebug() << "Failed to load plugin:" << absolutePath << loader.errorString();
        }
    }
}

const QList<PluginManager::PluginInfo>& PluginManager::loadedPlugins() const {
    return m_plugins;
}
