#include "projectsearchworker.h"
#include "projectsearchengine.h"

#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

namespace {

constexpr int kBatchSize = 80;

} // namespace

void ProjectSearchWorker::requestCancel()
{
    m_cancelled.store(true);
}

ProjectSearchWorker::ProjectSearchWorker(QObject *parent) : QObject(parent) {}

void ProjectSearchWorker::runSearch(QString rootPath, QString query)
{
    m_cancelled.store(false);

    const QString needle = query.trimmed();
    if (needle.isEmpty()) {
        emit searchFinished();
        return;
    }

    QStringList batchPaths;
    QList<int> batchLines;
    QStringList batchPreviews;

    auto flushBatch = [this, &batchPaths, &batchLines, &batchPreviews]() {
        if (batchPaths.isEmpty())
            return;
        emit hitsBatch(batchPaths, batchLines, batchPreviews);
        batchPaths.clear();
        batchLines.clear();
        batchPreviews.clear();
    };

    const QString rootClean =
        QDir::cleanPath(QFileInfo(rootPath).absoluteFilePath());
    QDirIterator it(rootClean, QDir::Files | QDir::Readable | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        if (m_cancelled.load())
            break;

        const QString filePath = it.next();
        if (ProjectSearchEngine::pathContainsSkippedDirectory(rootClean, filePath))
            continue;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        constexpr qint64 kProbe = 4096;
        const QByteArray probe = file.peek(kProbe);
        if (ProjectSearchEngine::isProbableBinarySample(probe)) {
            file.close();
            continue;
        }
        file.seek(0);

        QTextStream stream(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        stream.setEncoding(QStringConverter::Utf8);
#else
        if (QTextCodec *utf8 = QTextCodec::codecForName("UTF-8"))
            stream.setCodec(utf8);
#endif

        int lineNumber = 0;
        while (!stream.atEnd()) {
            if (m_cancelled.load())
                break;

            ++lineNumber;
            const QString line = stream.readLine();
            if (line.contains(needle, Qt::CaseInsensitive)) {
                batchPaths.append(filePath);
                batchLines.append(lineNumber);
                batchPreviews.append(line);
                if (batchPaths.size() >= kBatchSize)
                    flushBatch();
            }
        }
    }

    flushBatch();
    emit searchFinished();
}
