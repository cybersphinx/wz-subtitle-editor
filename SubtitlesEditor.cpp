#include "SubtitlesEditor.h"
#include "SubtitlesWidget.h"
#include "ui_SubtitlesEditor.h"

#include <QtCore/QSettings>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QDesktopServices>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    subtitles.append(QList<Subtitle>());
    subtitles.append(QList<Subtitle>());

    currentSubtitle = 0;
    currentTrack = 0;

    ui->setupUi(this);

    subtitlesWidget = new SubtitlesWidget(this);
    subtitlesWidget->show();
    subtitlesWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | subtitlesWidget->windowFlags());
    subtitlesWidget->setWindowModality(Qt::NonModal);
    subtitlesWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    subtitlesWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    subtitlesWidget->resize(100, 100);
    subtitlesWidget->move(QPoint(100, 100));

    mediaObject = new Phonon::MediaObject(this);
    mediaObject->setTickInterval(100);
    Phonon::createPath(mediaObject, ui->videoWidget);

    audioOutput = new Phonon::AudioOutput(this);
    Phonon::createPath(mediaObject, audioOutput);

    ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->actionPlayPause->setShortcut(tr("Space"));
    ui->actionPlayPause->setDisabled(true);

    ui->actionStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->actionStop->setDisabled(true);

    fileNameLabel = new QLabel(tr("No file loaded"), this);
    fileNameLabel->setMaximumWidth(300);
    timeLabel = new QLabel("00:00.0 / 00:00.0", this);

    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionExit->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    ui->playPauseButton->setDefaultAction(ui->actionPlayPause);
    ui->stopButton->setDefaultAction(ui->actionStop);
    ui->addButton->setDefaultAction(ui->actionAdd);
    ui->removeButton->setDefaultAction(ui->actionRemove);
    ui->previousButton->setDefaultAction(ui->actionPrevious);
    ui->nextButton->setDefaultAction(ui->actionNext);
    ui->seekSlider->setMediaObject(mediaObject);
    ui->volumeSlider->setAudioOutput(audioOutput);
    ui->statusBar->addPermanentWidget(fileNameLabel);
    ui->statusBar->addPermanentWidget(timeLabel);

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(addSubtitle()));
    connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(removeSubtitle()));
    connect(ui->actionPrevious, SIGNAL(triggered()), this, SLOT(previousSubtitle()));
    connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(nextSubtitle()));
    connect(ui->actionRescale, SIGNAL(triggered()), this, SLOT(rescaleSubtitles()));
    connect(ui->actionPlayPause, SIGNAL(triggered()), this, SLOT(playPause()));
    connect(ui->actionStop, SIGNAL(triggered()), mediaObject, SLOT(stop()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));
    connect(ui->actionAboutApplication, SIGNAL(triggered()), this, SLOT(actionAboutApplication()));
    connect(ui->trackComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectTrack(int)));
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick()));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);

    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

QString MainWindow::timeToString(qint64 time)
{
    QString string;
    int fractions = (time / 100);
    int seconds = (fractions / 10);
    int minutes = (seconds / 60);

    if (minutes < 10)
    {
        string.append('0');
    }

    string.append(QString::number(minutes));
    string.append(':');

    seconds = (seconds - (minutes * 60));

    if (seconds < 10)
    {
        string.append('0');
    }

    string.append(QString::number(seconds));
    string.append('.');

    fractions = (fractions - (seconds * 10) - (minutes * 600));

    string.append(QString::number(fractions));

    return string;
}

QList<Subtitle> MainWindow::readSubtitles(const QString &fileName)
{
    QFile file(fileName);
    QList<Subtitle> subtitles;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Can not read subtitle file:\n%1").arg(fileName));

        return subtitles;
    }

    QTextStream textStream(&file);

    while (!textStream.atEnd())
    {
        QString line = textStream.readLine().trimmed();

        if (line.isEmpty() || line.startsWith("//"))
        {
            continue;
        }

        QRegExp expression("(\\d+)\\s+(\\d+)\\s+([\\d\\.]+)\\s+([\\d\\.]+)\\s+_?\\(?\"(.+)\"\\)?");

        if (expression.exactMatch(line))
        {
            QStringList capturedTexts = expression.capturedTexts();
            Subtitle subtitle;

            subtitle.text = capturedTexts.at(5);
            subtitle.beginTime = capturedTexts.at(3).toDouble();
            subtitle.endTime = capturedTexts.at(4).toDouble();
            subtitle.positionX = capturedTexts.at(1).toInt();
            subtitle.positionY = capturedTexts.at(2).toInt();

            subtitles.append(subtitle);
        }
    }

    file.close();

    return subtitles;
}

double MainWindow::timeToSeconds(QTime time)
{
    double seconds = (time.minute() * 60);
    seconds += time.second();
    seconds += (time.msec() / 1000);

    return seconds;
}

bool MainWindow::saveSubtitles(QString fileName)
{
    Q_UNUSED(fileName)

    for (int i = 0; i < 2; ++i)
    {
        if (subtitles[i].isEmpty())
        {
            continue;
        }

        if (i > 0)
        {
            fileName = fileName.left(fileName.length() - 3) + "txa";
        }

        QFile file(fileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, tr("Error"), tr("Can not save subtitle file:\n%1").arg(fileName));

            return false;
        }

        QTextStream textStream(&file);

        for (int j = 0; j < subtitles[i].count(); ++j)
        {
            textStream << QString("%1\t%2\t\t%3\t%4\t_(\"%5\")\n").arg(subtitles[i][j].positionX).arg(subtitles[i][j].positionY).arg(subtitles[i][j].beginTime).arg(subtitles[i][j].endTime).arg(subtitles[i][j].text);

            if ((j + 1) < subtitles[i].count() && subtitles[i][j].beginTime != subtitles[i][j + 1].beginTime)
            {
                textStream << "\n";
            }
        }

        file.close();
    }

    return true;
}

void MainWindow::openMovie(const QString &filename)
{
    mediaObject->setCurrentSource(Phonon::MediaSource(filename));

    fileNameLabel->setText(QFileInfo(filename).fileName());
    timeLabel->setText(QString("00:00.0 / %1").arg(timeToString(mediaObject->totalTime())));
}

void MainWindow::openSubtitle(const QString &filename, int index)
{
    subtitles[index] = readSubtitles(filename);
}

void MainWindow::actionOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video or Subtitle file"),
            QSettings().value("lastUsedDir", QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString(),
            tr("Video and subtitle files (*.txt *.og?)"));

    if (!fileName.isEmpty())
    {
        const QString basename = fileName.left(fileName.length() - 3);
        const QString oggfile = basename + "ogg";
        const QString ogmfile = basename + "ogm";
        const QString ogvfile = basename + "ogv";
        const QString txtfile = basename + "txt";
        const QString txafile = basename + "txa";

        subtitles[0].clear();
        subtitles[1].clear();

        if (QFile::exists(oggfile))
            openMovie(oggfile);
        if (QFile::exists(ogmfile))
            openMovie(ogmfile);
        if (QFile::exists(ogvfile))
            openMovie(ogvfile);
        if (QFile::exists(txtfile))
        {
            openSubtitle(txtfile, 0);
            currentPath = fileName;
        }
        if (QFile::exists(txafile))
        {
            openSubtitle(txafile, 1);
            currentPath = fileName;
        }

        selectTrack(0);

        QSettings().setValue("lastUsedDir", QFileInfo(fileName).dir().path());
    }
}

void MainWindow::actionSave()
{
    if (currentPath.isEmpty())
    {
        actionSaveAs();
    }
    else
    {
        saveSubtitles(currentPath);
    }
}

void MainWindow::actionSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Subtitle file"), (currentPath.isEmpty()?QDesktopServices::storageLocation(QDesktopServices::HomeLocation):QFileInfo(currentPath).dir().path()));

    if (!fileName.isEmpty())
    {
        saveSubtitles(fileName);
    }
}

void MainWindow::actionAboutApplication()
{
    QMessageBox::about(this, tr("About Subtitles Editor"), QString(tr("<b>Subtitles Editor %1</b><br />Subtitles previewer and editor for Warzone 2100.").arg(QApplication::instance()->applicationVersion())));
}

void MainWindow::stateChanged(Phonon::State state)
{
    switch (state)
    {
        case Phonon::ErrorState:
            if (mediaObject->errorType() == Phonon::FatalError)
            {
                QMessageBox::warning(this, tr("Fatal Error"), mediaObject->errorString());
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), mediaObject->errorString());
            }
            break;
        case Phonon::PlayingState:
            ui->actionPlayPause->setEnabled(true);
            ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            ui->actionStop->setEnabled(true);
            break;
        case Phonon::StoppedState:
            ui->actionPlayPause->setEnabled(true);
            ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            ui->actionStop->setEnabled(false);
            timeLabel->setText(QString("00:00.0 / %1").arg(timeToString(mediaObject->totalTime())));
            break;
        case Phonon::PausedState:
            ui->actionPlayPause->setEnabled(true);
            ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            ui->actionStop->setEnabled(true);
            break;
        default:
            break;
    }
}

void MainWindow::tick()
{
    timeLabel->setText(QString("%1 / %2").arg(timeToString(mediaObject->currentTime())).arg(timeToString(mediaObject->totalTime())));

    QList<Subtitle> currentBottomSubtitles;
    QList<Subtitle> currentTopSubtitles;
    qint64 currentTime = (mediaObject->currentTime() / 1000);

    for (int i = 0; i < subtitles[0].count(); ++i)
    {
        if (subtitles[0].at(i).beginTime < currentTime && subtitles[0].at(i).endTime > currentTime)
        {
            currentBottomSubtitles.append(subtitles[0].at(i));
        }
    }

    for (int i = 0; i < subtitles[1].count(); ++i)
    {
        if (subtitles[1].at(i).beginTime < currentTime && subtitles[1].at(i).endTime > currentTime)
        {
            currentTopSubtitles.append(subtitles[1].at(i));
        }
    }

    subtitlesWidget->showText(currentBottomSubtitles, currentTopSubtitles);
    subtitlesWidget->update();
}

void MainWindow::selectTrack(int track)
{
    currentSubtitle = 0;
    currentTrack = track;

    showSubtitle();
}

void MainWindow::addSubtitle()
{
    Subtitle subtitle;
    subtitle.positionX = 20;
    subtitle.positionY = 432;
    subtitle.beginTime = 0;
    subtitle.endTime = 0;

    subtitles[currentTrack].insert(currentSubtitle, subtitle);

    nextSubtitle();
}

void MainWindow::removeSubtitle()
{
    if (QMessageBox::question(this, tr("Remove Subtitle"), tr("Are you sure that you want to remove this subtitle?")))
    {
        subtitles[currentTrack].removeAt(currentSubtitle);

        showSubtitle();
    }
}

void MainWindow::previousSubtitle()
{
    --currentSubtitle;

    showSubtitle();
}

void MainWindow::nextSubtitle()
{
    ++currentSubtitle;

    showSubtitle();
}

void MainWindow::showSubtitle()
{
    disconnect(ui->subtitleTextEdit, SIGNAL(textChanged()), this, SLOT(updateSubtitle()));
    disconnect(ui->xPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
    disconnect(ui->yPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
    disconnect(ui->beginTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
    disconnect(ui->lengthTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));

    if (currentTrack < 0 || currentTrack > 1)
    {
        currentTrack = 0;
    }

    if (currentSubtitle < 0)
    {
        if (subtitles[currentTrack].count() > 0)
        {
            currentSubtitle = (subtitles[currentTrack].count() - 1);
        }
        else
        {
            currentSubtitle = 0;
        }
    }
    else if (currentSubtitle >= subtitles[currentTrack].count())
    {
        currentSubtitle = 0;
    }    

    if (currentSubtitle < subtitles[currentTrack].count())
    {
        QTime nullTime(0, 0, 0, 0);

        ui->subtitleTextEdit->setPlainText(subtitles[currentTrack].at(currentSubtitle).text);
        ui->beginTimeEdit->setTime(nullTime.addMSecs((subtitles[currentTrack].at(currentSubtitle).beginTime * 1000)));
        ui->lengthTimeEdit->setTime(nullTime.addMSecs(((subtitles[currentTrack].at(currentSubtitle).endTime * 1000) - (subtitles[currentTrack].at(currentSubtitle).beginTime * 1000))));
        ui->xPositionSpinBox->setValue(subtitles[currentTrack].at(currentSubtitle).positionX);
        ui->yPositionSpinBox->setValue(subtitles[currentTrack].at(currentSubtitle).positionY);
    }
    else
    {
        ui->subtitleTextEdit->clear();
        ui->beginTimeEdit->setTime(QTime());
        ui->lengthTimeEdit->setTime(QTime());
        ui->xPositionSpinBox->setValue(0);
        ui->yPositionSpinBox->setValue(0);
    }

    connect(ui->subtitleTextEdit, SIGNAL(textChanged()), this, SLOT(updateSubtitle()));
    connect(ui->xPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
    connect(ui->yPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
    connect(ui->beginTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
    connect(ui->lengthTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
}

void MainWindow::updateSubtitle()
{
    if (currentSubtitle == 0 && subtitles[currentTrack].count() == 0)
    {
        Subtitle subtitle;

        subtitles[currentTrack].append(subtitle);
    }

    subtitles[currentTrack][currentSubtitle].text = ui->subtitleTextEdit->toPlainText();
    subtitles[currentTrack][currentSubtitle].positionX = ui->xPositionSpinBox->value();
    subtitles[currentTrack][currentSubtitle].positionY = ui->yPositionSpinBox->value();
    subtitles[currentTrack][currentSubtitle].beginTime = timeToSeconds(ui->beginTimeEdit->time());
    subtitles[currentTrack][currentSubtitle].endTime = (timeToSeconds(ui->beginTimeEdit->time()) + timeToSeconds(ui->lengthTimeEdit->time()));
}

void MainWindow::rescaleSubtitles()
{
    bool ok = false;
    double scale = QInputDialog::getDouble(this, tr("Rescale Subtitles"), tr("Insert time multiplier:"), 1, 0, 100, 5, &ok);

    for (int i = 0; i < subtitles[0].count(); ++i)
    {
        subtitles[0][i].beginTime *= scale;
        subtitles[0][i].endTime *= scale;
    }

    for (int i = 0; i < subtitles[1].count(); ++i)
    {
        subtitles[1][i].beginTime *= scale;
        subtitles[1][i].endTime *= scale;
    }

    showSubtitle();
}

void MainWindow::playPause()
{
    if (mediaObject->state() == Phonon::PlayingState)
    {
         mediaObject->pause();
    }
    else
    {
         mediaObject->play();
    }
}

