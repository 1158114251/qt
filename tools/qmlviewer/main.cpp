/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qml.h"
#include "qmlviewer.h"
#include <QWidget>
#include <QDir>
#include "qfxtestengine.h"
#include <QApplication>


void usage()
{
    qWarning("Usage: qmlviewer [options] <filename>");
    qWarning(" ");
    qWarning(" options:");
    qWarning("  -v, -version ............................. display version");
    qWarning("  -frameless ............................... run with no window frame");
    qWarning("  -skin <qvfbskindir> ...................... run with a skin window frame");
    qWarning("  -recorddither ordered|threshold|floyd .... set dither mode used for recording");
    qWarning("  -recordperiod <milliseconds> ............. set time between recording frames");
    qWarning("  -autorecord [from-]<tomilliseconds> ...... set recording to start and stop automatically");
    qWarning("  -devicekeys .............................. use numeric keys (see F1)");
    qWarning("  -cache ................................... enable a disk cache of remote content");
    qWarning("  -recordtest <directory> .................. record an autotest");
    qWarning("  -runtest <directory> ..................... run a previously recorded test");
    qWarning(" ");
    qWarning(" Press F1 for interactive help");
    exit(1);
}

int main(int argc, char ** argv)
{
    //### default to using raster graphics backend for now
    int newargc = argc + 2;
    char **newargv;
    newargv = new char * [newargc];
    for (int i = 0; i < argc; ++i) {
        newargv[i] = argv[i];
        if (!qstrcmp(argv[i], "-graphicssystem")) {
            newargc -= 2;
            break;
        }
    }
    char system[] = "-graphicssystem";
    newargv[argc] = system;
    char raster[] = "raster";
    newargv[argc+1] = raster;


    QApplication app(newargc, newargv);
    app.setApplicationName("viewer");

    bool frameless = false;
    QString fileName;
    int period = 0;
    int autorecord_from = 0;
    int autorecord_to = 0;
    QString dither = "threshold";
    QString skin;
    bool devkeys = false;
    bool cache = false;
    QFxTestEngine::TestMode testMode = QFxTestEngine::NoTest;
    QString testDir;

    for (int i = 1; i < newargc; ++i) {
        QString arg = newargv[i];
        if (arg == "-frameless") {
            frameless = true;
        } else if (arg == "-skin") {
            skin = QString(argv[++i]);
        } else if (arg == "-cache") {
            cache = true;
        } else if (arg == "-recordperiod") {
            period = QString(argv[++i]).toInt();
        } else if (arg == "-autorecord") {
            QString range = QString(argv[++i]);
            int dash = range.indexOf('-');
            if (dash > 0)
                autorecord_from = range.left(dash).toInt();
            autorecord_to = range.mid(dash+1).toInt();
        } else if (arg == "-devicekeys") {
            devkeys = true;
        } else if (arg == "-recordtest") {
            testMode = QFxTestEngine::RecordTest;
            if(i + 1 >= newargc) 
                usage();
            testDir = newargv[i + 1];
            ++i;
        } else if (arg == "-runtest") {
            testMode = QFxTestEngine::PlaybackTest;
            if(i + 1 >= newargc) 
                usage();
            testDir = newargv[i + 1];
            ++i;
        } else if (arg == QLatin1String("-v") || arg == QLatin1String("-version")) {
            fprintf(stderr, "Qt Declarative UI Viewer version %s\n", QT_VERSION_STR);
            return 0;
        } else if (arg[0] != '-') {
            fileName = arg;
        } else if (1 || arg == "-help") {
            usage();
        }
    }

    if (fileName.isEmpty())
        usage();

    QmlViewer viewer(testMode, testDir, 0, frameless ? Qt::FramelessWindowHint : Qt::Widget);
    viewer.setCacheEnabled(cache);
    viewer.openQml(fileName);
    if (period>0)
        viewer.setRecordPeriod(period);
    if (autorecord_to)
        viewer.setAutoRecord(autorecord_from,autorecord_to);
    if (QDir(skin).exists())
        viewer.setSkin(skin);
    if (devkeys)
        viewer.setDeviceKeys(true);
    viewer.setRecordDither(dither);
    viewer.show();

    return app.exec();
}

