#ifndef TOOLSMENU_H
#define TOOLSMENU_H

#include "core/modules/ModuleManager.h"
#include "ui/MenuBar/basemenu.h"

#include <QList>

class IDEWindow;
class QAction;

class ToolsMenu : public BaseMenu
{
    Q_OBJECT
private:

public:
    ToolsMenu();
    void setupConnections(IDEWindow* ideWind);

signals:
    void openTabModule(TabModuleDescription desc);
};

#endif // TOOLSMENU_H
