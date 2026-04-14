#pragma once
#include <QtPlugin>

class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Вызывается после загрузки плагина.
    // В этом методе плагин должен регистрировать свои инструменты
    // в ToolsRegistry.
    virtual void initialize() = 0;
};

#define IPlugin_iid "com.cremniy.IPlugin"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_iid)
