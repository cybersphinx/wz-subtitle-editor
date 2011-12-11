#ifndef SUBTITLESEDITOR_H
#define SUBTITLESEDITOR_H

#include <QtGui/QMainWindow>
#include <QtGui/QLabel>

#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>

namespace Ui
{
    class MainWindow;
}

struct Subtitle
{
    QString text;
    double beginTime;
    double endTime;
    int positionX;
    int positionY;
};

class SubtitlesWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void actionOpen();
    void actionSave();
    void actionSaveAs();
    void actionAboutApplication();
    void stateChanged(Phonon::State state);
    void tick();
    void selectTrack(int track);
    void addSubtitle();
    void removeSubtitle();
    void previousSubtitle();
    void nextSubtitle();
    void showSubtitle();
    void updateSubtitle();
    void rescaleSubtitles();
    void playPause();

protected:
    void changeEvent(QEvent *event);
    QString timeToString(qint64 time);
    QList<Subtitle> readSubtitles(const QString &fileName);
    double timeToSeconds(QTime time);
    bool saveSubtitles(QString fileName);

private:
    Ui::MainWindow *ui;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    SubtitlesWidget *subtitlesWidget;
    QString currentPath;
    QLabel *fileNameLabel;
    QLabel *timeLabel;
    QList<QList<Subtitle> > subtitles;
    int currentSubtitle;
    int currentTrack;
};

#endif // SUBTITLESEDITOR_H
