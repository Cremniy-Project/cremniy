#ifndef TOOLSMENU_H
#define TOOLSMENU_H

#include "ui/MenuBar/basemenu.h"

#include <QList>

class IDEWindow;
class QAction;

class ToolsMenu : public BaseMenu
{
    Q_OBJECT
private:
    QAction* m_reverseCalculator;
    IDEWindow* m_ideWindow = nullptr;
    QList<QAction*> m_toolTabActions;
public:
    ToolsMenu();
    void setupConnections(IDEWindow* ideWind);
private:
    void on_Open_ReverseCalculator();
};

#endif // TOOLSMENU_H
