#include "dummyplugin.h"
#include "core/ToolsRegistry.h"
#include <QMessageBox>

void DummyPlugin::initialize() {
    ToolsRegistry::Descriptor desc;
    desc.id = "dummyplugin.tool";
    desc.name = "Dummy Extension Window";
    desc.kind = ToolKind::Window;
    desc.windowOpener = [](QWidget* parent) {
        QMessageBox::information(parent, "Dummy Extension", "Hello from the community plugin!");
    };
    
    ToolsRegistry::instance().registerTool(desc);
}
