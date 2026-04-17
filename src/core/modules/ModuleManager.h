#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "core/modules/ReferenceBase.h"
#include "core/modules/TabBase.h"
#include "core/modules/WindowBase.h"

using CreatorTabModule = std::function<TabBase*()>;
using CreatorWindowModule = std::function<WindowBase*()>;
using CreatorReferenceModule = std::function<ReferenceBase*()>;

struct TabModuleDescription {
    CreatorTabModule creator;
    std::function<QString()> name;
    int position;
};

struct WindowModuleDescription {
    CreatorWindowModule creator;
    std::function<QString()> name;
};

struct ReferenceModuleDescription {
    CreatorReferenceModule creator;
    std::function<QString()> name;
};

class ModuleManager {

public:
    static ModuleManager& instance();

    void registerTab(std::function<QString()> name, const QString& group, CreatorTabModule creator, const int& position = 0);
    void registerWindow(std::function<QString()> name, const QString& group, CreatorWindowModule creator);
    void registerReference(std::function<QString()> name,const QString& group, CreatorReferenceModule creator);

    QList<QString> getTabGroups() const;
    QList<QString> getWindowGroups() const;
    QList<QString> getReferenceGroups() const;

    const QVector<TabModuleDescription>& getTabsByGroup(const QString& group) const;
    const QVector<WindowModuleDescription>& getWindowsByGroup(const QString& group) const;
    const QVector<ReferenceModuleDescription>& getReferencesByGroup(const QString& group) const;

    TabBase* createTab(const QString& group, const int& index);
    WindowBase* createWindow(const QString& group, const int& index);
    ReferenceBase* createReference(const QString& group, const int& index);

private:

    QHash<QString, QVector<TabModuleDescription>> m_tabModuleCreators;
    QHash<QString, QVector<WindowModuleDescription>> m_windowModuleCreators;
    QHash<QString, QVector<ReferenceModuleDescription>> m_referenceModuleCreators;

};

#endif // MODULEMANAGER_H
