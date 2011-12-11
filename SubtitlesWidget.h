#ifndef SUBTITLESWIDGET_H
#define SUBTITLESWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QGraphicsDropShadowEffect>


#include "SubtitlesEditor.h"

class SubtitlesWidget : public QWidget
{
Q_OBJECT

public:
    explicit SubtitlesWidget(QWidget *parent = 0);

public slots:
    void showText(QList<Subtitle> bottomSubtitles, QList<Subtitle> topSubtitles);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QList<Subtitle> topSubtitles;
    QList<Subtitle> bottomSubtitles;
    QGraphicsDropShadowEffect *shadowEffect;
};

#endif // SUBTITLESWIDGET_H
