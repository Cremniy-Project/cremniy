#include "BuildConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

bool BuildConfigManager::load(const QString& projectDir, BuildConfig& out) {
    QFile file(QDir(projectDir).filePath(CONFIG_FILE));
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject())
        return false;

    QJsonObject obj = doc.object();
    out.build = obj.value("build").toString();
    out.run   = obj.value("run").toString();
    out.clean = obj.value("clean").toString();
    return true;
}

bool BuildConfigManager::save(const QString& projectDir, const BuildConfig& cfg) {
    QJsonObject obj;
    obj["build"] = cfg.build;
    obj["run"]   = cfg.run;
    obj["clean"] = cfg.clean;

    QFile file(QDir(projectDir).filePath(CONFIG_FILE));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}

bool BuildConfigManager::autoDetect(const QString& projectDir, BuildConfig& out) {
    QDir dir(projectDir);

    if (dir.exists("CMakeLists.txt")) {
        out = defaultCMakeTemplate(projectDir);
        return true;
    }

    if (dir.exists("Makefile")) {
        out.build = "make";
        out.run = "./<target>";
        out.clean = "make clean";
        return true;
    }

    return false;
}

BuildConfig BuildConfigManager::defaultCMakeTemplate(const QString& projectDir)
{
    QDir dir(projectDir);
    QString projectName;

    if (dir.exists("CMakeLists.txt"))
        projectName = detectCMakeProjectName(dir.filePath("CMakeLists.txt"));

    if (projectName.isEmpty())
        projectName = QFileInfo(projectDir).fileName().trimmed();

    if (projectName.isEmpty())
        projectName = "app";

    BuildConfig cfg;
    cfg.build = "cmake -S . -B build && cmake --build build";
    cfg.clean = "cmake --build build --target clean";

#ifdef Q_OS_WIN
    cfg.run = QString(".\\build\\Release\\%1.exe").arg(projectName);
#else
    cfg.run = QString("./build/%1").arg(projectName);
#endif

    return cfg;
}

QString BuildConfigManager::detectCMakeProjectName(const QString& cmakeListsPath)
{
    QFile file(cmakeListsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    const QString content = QString::fromUtf8(file.readAll());
    const QRegularExpression re(R"(project\s*\(\s*([^\s\)]+))", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = re.match(content);
    if (!match.hasMatch())
        return {};

    return match.captured(1).trimmed();
}
