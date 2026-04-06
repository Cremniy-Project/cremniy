#include "filetreeview.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QMimeData>


FileTreeView::FileTreeView(QWidget *parent)
    : QTreeView(parent)
{
}

void FileTreeView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = this->indexAt(event->pos());
    if (index.isValid())
        emit mouseClicked(index, event->button());

    QTreeView::mousePressEvent(event);
}

void FileTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    QModelIndex currentHover = indexAt(event->pos());
    
    if (currentHover != m_hoverIndex) {
        m_hoverIndex = currentHover;
        if (m_expandTimer->isActive()) {
            m_expandTimer->stop();
        }
        
        QFileSystemModel *fsModel = qobject_cast<QFileSystemModel*>(model());
        if (fsModel && fsModel->isDir(m_hoverIndex) && !isExpanded(m_hoverIndex)) {
            m_expandTimer->start();
        }
    }
    
    QTreeView::dragMoveEvent(event);
}

void FileTreeView::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData->hasUrls()) {
        event->ignore();
        return;
    }

    QFileSystemModel *fsModel = qobject_cast<QFileSystemModel*>(model());
    if (!fsModel) {
        event->ignore();
        return;
    }

    QModelIndex targetIndex = indexAt(event->pos());
    // Only drop into directories
    if (!targetIndex.isValid() || !fsModel->isDir(targetIndex)) {
        event->ignore();
        return;
    }

    QString targetDirPath = fsModel->filePath(targetIndex);
    bool movedAny = false;

    for (const QUrl &url : mimeData->urls()) {
        if (!url.isLocalFile()) continue;
        
        QString sourcePath = url.toLocalFile();
        QFileInfo sourceInfo(sourcePath);
        QString targetPath = QDir(targetDirPath).filePath(sourceInfo.fileName());

        if (sourcePath == targetPath) continue;

        QString msg = QString("Are you sure you want to move '%1' into '%2'?")
                      .arg(sourceInfo.fileName(), QFileInfo(targetDirPath).fileName());

        auto reply = QMessageBox::question(this, "Confirm Move", msg, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (QFile::rename(sourcePath, targetPath)) {
                movedAny = true;
            } else {
                QMessageBox::warning(this, "Error", "Failed to move the file.");
            }
        }
    }

    if (movedAny) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
