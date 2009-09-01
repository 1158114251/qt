/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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
** In addition, as a special exception, Nokia gives you certain
** additional rights.  These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
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



#include <QtTest/QtTest>
#include <QtCore/qlocale.h>
#include <qaudiooutput.h>
#include <qaudiodeviceinfo.h>
#include <qaudio.h>
#include <qaudioformat.h>


class tst_QAudioOutput : public QObject
{
    Q_OBJECT
public:
    tst_QAudioOutput(QObject* parent=0) : QObject(parent) {}

private slots:
    void initTestCase();
    void settings();
    void notifyInterval();
    void pullFile();
    void pushFile();

private:
    bool          available;
    QAudioFormat  format;
    QAudioOutput* audio;
};

void tst_QAudioOutput::initTestCase()
{
    format.setFrequency(8000);
    format.setChannels(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    // Only perform tests if audio output device exists!
    QList<QAudioDeviceId> devices = QAudioDeviceInfo::deviceList(QAudio::AudioOutput);
    if(devices.size() > 0)
        available = true;
    else {
        qWarning()<<"NOTE: no audio output device found, no test will be performed";
        available = false;
    }
    audio = new QAudioOutput(format, this);
}

void tst_QAudioOutput::settings()
{
    if(available) {
        QAudioFormat f = audio->format();

        QVERIFY(format.channels() == f.channels());
        QVERIFY(format.frequency() == f.frequency());
        QVERIFY(format.sampleSize() == f.sampleSize());
        QVERIFY(format.codec() == f.codec());
        QVERIFY(format.byteOrder() == f.byteOrder());
        QVERIFY(format.sampleType() == f.sampleType());
    }
}

void tst_QAudioOutput::notifyInterval()
{
    if(available) {
        QVERIFY(audio->notifyInterval() == 1000);   // Default

        audio->setNotifyInterval(500);
        QVERIFY(audio->notifyInterval() == 500);    // Custom

        audio->setNotifyInterval(1000);             // reset
    }
}

void tst_QAudioOutput::pullFile()
{
    if(available) {
        QFile filename(SRCDIR "4.wav");
        QVERIFY(filename.exists());
        filename.open(QIODevice::ReadOnly);

        QSignalSpy readSignal(audio, SIGNAL(notify()));
        audio->setNotifyInterval(100);
        audio->start(&filename);

        QTestEventLoop::instance().enterLoop(1);
        // 4.wav is a little less than 700ms, so notify should fire 6 times!
        QVERIFY(readSignal.count() >= 6);
        QVERIFY(audio->totalTime() == 692250);

        audio->stop();
        filename.close();
    }
}

void tst_QAudioOutput::pushFile()
{
    if(available) {
        QFile filename(SRCDIR "4.wav");
        QVERIFY(filename.exists());
        filename.open(QIODevice::ReadOnly);

        const qint64 fileSize = filename.size();

        QIODevice* feed = audio->start(0);

        char* buffer = new char[fileSize];
        filename.read(buffer, fileSize);

        qint64 counter=0;
        qint64 written=0;
        while(written < fileSize) {
            written+=feed->write(buffer+written,fileSize-written);
            QTest::qWait(20);
            counter++;
        }
        QTestEventLoop::instance().enterLoop(1);

        QVERIFY(written == fileSize);
        QVERIFY(audio->totalTime() == 692250);

        audio->stop();
        filename.close();
        delete [] buffer;
        delete audio;
    }
}

QTEST_MAIN(tst_QAudioOutput)

#include "tst_qaudiooutput.moc"
