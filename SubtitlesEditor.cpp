/***********************************************************************************
* Warzone 2100 Subtitles Editor.
* Copyright (C) 2010 - 2011 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
***********************************************************************************/

#include "SubtitlesEditor.h"
#include "ui_SubtitlesEditor.h"

#include <QtCore/QSettings>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QDesktopServices>

#include <Phonon/AudioOutput>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
	m_ui(new Ui::MainWindow),
	m_currentSubtitle(0),
	m_currentTrack(0)
{
	m_subtitles.append(QList<Subtitle>());
	m_subtitles.append(QList<Subtitle>());

	m_ui->setupUi(this);

	Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(this);
	audioOutput->setVolume(QSettings().value("volume", 0.8).toReal());

	m_mediaObject = new Phonon::MediaObject(this);
	m_mediaObject->setTickInterval(100);

	Phonon::createPath(m_mediaObject, m_ui->videoWidget);
	Phonon::createPath(m_mediaObject, audioOutput);

	m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-start", style()->standardIcon(QStyle::SP_MediaPlay)));
	m_ui->actionPlayPause->setShortcut(tr("Space"));
	m_ui->actionPlayPause->setDisabled(true);

	m_ui->actionStop->setIcon(QIcon::fromTheme("media-playback-stop", style()->standardIcon(QStyle::SP_MediaStop)));
	m_ui->actionStop->setDisabled(true);

	m_fileNameLabel = new QLabel(tr("No file loaded"), this);
	m_fileNameLabel->setMaximumWidth(300);
	m_timeLabel = new QLabel("00:00.0 / 00:00.0", this);

	m_ui->actionOpen->setIcon(QIcon::fromTheme("document-open", style()->standardIcon(QStyle::SP_DirOpenIcon)));
	m_ui->actionSave->setIcon(QIcon::fromTheme("document-save", style()->standardIcon(QStyle::SP_DialogSaveButton)));
	m_ui->actionSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
	m_ui->actionExit->setIcon(QIcon::fromTheme("application-exit", style()->standardIcon(QStyle::SP_DialogCloseButton)));
	m_ui->actionAdd->setIcon(QIcon::fromTheme("list-add"));
	m_ui->actionRemove->setIcon(QIcon::fromTheme("list-remove"));
	m_ui->actionPrevious->setIcon(QIcon::fromTheme("go-previous"));
	m_ui->actionNext->setIcon(QIcon::fromTheme("go-next"));
	m_ui->actionRescale->setIcon(QIcon::fromTheme("chronometer"));
	m_ui->actionAboutApplication->setIcon(QIcon::fromTheme("help-about"));
	m_ui->playPauseButton->setDefaultAction(m_ui->actionPlayPause);
	m_ui->stopButton->setDefaultAction(m_ui->actionStop);
	m_ui->addButton->setDefaultAction(m_ui->actionAdd);
	m_ui->removeButton->setDefaultAction(m_ui->actionRemove);
	m_ui->previousButton->setDefaultAction(m_ui->actionPrevious);
	m_ui->nextButton->setDefaultAction(m_ui->actionNext);
	m_ui->seekSlider->setMediaObject(m_mediaObject);
	m_ui->volumeSlider->setAudioOutput(audioOutput);
	m_ui->statusBar->addPermanentWidget(m_fileNameLabel);
	m_ui->statusBar->addPermanentWidget(m_timeLabel);

	resize(QSettings().value("Window/size", size()).toSize());
	move(QSettings().value("Window/position", pos()).toPoint());
	restoreState(QSettings().value("Window/state", QByteArray()).toByteArray());

	connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
	connect(m_ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
	connect(m_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
	connect(m_ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_ui->actionAdd, SIGNAL(triggered()), this, SLOT(addSubtitle()));
	connect(m_ui->actionRemove, SIGNAL(triggered()), this, SLOT(removeSubtitle()));
	connect(m_ui->actionPrevious, SIGNAL(triggered()), this, SLOT(previousSubtitle()));
	connect(m_ui->actionNext, SIGNAL(triggered()), this, SLOT(nextSubtitle()));
	connect(m_ui->actionRescale, SIGNAL(triggered()), this, SLOT(rescaleSubtitles()));
	connect(m_ui->actionPlayPause, SIGNAL(triggered()), this, SLOT(playPause()));
	connect(m_ui->actionStop, SIGNAL(triggered()), m_mediaObject, SLOT(stop()));
	connect(m_ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));
	connect(m_ui->actionAboutApplication, SIGNAL(triggered()), this, SLOT(actionAboutApplication()));
	connect(m_ui->trackComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectTrack(int)));
	connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State)));
	connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick()));
	connect(m_mediaObject, SIGNAL(finished()), this, SLOT(finished()));
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::changeEvent(QEvent *event)
{
	QMainWindow::changeEvent(event);

	switch (event->type())
	{
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QSettings settings;
	settings.setValue("Window/size", size());
	settings.setValue("Window/position", pos());
	settings.setValue("Window/state", saveState());
	settings.setValue("volume", m_ui->volumeSlider->audioOutput()->volume());

	event->accept();
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
		if (m_subtitles[i].isEmpty())
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

		for (int j = 0; j < m_subtitles[i].count(); ++j)
		{
			textStream << QString("%1\t%2\t\t%3\t%4\t_(\"%5\")\n").arg(m_subtitles[i][j].positionX).arg(m_subtitles[i][j].positionY).arg(m_subtitles[i][j].beginTime).arg(m_subtitles[i][j].endTime).arg(m_subtitles[i][j].text);

			if ((j + 1) < m_subtitles[i].count() && m_subtitles[i][j].beginTime != m_subtitles[i][j + 1].beginTime)
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
	m_mediaObject->setCurrentSource(Phonon::MediaSource(filename));

	m_ui->actionPlayPause->setEnabled(true);

	m_fileNameLabel->setText(QFileInfo(filename).fileName());
	m_timeLabel->setText(QString("00:00.0 / %1").arg(timeToString(m_mediaObject->totalTime())));
}

void MainWindow::openSubtitle(const QString &filename, int index)
{
	m_subtitles[index] = readSubtitles(filename);
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

		m_subtitles[0].clear();
		m_subtitles[1].clear();

		if (QFile::exists(oggfile))
		{
			openMovie(oggfile);
		}

		if (QFile::exists(ogmfile))
		{
			openMovie(ogmfile);
		}

		if (QFile::exists(ogvfile))
		{
			openMovie(ogvfile);
		}

		if (QFile::exists(txtfile))
		{
			openSubtitle(txtfile, 0);

			m_currentPath = fileName;
		}

		if (QFile::exists(txafile))
		{
			openSubtitle(txafile, 1);

			m_currentPath = fileName;
		}

		selectTrack(0);

		QSettings().setValue("lastUsedDir", QFileInfo(fileName).dir().path());
	}
}

void MainWindow::actionSave()
{
	if (m_currentPath.isEmpty())
	{
		actionSaveAs();
	}
	else
	{
		saveSubtitles(m_currentPath);
	}
}

void MainWindow::actionSaveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Subtitle file"), (m_currentPath.isEmpty()?QDesktopServices::storageLocation(QDesktopServices::HomeLocation):QFileInfo(m_currentPath).dir().path()));

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
			if (m_mediaObject->errorType() == Phonon::FatalError)
			{
				QMessageBox::warning(this, tr("Fatal Error"), m_mediaObject->errorString());
			}
			else
			{
				QMessageBox::warning(this, tr("Error"), m_mediaObject->errorString());
			}

			m_ui->actionPlayPause->setEnabled(false);
		case Phonon::StoppedState:
			m_ui->actionPlayPause->setText(tr("Play"));
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-play", style()->standardIcon(QStyle::SP_MediaPlay)));
			m_ui->actionStop->setEnabled(false);
			m_timeLabel->setText(QString("00:00.0 / %1").arg(timeToString(m_mediaObject->totalTime())));
			break;
		case Phonon::PlayingState:
			m_ui->actionPlayPause->setText(tr("Pause"));
			m_ui->actionPlayPause->setEnabled(true);
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-pause", style()->standardIcon(QStyle::SP_MediaPause)));
			m_ui->actionStop->setEnabled(true);
			break;
		case Phonon::PausedState:
			m_ui->actionPlayPause->setText(tr("Play"));
			m_ui->actionPlayPause->setEnabled(true);
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-play", style()->standardIcon(QStyle::SP_MediaPlay)));
			m_ui->actionStop->setEnabled(true);
			break;
		default:
			break;
	}
}

void MainWindow::finished()
{
	m_mediaObject->stop();
}

void MainWindow::tick()
{
	m_timeLabel->setText(QString("%1 / %2").arg(timeToString(m_mediaObject->currentTime())).arg(timeToString(m_mediaObject->totalTime())));

	QString currentBottomSubtitles;
	QString currentTopSubtitles;
	qint64 currentTime = (m_mediaObject->currentTime() / 1000);

	for (int i = 0; i < m_subtitles[0].count(); ++i)
	{
		if (m_subtitles[0].at(i).beginTime < currentTime && m_subtitles[0].at(i).endTime > currentTime)
		{
			currentBottomSubtitles.append(m_subtitles[0].at(i).text);
			currentBottomSubtitles.append(" | ");
			m_currentSubtitle = i;
		}
	}
	for (int i = 0; i < m_subtitles[1].count(); ++i)
	{
		if (m_subtitles[1].at(i).beginTime < currentTime && m_subtitles[1].at(i).endTime > currentTime)
		{
			currentTopSubtitles.append(m_subtitles[1].at(i).text);
			currentTopSubtitles.append(" | ");
		}
	}

	showSubtitle();
	currentTopSubtitles = currentTopSubtitles.left(currentTopSubtitles.length() - 3);
	currentBottomSubtitles = currentBottomSubtitles.left(currentBottomSubtitles.length() - 3);
	m_ui->bottomSubs->setText(currentBottomSubtitles);
	m_ui->topSubs->setText(currentTopSubtitles);
}

void MainWindow::selectTrack(int track)
{
	m_currentSubtitle = 0;
	m_currentTrack = track;

	showSubtitle();
}

void MainWindow::addSubtitle()
{
	Subtitle subtitle;
	subtitle.positionX = 20;
	subtitle.positionY = 432;
	subtitle.beginTime = 0;
	subtitle.endTime = 0;

	m_subtitles[m_currentTrack].insert(m_currentSubtitle, subtitle);

	nextSubtitle();
}

void MainWindow::removeSubtitle()
{
	if (QMessageBox::question(this, tr("Remove Subtitle"), tr("Are you sure that you want to remove this subtitle?")))
	{
		m_subtitles[m_currentTrack].removeAt(m_currentSubtitle);

		showSubtitle();
	}
}

void MainWindow::previousSubtitle()
{
	--m_currentSubtitle;

	showSubtitle();
}

void MainWindow::nextSubtitle()
{
	++m_currentSubtitle;

	showSubtitle();
}

void MainWindow::showSubtitle()
{
	disconnect(m_ui->subtitleTextEdit, SIGNAL(textChanged()), this, SLOT(updateSubtitle()));
	disconnect(m_ui->xPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	disconnect(m_ui->yPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	disconnect(m_ui->beginTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
	disconnect(m_ui->lengthTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));

	if (m_currentTrack < 0 || m_currentTrack > 1)
	{
		m_currentTrack = 0;
	}

	if (m_currentSubtitle < 0)
	{
		if (m_subtitles[m_currentTrack].count() > 0)
		{
			m_currentSubtitle = (m_subtitles[m_currentTrack].count() - 1);
		}
		else
		{
			m_currentSubtitle = 0;
		}
	}
	else if (m_currentSubtitle >= m_subtitles[m_currentTrack].count())
	{
		m_currentSubtitle = 0;
	}

	if (m_currentSubtitle < m_subtitles[m_currentTrack].count())
	{
		QTime nullTime(0, 0, 0, 0);

		m_ui->subtitleTextEdit->setPlainText(m_subtitles[m_currentTrack].at(m_currentSubtitle).text);
		m_ui->beginTimeEdit->setTime(nullTime.addMSecs((m_subtitles[m_currentTrack].at(m_currentSubtitle).beginTime * 1000)));
		m_ui->lengthTimeEdit->setTime(nullTime.addMSecs(((m_subtitles[m_currentTrack].at(m_currentSubtitle).endTime * 1000) - (m_subtitles[m_currentTrack].at(m_currentSubtitle).beginTime * 1000))));
		m_ui->xPositionSpinBox->setValue(m_subtitles[m_currentTrack].at(m_currentSubtitle).positionX);
		m_ui->yPositionSpinBox->setValue(m_subtitles[m_currentTrack].at(m_currentSubtitle).positionY);
	}
	else
	{
		m_ui->subtitleTextEdit->clear();
		m_ui->beginTimeEdit->setTime(QTime());
		m_ui->lengthTimeEdit->setTime(QTime());
		m_ui->xPositionSpinBox->setValue(0);
		m_ui->yPositionSpinBox->setValue(0);
	}

	connect(m_ui->subtitleTextEdit, SIGNAL(textChanged()), this, SLOT(updateSubtitle()));
	connect(m_ui->xPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	connect(m_ui->yPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	connect(m_ui->beginTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
	connect(m_ui->lengthTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
}

void MainWindow::updateSubtitle()
{
	if (m_currentSubtitle == 0 && m_subtitles[m_currentTrack].count() == 0)
	{
		Subtitle subtitle;

		m_subtitles[m_currentTrack].append(subtitle);
	}

	m_subtitles[m_currentTrack][m_currentSubtitle].text = m_ui->subtitleTextEdit->toPlainText();
	m_subtitles[m_currentTrack][m_currentSubtitle].positionX = m_ui->xPositionSpinBox->value();
	m_subtitles[m_currentTrack][m_currentSubtitle].positionY = m_ui->yPositionSpinBox->value();
	m_subtitles[m_currentTrack][m_currentSubtitle].beginTime = timeToSeconds(m_ui->beginTimeEdit->time());
	m_subtitles[m_currentTrack][m_currentSubtitle].endTime = (timeToSeconds(m_ui->beginTimeEdit->time()) + timeToSeconds(m_ui->lengthTimeEdit->time()));
}

void MainWindow::rescaleSubtitles()
{
	bool ok = false;
	double scale = QInputDialog::getDouble(this, tr("Rescale Subtitles"), tr("Insert time multiplier:"), 1, 0, 100, 5, &ok);

	for (int i = 0; i < m_subtitles[0].count(); ++i)
	{
		m_subtitles[0][i].beginTime *= scale;
		m_subtitles[0][i].endTime *= scale;
	}

	for (int i = 0; i < m_subtitles[1].count(); ++i)
	{
		m_subtitles[1][i].beginTime *= scale;
		m_subtitles[1][i].endTime *= scale;
	}

	showSubtitle();
}

void MainWindow::playPause()
{
	if (m_mediaObject->state() == Phonon::PlayingState)
	{
		m_mediaObject->pause();
	}
	else
	{
		m_mediaObject->play();
	}
}
