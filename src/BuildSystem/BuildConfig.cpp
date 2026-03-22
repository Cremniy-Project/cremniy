#include "BuildConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

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
        QString buildDir = projectDir + "/build";
        out.build = QString("cmake --build \"%1\"").arg(buildDir);
        out.run = QString("\"%1/Debug/app.exe\"").arg(buildDir);
        out.clean = QString("cmake --build \"%1\" --target clean").arg(buildDir);
        return true;
    }

    if (dir.exists("Makefile")) {
        out.build = "make";
        out.run = QString("\"%1/app\"").arg(projectDir);
        out.clean = "make clean";
        return true;
    }

    return false;
}