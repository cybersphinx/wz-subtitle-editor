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

#ifndef SUBTITLESEDITOR_H
#define SUBTITLESEDITOR_H

#include <QtGui/QMainWindow>
#include <QtGui/QLabel>

#include <Phonon/MediaObject>

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
	void closeEvent(QCloseEvent *event);
	QString timeToString(qint64 time);
	QList<Subtitle> readSubtitles(const QString &fileName);
	double timeToSeconds(QTime time);
	bool saveSubtitles(QString fileName);

private:
	Ui::MainWindow *m_ui;
	Phonon::MediaObject *m_mediaObject;
	QString m_currentPath;
	QLabel *m_fileNameLabel;
	QLabel *m_timeLabel;
	QList<QList<Subtitle> > m_subtitles;
	int m_currentSubtitle;
	int m_currentTrack;

	void openMovie(const QString &filename);
	void openSubtitle(const QString &filename, int index);
};

#endif // SUBTITLESEDITOR_H
