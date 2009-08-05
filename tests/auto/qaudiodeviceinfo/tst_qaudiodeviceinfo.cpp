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
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
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
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>
#include <QtCore/qlocale.h>
#include <qaudiodeviceinfo.h>

#include <QStringList>
#include <QList>


class tst_QAudioDeviceInfo : public QObject
{
    Q_OBJECT
public:
    tst_QAudioDeviceInfo(QObject* parent=0) : QObject(parent) {}

private slots:
    void checkAvailableDefaultInput();
    void checkAvailableDefaultOutput();
    void outputList();
    void codecs();
    void channels();
    void sampleSizes();
    void byteOrders();
    void sampleTypes();
    void frequencies();
    void isformat();
    void preferred();
    void nearest();

private:
    QAudioDeviceInfo* device;
};

void tst_QAudioDeviceInfo::checkAvailableDefaultInput()
{
    QVERIFY(!QAudioDeviceInfo::defaultInputDevice().isNull());
}

void tst_QAudioDeviceInfo::checkAvailableDefaultOutput()
{
    QVERIFY(!QAudioDeviceInfo::defaultOutputDevice().isNull());
}

void tst_QAudioDeviceInfo::outputList()
{
    QList<QAudioDeviceId> devices = QAudioDeviceInfo::deviceList(QAudio::AudioOutput);
    QVERIFY(devices.size() > 0);
    device = new QAudioDeviceInfo(devices.at(0), this);
}

void tst_QAudioDeviceInfo::codecs()
{
    QStringList avail = device->supportedCodecs();
    QVERIFY(avail.size() > 0);
}

void tst_QAudioDeviceInfo::channels()
{
    QList<int> avail = device->supportedChannels();
    QVERIFY(avail.size() > 0);
}

void tst_QAudioDeviceInfo::sampleSizes()
{
    QList<int> avail = device->supportedSampleSizes();
    QVERIFY(avail.size() > 0);
}

void tst_QAudioDeviceInfo::byteOrders()
{
    QList<QAudioFormat::Endian> avail = device->supportedByteOrders();
    QVERIFY(avail.size() > 0);
}

void tst_QAudioDeviceInfo::sampleTypes()
{
    QList<QAudioFormat::SampleType> avail = device->supportedSampleTypes();
    QVERIFY(avail.size() > 0);
}

void tst_QAudioDeviceInfo::frequencies()
{
    QList<int> avail = device->supportedFrequencies();
    QVERIFY(avail.size() > 0);
}

void tst_QAudioDeviceInfo::isformat()
{
    QAudioFormat     format;
    format.setFrequency(44100);
    format.setChannels(2);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");

    // Should always be true for these format
    QVERIFY(device->isFormatSupported(format));
}

void tst_QAudioDeviceInfo::preferred()
{
    QAudioFormat format = device->preferredFormat();
    QVERIFY(format.frequency() == 44100);
    QVERIFY(format.channels() == 2);
}

void tst_QAudioDeviceInfo::nearest()
{
    QAudioFormat format1, format2;
    format1.setFrequency(8000);
    format2 = device->nearestFormat(format1);
    QVERIFY(format2.frequency() == 44100);
}

QTEST_MAIN(tst_QAudioDeviceInfo)

#include "tst_qaudiodeviceinfo.moc"
