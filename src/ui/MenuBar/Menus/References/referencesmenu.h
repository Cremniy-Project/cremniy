#ifndef REFERENCESMENU_H
#define REFERENCESMENU_H

#include "ui/MenuBar/basemenu.h"

class ReferencesMenu : public BaseMenu
{
    Q_OBJECT

public:
    ReferencesMenu();
    void setupConnections(class IDEWindow* ideWind) override;

private:
    QList<class QAction*> m_toolActions;

};

#endif // REFERENCESMENU_H
