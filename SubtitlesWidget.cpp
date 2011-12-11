#include "SubtitlesWidget.h"

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

SubtitlesWidget::SubtitlesWidget(QWidget *parent) :
    QWidget(parent)
{
    shadowEffect = new QGraphicsDropShadowEffect(this);

    setAutoFillBackground(false);
    setGraphicsEffect(shadowEffect);
}

void SubtitlesWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    shadowEffect->setOffset(height() * 0.002);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);
    painter.setPen(Qt::white);

    QFont font = painter.font();
    font.setPixelSize(height() * 0.02);

    painter.setFont(font);

    for (int i = 0; i < topSubtitles.count(); ++i)
    {
        QRectF rectangle((width() * 0.05), (height() * 0.03 * i), (width() * 0.9), (height() * 0.03));

        painter.drawText(rectangle, Qt::AlignTop | Qt::AlignLeft, topSubtitles.at(i).text);
    }

    for (int i = 0; i < bottomSubtitles.count(); ++i)
    {
        QRectF rectangle((width() * 0.05), ((height() * 0.9) - (height() * 0.03 * i)), (width() * 0.9), (height() * 0.03));

        painter.drawText(rectangle, Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, bottomSubtitles.at(i).text);
    }
}

void SubtitlesWidget::showText(QList<Subtitle> bottomSubtitles, QList<Subtitle> topSubtitles)
{
    this->topSubtitles = topSubtitles;
    this->bottomSubtitles = bottomSubtitles;
}
