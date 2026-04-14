#pragma once
#include <QObject>
#include "core/IPlugin.h"

class DummyPlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "plugin.json")
    Q_INTERFACES(IPlugin)
public:
    void initialize() override;
};
