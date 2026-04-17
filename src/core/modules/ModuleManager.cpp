#include "core/modules/ModuleManager.h"


ModuleManager& ModuleManager::instance() {
    static ModuleManager inst;
    return inst;
}

void ModuleManager::registerTab(std::function<QString()> name, const QString& group, CreatorTabModule creator, const int& position) {
    TabModuleDescription tabDesc;
    tabDesc.creator = creator;
    tabDesc.name = name;
    tabDesc.position = position;
    m_tabModuleCreators[group.trimmed()].append(tabDesc);
}

void ModuleManager::registerWindow(std::function<QString()> name, const QString& group, CreatorWindowModule creator) {
    WindowModuleDescription winDesc;
    winDesc.creator = creator;
    winDesc.name = name;
    m_windowModuleCreators[group.trimmed()].append(winDesc);
}

void ModuleManager::registerReference(std::function<QString()> name, const QString& group, CreatorReferenceModule creator) {
    ReferenceModuleDescription refDesc;
    refDesc.creator = creator;
    refDesc.name = name;
    m_referenceModuleCreators[group.trimmed()].append(refDesc);
}

QList<QString> ModuleManager::getTabGroups() const{
    QList<QString> keys = m_tabModuleCreators.keys();
    std::sort(keys.begin(), keys.end(), [](const QString& a, const QString& b) {
        if (a.isEmpty() && !b.isEmpty()) return false;
        if (!a.isEmpty() && b.isEmpty()) return true;

        return a < b;
    });
    return keys;
}

QList<QString> ModuleManager::getWindowGroups() const{
    QList<QString> keys = m_windowModuleCreators.keys();
    std::sort(keys.begin(), keys.end(), [](const QString& a, const QString& b) {
        if (a.isEmpty() && !b.isEmpty()) return false;
        if (!a.isEmpty() && b.isEmpty()) return true;

        return a < b;
    });
    return keys;
}

QList<QString> ModuleManager::getReferenceGroups() const{
    QList<QString> keys = m_referenceModuleCreators.keys();
    std::sort(keys.begin(), keys.end(), [](const QString& a, const QString& b) {
        if (a.isEmpty() && !b.isEmpty()) return false;
        if (!a.isEmpty() && b.isEmpty()) return true;

        return a < b;
    });
    return keys;
}

const QVector<TabModuleDescription>& ModuleManager::getTabsByGroup(const QString& group) const {
    static const QVector<TabModuleDescription> empty;
    auto it = m_tabModuleCreators.find(group);
    if (it == m_tabModuleCreators.end()) return empty;
    return it.value();
}

const QVector<WindowModuleDescription>& ModuleManager::getWindowsByGroup(const QString& group) const {
    static const QVector<WindowModuleDescription> empty;
    auto it = m_windowModuleCreators.find(group);
    if (it == m_windowModuleCreators.end()) return empty;
    return it.value();
}

const QVector<ReferenceModuleDescription>& ModuleManager::getReferencesByGroup(const QString& group) const {
    static const QVector<ReferenceModuleDescription> empty;
    auto it = m_referenceModuleCreators.find(group);
    if (it == m_referenceModuleCreators.end()) return empty;
    return it.value();
}

TabBase* ModuleManager::createTab(const QString& group, const int& index) {
    if (m_tabModuleCreators.contains(group)) {
        const QVector<TabModuleDescription>& tabDescs = m_tabModuleCreators[group];
        if (index >= 0 && index < tabDescs.size()) {
            return tabDescs[index].creator();
        }
    }
    return nullptr;
}

WindowBase* ModuleManager::createWindow(const QString& group, const int& index) {
    if (m_windowModuleCreators.contains(group)) {
        const QVector<WindowModuleDescription>& windowDesc = m_windowModuleCreators[group];
        if (index >= 0 && index < windowDesc.size()) {
            return windowDesc[index].creator();
        }
    }
    return nullptr;
}

ReferenceBase* ModuleManager::createReference(const QString& group, const int& index) {
    if (m_referenceModuleCreators.contains(group)) {
        const QVector<ReferenceModuleDescription>& referenceDesc = m_referenceModuleCreators[group];
        if (index >= 0 && index < referenceDesc.size()) {
            return referenceDesc[index].creator();
        }
    }
    return nullptr;
}