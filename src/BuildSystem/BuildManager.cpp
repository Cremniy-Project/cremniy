#include "BuildManager.h"
#include <QStringList>

BuildManager::BuildManager(QObject* parent) : QObject(parent) {
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        const QString out = QString::fromUtf8(m_process.readAllStandardOutput());
        for (const QString& line : out.split('\n', Qt::SkipEmptyParts))
            emit outputLine(line);
    });

    connect(&m_process, &QProcess::readyReadStandardError, this, [this]() {
        const QString err = QString::fromUtf8(m_process.readAllStandardError());
        for (const QString& line : err.split('\n', Qt::SkipEmptyParts))
            emit outputLine("[stderr] " + line);
    });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) {
                emit processFinished(code);
            });

    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err) {
        emit errorOccurred(m_process.errorString());
    });
}

void BuildManager::setProjectDir(const QString& dir) { m_projectDir = dir; }
void BuildManager::setConfig(const BuildConfig& cfg) { m_config = cfg; }

bool BuildManager::isRunning() const {
    return m_process.state() != QProcess::NotRunning;
}

void BuildManager::runBuild() { startCommand(m_config.build); }
void BuildManager::runRun()   { startCommand(m_config.run);   }
void BuildManager::runClean() { startCommand(m_config.clean); }

void BuildManager::stopProcess() {
    if (isRunning()) {
        m_process.terminate();
        if (!m_process.waitForFinished(2000))
            m_process.kill();
    }
}

void BuildManager::startCommand(const QString& cmd) {
    if (cmd.trimmed().isEmpty()) {
        emit errorOccurred("Command is not configured.");
        return;
    }
    if (isRunning()) {
        emit errorOccurred("A process is already running.");
        return;
    }

    m_process.setWorkingDirectory(m_projectDir);
    emit processStarted(cmd);
    emit outputLine("$ " + cmd);

#ifdef Q_OS_WIN
    m_process.start("cmd.exe", QStringList{"/C", cmd});
#else
    m_process.start("sh", QStringList{"-c", cmd});
#endif
}