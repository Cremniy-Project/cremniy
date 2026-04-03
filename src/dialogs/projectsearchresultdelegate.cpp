#include "projectsearchresultdelegate.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>

ProjectSearchResultDelegate::ProjectSearchResultDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ProjectSearchResultDelegate::setNeedle(const QString &needle)
{
    m_needle = needle;
}

void ProjectSearchResultDelegate::setPreviewColumn(int column)
{
    m_previewColumn = column;
}

void ProjectSearchResultDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    if (index.column() != m_previewColumn || m_needle.isEmpty()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    const QString text = index.data(Qt::DisplayRole).toString();
    const QRect textArea = style->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);

    painter->save();
    painter->setFont(option.font);
    QFontMetrics fm(option.font);
    const int yBase =
        textArea.top() + fm.ascent() + (textArea.height() - fm.height()) / 2;

    const QColor normalPen = option.palette.color(QPalette::Text);
    const QColor hiBg = option.palette.color(QPalette::Highlight);
    const QColor hiFg = option.palette.color(QPalette::HighlightedText);

    const QString lowerText = text.toLower();
    const QString lowerNeedle = m_needle.toLower();
    int last = 0;
    int x = textArea.left();

    while (last < text.size()) {
        const int hit = lowerText.indexOf(lowerNeedle, last);
        if (hit < 0) {
            painter->setPen(normalPen);
            painter->drawText(x, yBase, text.mid(last));
            break;
        }
        if (hit > last) {
            const QString before = text.mid(last, hit - last);
            painter->setPen(normalPen);
            painter->drawText(x, yBase, before);
            x += fm.horizontalAdvance(before);
        }
        const QString matched = text.mid(hit, m_needle.size());
        const int w = fm.horizontalAdvance(matched);
        const int rowTop = textArea.top() + (textArea.height() - fm.height()) / 2;
        painter->fillRect(QRect(x, rowTop, w, fm.height()), hiBg);
        painter->setPen(hiFg);
        painter->drawText(x, yBase, matched);
        painter->setPen(normalPen);
        x += w;
        last = hit + m_needle.size();
    }

    painter->restore();
}
