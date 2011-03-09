/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qthread.h"

#include "qplatformdefs.h"

#include <private/qcoreapplication_p.h>
#if !defined(QT_NO_GLIB)
#  include "../kernel/qeventdispatcher_glib_p.h"
#endif

#include <private/qeventdispatcher_symbian_p.h>

#include "qthreadstorage.h"

#include "qthread_p.h"

#include "qdebug.h"

#include <sched.h>
#include <errno.h>

#ifdef Q_OS_BSD4
#include <sys/sysctl.h>
#endif
#ifdef Q_OS_VXWORKS
#  if (_WRS_VXWORKS_MAJOR > 6) || ((_WRS_VXWORKS_MAJOR == 6) && (_WRS_VXWORKS_MINOR >= 6))
#    include <vxCpuLib.h>
#    include <cpuset.h>
#    define QT_VXWORKS_HAS_CPUSET
#  endif
#endif

#ifdef Q_OS_HPUX
#include <sys/pstat.h>
#endif

#if defined(Q_OS_MAC)
# ifdef qDebug
#   define old_qDebug qDebug
#   undef qDebug
# endif
#ifndef QT_NO_CORESERVICES
# include <CoreServices/CoreServices.h>
#endif //QT_NO_CORESERVICES

# ifdef old_qDebug
#   undef qDebug
#   define qDebug QT_NO_QDEBUG_MACRO
#   undef old_qDebug
# endif
#endif

#if defined(Q_OS_LINUX) && !defined(SCHED_IDLE)
// from linux/sched.h
# define SCHED_IDLE    5
#endif

#if defined(Q_OS_DARWIN) || !defined(Q_OS_OPENBSD) && defined(_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING-0 >= 0)
#define QT_HAS_THREAD_PRIORITY_SCHEDULING
#endif


QT_BEGIN_NAMESPACE

#ifndef QT_NO_THREAD

enum { ThreadPriorityResetFlag = 0x80000000 };

static pthread_once_t current_thread_data_once = PTHREAD_ONCE_INIT;
static pthread_key_t current_thread_data_key;

static void destroy_current_thread_data(void *p)
{
    // POSIX says the value in our key is set to zero before calling
    // this destructor function, so we need to set it back to the
    // right value...
    pthread_setspecific(current_thread_data_key, p);
    reinterpret_cast<QThreadData *>(p)->deref();
    // ... but we must reset it to zero before returning so we aren't
    // called again (POSIX allows implementations to call destructor
    // functions repeatedly until all values are zero)
    pthread_setspecific(current_thread_data_key, 0);
}

static void create_current_thread_data_key()
{
    pthread_key_create(&current_thread_data_key, destroy_current_thread_data);
}

static void destroy_current_thread_data_key()
{
    pthread_once(&current_thread_data_once, create_current_thread_data_key);
    pthread_key_delete(current_thread_data_key);
}
Q_DESTRUCTOR_FUNCTION(destroy_current_thread_data_key)


// Utility functions for getting, setting and clearing thread specific data.
// In Symbian, TLS access is significantly faster than pthread_getspecific.
// However Symbian does not have the thread destruction cleanup functionality
// that pthread has, so pthread_setspecific is also used.
static QThreadData *get_thread_data()
{
    return reinterpret_cast<QThreadData *>(Dll::Tls());
}

static void set_thread_data(QThreadData *data)
{
    qt_symbian_throwIfError(Dll::SetTls(data));
//    pthread_once(&current_thread_data_once, create_current_thread_data_key);
//    pthread_setspecific(current_thread_data_key, data);
}

static void clear_thread_data()
{
    Dll::FreeTls();
//    pthread_setspecific(current_thread_data_key, 0);
}


static void init_symbian_thread_handle(RThread &thread)
{
    thread = RThread();
    TThreadId threadId = thread.Id();
    thread.Open(threadId);

    // Make thread handle accessible process wide
    RThread originalCloser = thread;
    thread.Duplicate(thread, EOwnerProcess);
    originalCloser.Close();
}

QThreadData *QThreadData::current()
{
    QThreadData *data = get_thread_data();
    if (!data) {
        void *a;
        if (QInternal::activateCallbacks(QInternal::AdoptCurrentThread, &a)) {
            QThread *adopted = static_cast<QThread*>(a);
            Q_ASSERT(adopted);
            data = QThreadData::get2(adopted);
            set_thread_data(data);
            adopted->d_func()->running = true;
            adopted->d_func()->finished = false;
            static_cast<QAdoptedThread *>(adopted)->init();
        } else {
            data = new QThreadData;
            QT_TRY {
                set_thread_data(data);
                data->thread = new QAdoptedThread(data);
            } QT_CATCH(...) {
                clear_thread_data();
                data->deref();
                data = 0;
                QT_RETHROW;
            }
            data->deref();
        }
        if (!QCoreApplicationPrivate::theMainThread)
            QCoreApplicationPrivate::theMainThread = data->thread;
    }
    return data;
}


void QAdoptedThread::init()
{
    Q_D(QThread);
    d->thread_id = RThread().Id();  // type operator to TUint
    init_symbian_thread_handle(d->data->symbian_thread_handle);
}

/*
   QThreadPrivate
*/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

typedef void*(*QtThreadCallback)(void*);

#if defined(Q_C_CALLBACKS)
}
#endif

#endif // QT_NO_THREAD

void QThreadPrivate::createEventDispatcher(QThreadData *data)
{
    data->eventDispatcher = new QEventDispatcherSymbian;
    data->eventDispatcher->startingUp();
}

#ifndef QT_NO_THREAD

void *QThreadPrivate::start(void *arg)
{
    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadData *data = QThreadData::get2(thr);

    // do we need to reset the thread priority?
    if (int(thr->d_func()->priority) & ThreadPriorityResetFlag) {
        thr->setPriority(QThread::Priority(thr->d_func()->priority & ~ThreadPriorityResetFlag));
    }

    // Because Symbian Open C does not provide a way to convert between
    // RThread and pthread_t, we must delay initialization of the RThread
    // handle when creating a thread, until we are running in the new thread.
    // Here, we pick up the current thread and assign that to the handle.
    init_symbian_thread_handle(data->symbian_thread_handle);

    // On symbian, threads other than the main thread are non critical by default
    // This means a worker thread can crash without crashing the application - to
    // use this feature, we would need to use RThread::Logon in the main thread
    // to catch abnormal thread exit and emit the finished signal.
    // For the sake of cross platform consistency, we set the thread as process critical
    // - advanced users who want the symbian behaviour can change the critical
    // attribute of the thread again once the app gains control in run()
    User::SetCritical(User::EProcessCritical);

    set_thread_data(data);

    data->ref();
    data->quitNow = false;

    // ### TODO: allow the user to create a custom event dispatcher
    createEventDispatcher(data);

    emit thr->started();
    thr->run();

    QThreadPrivate::finish(arg);

    return 0;
}

void QThreadPrivate::finish(void *arg, bool lockAnyway, bool closeNativeHandle)
{
    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadPrivate *d = thr->d_func();

    QMutexLocker locker(lockAnyway ? &d->mutex : 0);

    d->isInFinish = true;
    d->priority = QThread::InheritPriority;
    bool terminated = d->terminated;
    void *data = &d->data->tls;
    locker.unlock();
    if (terminated)
        emit thr->terminated();
    emit thr->finished();
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QThreadStorageData::finish((void **)data);
    locker.relock();
    d->terminated = false;

    QAbstractEventDispatcher *eventDispatcher = d->data->eventDispatcher;
    if (eventDispatcher) {
        d->data->eventDispatcher = 0;
        locker.unlock();
        eventDispatcher->closingDown();
        delete eventDispatcher;
        locker.relock();
    }

    d->thread_id = 0;
    if (closeNativeHandle)
        d->data->symbian_thread_handle.Close();
    d->running = false;
    d->finished = true;

    d->isInFinish = false;
    d->thread_done.wakeAll();
}




/**************************************************************************
 ** QThread
 *************************************************************************/

Qt::HANDLE QThread::currentThreadId()
{
    return (Qt::HANDLE) (TUint) RThread().Id();
}

#if defined(QT_LINUXBASE) && !defined(_SC_NPROCESSORS_ONLN)
// LSB doesn't define _SC_NPROCESSORS_ONLN.
#  define _SC_NPROCESSORS_ONLN 84
#endif

int QThread::idealThreadCount()
{
    int cores = -1;

     // ### TODO - Get the number of cores from HAL? when multicore architectures (SMP) are supported
    cores = 1;

    return cores;
}

void QThread::yieldCurrentThread()
{
    sched_yield();
}

/*  \internal
    helper function to do thread sleeps
*/
static void thread_sleep(unsigned long remaining, unsigned long scale)
{
    // maximum Symbian wait is 2^31 microseconds
    unsigned long maxWait = KMaxTInt / scale;
    do {
        unsigned long waitTime = qMin(maxWait, remaining);
        remaining -= waitTime;
        User::AfterHighRes(waitTime * scale);
    } while (remaining);
}

void QThread::sleep(unsigned long secs)
{
    thread_sleep(secs, 1000000ul);
}

void QThread::msleep(unsigned long msecs)
{
    thread_sleep(msecs, 1000ul);
}

void QThread::usleep(unsigned long usecs)
{
    thread_sleep(usecs, 1ul);
}

TThreadPriority calculateSymbianPriority(QThread::Priority priority)
    {
    // Both Qt & Symbian use limited enums; this matches the mapping previously done through conversion to Posix granularity
    TThreadPriority symPriority;
    switch (priority)
    {
        case QThread::IdlePriority:
            symPriority = EPriorityMuchLess;
            break;
        case QThread::LowestPriority:
        case QThread::LowPriority:
            symPriority = EPriorityLess;
            break;
        case QThread::NormalPriority:
            symPriority = EPriorityNormal;
            break;
        case QThread::HighPriority:
            symPriority = EPriorityMore;
            break;
        case QThread::HighestPriority:
        case QThread::TimeCriticalPriority:
        default:
            symPriority = EPriorityMuchMore;
            break;
    }
    return symPriority;
    }

void QThread::start(Priority priority)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (d->isInFinish)
        d->thread_done.wait(locker.mutex());

    if (d->running)
        return;

    d->running = true;
    d->finished = false;
    d->terminated = false;
    d->returnCode = 0;
    d->exited = false;

    d->priority = priority;

    if (d->stackSize == 0)
        // The default stack size on Symbian is very small, making even basic
        // operations like file I/O fail, so we increase it by default.
        d->stackSize = 0x14000; // Maximum stack size on Symbian.

    int code = 0;
    if (d->data->symbian_thread_handle.Create(KNullDesC, (TThreadFunction) QThreadPrivate::start, d->stackSize, NULL, this) == KErrNone)
        {
        d->thread_id = d->data->symbian_thread_handle.Id();
        TThreadPriority symPriority = calculateSymbianPriority(priority);
        d->data->symbian_thread_handle.SetPriority(symPriority);
        d->data->symbian_thread_handle.Resume();
        }
    else
        code = ENOMEM;  // probably the problem

    if (code) {
        qWarning("QThread::start: Thread creation error: %s", qPrintable(qt_error_string(code)));

        d->running = false;
        d->finished = false;
        d->thread_id = 0;
        d->data->symbian_thread_handle.Close();
    }
}

void QThread::terminate()
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (!d->thread_id)
        return;

    if (!d->running)
        return;
    if (!d->terminationEnabled) {
        d->terminatePending = true;
        return;
    }

    d->terminated = true;
    // "false, false" meaning:
    // 1. lockAnyway = false. Don't lock the mutex because it's already locked
    //    (see above).
    // 2. closeNativeSymbianHandle = false. We don't want to close the thread handle,
    //    because we need it here to terminate the thread.
    QThreadPrivate::finish(this, false, false);
    d->data->symbian_thread_handle.Terminate(KErrNone);
    d->data->symbian_thread_handle.Close();
}

bool QThread::wait(unsigned long time)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (d->thread_id == (TUint) RThread().Id()) {
        qWarning("QThread::wait: Thread tried to wait on itself");
        return false;
    }

    if (d->finished || !d->running)
        return true;

    while (d->running) {
        // Check if thread still exists. Needed because kernel will kill it without notification
        // before global statics are deleted at application exit.
        if (d->data->symbian_thread_handle.Handle()
            && d->data->symbian_thread_handle.ExitType() != EExitPending) {
            // Cannot call finish here as wait is typically called from another thread.
            // It won't be necessary anyway, as we should never get here under normal operations;
            // all QThreads are EProcessCritical and therefore cannot normally exit
            // undetected (i.e. panic) as long as all thread control is via QThread.
            return true;
        }
        if (!d->thread_done.wait(locker.mutex(), time))
            return false;
    }
    return true;
}

void QThread::setTerminationEnabled(bool enabled)
{
    QThread *thr = currentThread();
    Q_ASSERT_X(thr != 0, "QThread::setTerminationEnabled()",
               "Current thread was not started with QThread.");
    QThreadPrivate *d = thr->d_func();
    QMutexLocker locker(&d->mutex);
    d->terminationEnabled = enabled;
    if (enabled && d->terminatePending) {
        d->terminated = true;
        // "false" meaning:
        // -  lockAnyway = false. Don't lock the mutex because it's already locked
        //    (see above).
        QThreadPrivate::finish(thr, false);
        locker.unlock(); // don't leave the mutex locked!
        User::Exit(0);  // may be some other cleanup required? what if AS or cleanup stack?
    }
}

void QThread::setPriority(Priority priority)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);
    if (!d->running) {
        qWarning("QThread::setPriority: Cannot set priority, thread is not running");
        return;
    }

    d->priority = priority;

    // copied from start() with a few modifications:
    TThreadPriority symPriority = calculateSymbianPriority(priority);
    d->data->symbian_thread_handle.SetPriority(symPriority);
}

#endif // QT_NO_THREAD

QT_END_NAMESPACE

