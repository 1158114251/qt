/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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

#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <math.h>

#ifdef Q_OS_UNIX
#  include <pthread.h>
#  include <errno.h>
typedef pthread_mutex_t NativeMutexType;
void NativeMutexInitialize(NativeMutexType *mutex)
{
    pthread_mutex_init(mutex, NULL);
}
void NativeMutexDestroy(NativeMutexType *mutex)
{
    pthread_mutex_destroy(mutex);
}
void NativeMutexLock(NativeMutexType *mutex)
{
    pthread_mutex_lock(mutex);
}
void NativeMutexUnlock(NativeMutexType *mutex)
{
    pthread_mutex_unlock(mutex);
}
#elif defined(Q_OS_WIN)
#  define _WIN32_WINNT 0x0400
#  include <windows.h>
typedef CRITICAL_SECTION NativeMutexType;
void NativeMutexInitialize(NativeMutexType *mutex)
{
    InitializeCriticalSection(mutex);
}
void NativeMutexDestroy(NativeMutexType *mutex)
{
    DeleteCriticalSection(mutex);
}
void NativeMutexLock(NativeMutexType *mutex)
{
    EnterCriticalSection(mutex);
}
void NativeMutexUnlock(NativeMutexType *mutex)
{
    LeaveCriticalSection(mutex);
}
#endif

//TESTED_FILES=

class tst_QMutex : public QObject
{
    Q_OBJECT

    int threadCount;

public:
    tst_QMutex()
    {
        // at least 2 threads, even on single cpu/core machines
        threadCount = qMax(2, QThread::idealThreadCount());
        qDebug("thread count: %d", threadCount);
    }

private slots:
    void noThread_data();
    void noThread();

    void uncontendedNative();
    void uncontendedQMutex();
    void uncontendedQMutexLocker();

    void contendedNative_data();
    void contendedQMutex_data() { contendedNative_data(); }
    void contendedQMutexLocker_data() { contendedNative_data(); }

    void contendedNative();
    void contendedQMutex();
    void contendedQMutexLocker();
};

void tst_QMutex::noThread_data()
{
    QTest::addColumn<int>("t");

    QTest::newRow("noLock") << 1;
    QTest::newRow("QMutexInline") << 2;
    QTest::newRow("QMutex") << 3;
    QTest::newRow("QMutexLocker") << 4;
}

void tst_QMutex::noThread()
{
    volatile int count = 0;
    const int N = 5000000;
    QMutex mtx;

    QFETCH(int, t);
    switch(t) {
        case 1:
            QBENCHMARK {
                count = 0;
                for (int i = 0; i < N; i++) {
                    count++;
                }
            }
            break;
        case 2:
            QBENCHMARK {
                count = 0;
                for (int i = 0; i < N; i++) {
                    mtx.lockInline();
                    count++;
                    mtx.unlockInline();
                }
            }
            break;
        case 3:
            QBENCHMARK {
                count = 0;
                for (int i = 0; i < N; i++) {
                    mtx.lock();
                    count++;
                    mtx.unlock();
                }
            }
            break;
        case 4:
            QBENCHMARK {
                count = 0;
                for (int i = 0; i < N; i++) {
                    QMutexLocker locker(&mtx);
                    count++;
                }
            }
            break;
    }
    QCOMPARE(int(count), N);
}

void tst_QMutex::uncontendedNative()
{
    NativeMutexType mutex;
    NativeMutexInitialize(&mutex);
    QBENCHMARK {
        NativeMutexLock(&mutex);
        NativeMutexUnlock(&mutex);
    }
    NativeMutexDestroy(&mutex);
}

void tst_QMutex::uncontendedQMutex()
{
    QMutex mutex;
    QBENCHMARK {
        mutex.lock();
        mutex.unlock();
    }
}

void tst_QMutex::uncontendedQMutexLocker()
{
    QMutex mutex;
    QBENCHMARK {
        QMutexLocker locker(&mutex);
    }
}

void tst_QMutex::contendedNative_data()
{
    QTest::addColumn<int>("iterations");
    QTest::addColumn<int>("msleepDuration");
    QTest::newRow("-1")  << 100 <<  -1;
    QTest::newRow("0")   << 100 <<   0;
    QTest::newRow("1")   <<  10 <<   1;
    QTest::newRow("2")   <<  10 <<   2;
}

class NativeMutexThread : public QThread
{
    QSemaphore *semaphore1, *semaphore2;
    NativeMutexType *mutex;
    int iterations, msleepDuration;
public:
    bool done;
    NativeMutexThread(QSemaphore *semaphore1, QSemaphore *semaphore2, NativeMutexType *mutex, int iterations, int msleepDuration)
        : semaphore1(semaphore1), semaphore2(semaphore2), mutex(mutex), iterations(iterations), msleepDuration(msleepDuration), done(false)
    { }
    void run() {
        forever {
            semaphore1->acquire();
            if (done)
                break;
            for (int i = 0; i < iterations; ++i) {
                NativeMutexLock(mutex);
                if (msleepDuration >= 0)
                    msleep(msleepDuration);
                NativeMutexUnlock(mutex);
            }
            semaphore2->release();
        }
    }
};

void tst_QMutex::contendedNative()
{
    QFETCH(int, iterations);
    QFETCH(int, msleepDuration);

    QSemaphore semaphore1, semaphore2;
    NativeMutexType mutex;
    NativeMutexInitialize(&mutex);

    QVector<NativeMutexThread *> threads(threadCount);
    for (int i = 0; i < threads.count(); ++i) {
        threads[i] = new NativeMutexThread(&semaphore1, &semaphore2, &mutex, iterations, msleepDuration);
        threads[i]->start();
    }

    QBENCHMARK {
        semaphore1.release(threadCount);
        semaphore2.acquire(threadCount);
    }

    for (int i = 0; i < threads.count(); ++i)
        threads[i]->done = true;
    semaphore1.release(threadCount);
    for (int i = 0; i < threads.count(); ++i)
        threads[i]->wait();
    qDeleteAll(threads);

    NativeMutexDestroy(&mutex);
}

class QMutexThread : public QThread
{
    QSemaphore *semaphore1, *semaphore2;
    QMutex *mutex;
    int iterations, msleepDuration;
public:
    bool done;
    QMutexThread(QSemaphore *semaphore1, QSemaphore *semaphore2, QMutex *mutex, int iterations, int msleepDuration)
        : semaphore1(semaphore1), semaphore2(semaphore2), mutex(mutex), iterations(iterations), msleepDuration(msleepDuration), done(false)
    { }
    void run() {
        forever {
            semaphore1->acquire();
            if (done)
                break;
            for (int i = 0; i < iterations; ++i) {
                mutex->lock();
                if (msleepDuration >= 0)
                    msleep(msleepDuration);
                mutex->unlock();
            }
            semaphore2->release();
        }
    }
};

void tst_QMutex::contendedQMutex()
{
    QFETCH(int, iterations);
    QFETCH(int, msleepDuration);
    QSemaphore semaphore1, semaphore2;
    QMutex mutex;

    QVector<QMutexThread *> threads(threadCount);
    for (int i = 0; i < threads.count(); ++i) {
        threads[i] = new QMutexThread(&semaphore1, &semaphore2, &mutex, iterations, msleepDuration);
        threads[i]->start();
    }

    QBENCHMARK {
        semaphore1.release(threadCount);
        semaphore2.acquire(threadCount);
    }

    for (int i = 0; i < threads.count(); ++i)
        threads[i]->done = true;
    semaphore1.release(threadCount);
    for (int i = 0; i < threads.count(); ++i)
        threads[i]->wait();
    qDeleteAll(threads);
}

class QMutexLockerThread : public QThread
{
    QSemaphore *semaphore1, *semaphore2;
    QMutex *mutex;
    int iterations, msleepDuration;
public:
    bool done;
    QMutexLockerThread(QSemaphore *semaphore1, QSemaphore *semaphore2, QMutex *mutex, int iterations, int msleepDuration)
        : semaphore1(semaphore1), semaphore2(semaphore2), mutex(mutex), iterations(iterations), msleepDuration(msleepDuration), done(false)
    { }
    void run() {
        forever {
            semaphore1->acquire();
            if (done)
                break;
            for (int i = 0; i < iterations; ++i) {
                QMutexLocker locker(mutex);
                if (msleepDuration >= 0)
                    msleep(msleepDuration);
            }
            semaphore2->release();
        }
    }
};

void tst_QMutex::contendedQMutexLocker()
{
    QFETCH(int, iterations);
    QFETCH(int, msleepDuration);
    QSemaphore semaphore1, semaphore2;
    QMutex mutex;

    QVector<QMutexLockerThread *> threads(threadCount);
    for (int i = 0; i < threads.count(); ++i) {
        threads[i] = new QMutexLockerThread(&semaphore1, &semaphore2, &mutex, iterations, msleepDuration);
        threads[i]->start();
    }

    QBENCHMARK {
        semaphore1.release(threadCount);
        semaphore2.acquire(threadCount);
    }

    for (int i = 0; i < threads.count(); ++i)
        threads[i]->done = true;
    semaphore1.release(threadCount);
    for (int i = 0; i < threads.count(); ++i)
        threads[i]->wait();
    qDeleteAll(threads);
}

QTEST_MAIN(tst_QMutex)
#include "tst_qmutex.moc"
