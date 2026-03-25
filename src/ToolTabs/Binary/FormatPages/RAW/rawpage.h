#ifndef RAWPAGE_H
#define RAWPAGE_H

#include "QHexView/qhexview.h"
#include "formatpage.h"

class RAWPage : public FormatPage
{ Q_OBJECT

private:
    QHexView* m_hexViewWidget;
    FileDataBuffer* m_sharedBuffer = nullptr;

public:
    explicit RAWPage(QWidget *parent = nullptr);

    QString pageName() const override { return "RAW"; }

    void setPageData(QByteArray& data) override;
    QByteArray getPageData() const override;
    void setSelection(qint64 pos, qint64 length) override;
    void showFind() override;
    void setSharedBuffer(FileDataBuffer* buffer) override;

};

#endif // RAWPAGE_H
