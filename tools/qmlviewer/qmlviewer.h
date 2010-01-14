/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QMLVIEWER_H
#define QMLVIEWER_H

#include <QMainWindow>
#include <QMenuBar>
#include <private/qmltimer_p.h>
#include <QTime>
#include <QList>

QT_BEGIN_NAMESPACE

class QmlView;
class PreviewDeviceSkin;
class QmlGraphicsTestEngine;
class QProcess;
class RecordingDialog;
class QmlGraphicsTester;
class QNetworkReply;

class QmlViewer
#if defined(Q_OS_SYMBIAN)
    : public QMainWindow
#else
    : public QWidget
#endif
{
Q_OBJECT
public:
    QmlViewer(QWidget *parent=0, Qt::WindowFlags flags=0);

    enum ScriptOption {
        Play = 0x00000001,
        Record = 0x00000002,
        TestImages = 0x00000004,
        TestErrorProperty = 0x00000008,
        SaveOnExit = 0x00000010,
        ExitOnComplete = 0x00000020,
        ExitOnFailure = 0x00000040
    };
    Q_DECLARE_FLAGS(ScriptOptions, ScriptOption)
    void setScript(const QString &s) { m_script = s; }
    void setScriptOptions(ScriptOptions opt) { m_scriptOptions = opt; }
    void setRecordDither(const QString& s) { record_dither = s; }
    void setRecordRate(int fps);
    void setRecordFile(const QString&);
    void setRecordArgs(const QStringList&);
    void setRecording(bool on);
    bool isRecording() const { return recordTimer.isRunning(); }
    void setAutoRecord(int from, int to);
    void setDeviceKeys(bool);
    void setNetworkCacheSize(int size);
    void addLibraryPath(const QString& lib);
    void setUseGL(bool use);
    void setUseNativeFileBrowser(bool);

    QStringList builtinSkins() const;

    QMenuBar *menuBar() const;

public slots:
    void sceneResized(QSize size);
    void open(const QString&);
    void openWgt(const QString&);
    void openQml(const QString&);
    void openFile();
    void reload();
    void takeSnapShot();
    void toggleRecording();
    void toggleRecordingWithSelection();
    void ffmpegFinished(int code);
    void setSkin(const QString& skinDirectory);
    void showProxySettings ();
    void proxySettingsChanged ();
    void setScaleView();
    void executeErrors();
    void setSlowMode(bool);
    void launch(const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *);

    void createMenu(QMenuBar *menu, QMenu *flatmenu);

private slots:
    void autoStartRecording();
    void autoStopRecording();
    void recordFrame();
    void chooseRecordingOptions();
    void pickRecordingFile();
    void setScaleSkin();
    void adjustSizeSlot();
    void setPortrait();
    void setLandscape();
    void toggleOrientation();
    void startNetwork();
    void toggleFullScreen();
    void unpackWgt();

private:
    void setupProxy();
    QString getVideoFileName();

    PreviewDeviceSkin *skin;
    QSize skinscreensize;
    QmlView *canvas;
    QString currentFileOrUrl;
    QmlTimer recordTimer;
    QString frame_fmt;
    QImage frame;
    QList<QImage*> frames;
    QProcess* frame_stream;
    QmlTimer autoStartTimer;
    QmlTimer autoStopTimer;
    QString record_dither;
    QString record_file;
    QSize record_outsize;
    QStringList record_args;
    int record_rate;
    int record_autotime;
    bool devicemode;
    QAction *recordAction;
    QString currentSkin;
    bool scaleSkin;
    mutable QMenuBar *mb;
    RecordingDialog *recdlg;

    void senseImageMagick();
    void senseFfmpeg();
    QWidget *ffmpegHelpWindow;
    bool ffmpegAvailable;
    bool convertAvailable;

    QAction *portraitOrientation;
    QAction *landscapeOrientation;

    QString m_script;
    ScriptOptions m_scriptOptions;
    QmlGraphicsTester *tester;

    QNetworkReply *wgtreply;
    QString wgtdir;

    bool useQmlFileBrowser;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QmlViewer::ScriptOptions)

QT_END_NAMESPACE

#endif
