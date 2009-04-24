/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_EMBEDDED_LICENSE$
**
****************************************************************************/

#include "qeventdispatcher_symbian_p.h"
#include <private/qthread_p.h>
#include <qcoreapplication.h>
#include <private/qcoreapplication_p.h>
#include <qdatetime.h>

#include <unistd.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

#define WAKE_UP_PRIORITY CActive::EPriorityStandard
#define TIMER_PRIORITY CActive::EPriorityLow
#define COMPLETE_ZERO_TIMERS_PRIORITY CActive::EPriorityIdle

static inline int qt_pipe_write(int socket, const char *data, qint64 len)
{
	return ::write(socket, data, len);
}
#if defined(write)
# undef write
#endif

static inline int qt_pipe_close(int socket)
{
	return ::close(socket);
}
#if defined(close)
# undef close
#endif

static inline int qt_pipe_fcntl(int socket, int command)
{
	return ::fcntl(socket, command);
}
static inline int qt_pipe2_fcntl(int socket, int command, int option)
{
	return ::fcntl(socket, command, option);
}
#if defined(fcntl)
# undef fcntl
#endif

static inline int qt_socket_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

// This simply interrupts the select and locks the mutex until destroyed.
class QSelectMutexGrabber
{
public:
    QSelectMutexGrabber(int fd, QMutex *mutex)
        : m_mutex(mutex)
    {
        if (m_mutex->tryLock())
            return;

        char dummy = 0;
        qt_pipe_write(fd, &dummy, 1);

        m_mutex->lock();
    }

    ~QSelectMutexGrabber()
    {
        m_mutex->unlock();
    }

private:
    QMutex *m_mutex;
};

QWakeUpActiveObject::QWakeUpActiveObject(QEventDispatcherSymbian *dispatcher)
    : CActive(WAKE_UP_PRIORITY),
      m_dispatcher(dispatcher)
{
    CActiveScheduler::Add(this);
    iStatus = KRequestPending;
    SetActive();
}

QWakeUpActiveObject::~QWakeUpActiveObject()
{
    Cancel();
}

void QWakeUpActiveObject::DoCancel()
{
    if (iStatus.Int() & KRequestPending) {
        TRequestStatus *status = &iStatus;
        QEventDispatcherSymbian::RequestComplete(status, KErrNone);
    }
}

void QWakeUpActiveObject::RunL()
{
    iStatus = KRequestPending;
    SetActive();
    m_dispatcher->wakeUpWasCalled();
}

QTimerActiveObject::QTimerActiveObject(SymbianTimerInfo *timerInfo)
    : CActive(TIMER_PRIORITY),
      m_timerInfo(timerInfo)
{
}

QTimerActiveObject::~QTimerActiveObject()
{
    Cancel();
}

void QTimerActiveObject::DoCancel()
{
    if (m_timerInfo->interval > 0) {
        m_rTimer.Cancel();
        m_rTimer.Close();
    } else {
        if (iStatus.Int() & KRequestPending) {
            TRequestStatus *status = &iStatus;
            QEventDispatcherSymbian::RequestComplete(status, KErrNone);
        }
    }
}

void QTimerActiveObject::RunL()
{
    if (m_timerInfo->interval > 0) {
        // Start a new timer immediately so that we don't lose time.
        iStatus = KRequestPending;
        SetActive();
        m_rTimer.After(iStatus, m_timerInfo->interval*1000);

        m_timerInfo->dispatcher->timerFired(m_timerInfo->timerId);
    } else {
        // However, we only complete zero timers after the event has finished,
        // in order to prevent busy looping when doing nested loops.

        // Keep the refpointer around in order to avoid deletion until the end of this function.
        SymbianTimerInfoPtr timerInfoPtr(m_timerInfo);

        m_timerInfo->dispatcher->timerFired(m_timerInfo->timerId);

        iStatus = KRequestPending;
        SetActive();
        // We complete it after the processEvents is done.
    }
}

void QTimerActiveObject::Start()
{
    CActiveScheduler::Add(this);
    if (m_timerInfo->interval > 0) {
        m_rTimer.CreateLocal();
        iStatus = KRequestPending;
        SetActive();
        m_rTimer.After(iStatus, m_timerInfo->interval*1000);
    } else {
        iStatus = KRequestPending;
        SetActive();
        TRequestStatus *status = &iStatus;
        QEventDispatcherSymbian::RequestComplete(status, KErrNone);
    }
}

SymbianTimerInfo::~SymbianTimerInfo()
{
    delete timerAO;
}

QCompleteZeroTimersActiveObject::QCompleteZeroTimersActiveObject(QEventDispatcherSymbian *dispatcher)
    : CActive(COMPLETE_ZERO_TIMERS_PRIORITY),
      m_dispatcher(dispatcher)
{
    CActiveScheduler::Add(this);
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus *status = &iStatus;
    QEventDispatcherSymbian::RequestComplete(status, KErrNone);
}

QCompleteZeroTimersActiveObject::~QCompleteZeroTimersActiveObject()
{
    Cancel();
}

bool QCompleteZeroTimersActiveObject::ref()
{
    return (++m_refCount != 0);
}

bool QCompleteZeroTimersActiveObject::deref()
{
    return (--m_refCount != 0);
}

void QCompleteZeroTimersActiveObject::DoCancel()
{
    if (iStatus.Int() & KRequestPending) {
        TRequestStatus *status = &iStatus;
        QEventDispatcherSymbian::RequestComplete(status, KErrNone);
    }
}

void QCompleteZeroTimersActiveObject::RunL()
{
    m_dispatcher->completeZeroTimers();

    iStatus = KRequestPending;
    SetActive();
    TRequestStatus *status = &iStatus;
    QEventDispatcherSymbian::RequestComplete(status, KErrNone);
}

QSelectThread::QSelectThread()
    : m_quit(false)
{
    if (::pipe(m_pipeEnds) != 0) {
        qWarning("Select thread was unable to open a pipe, errno: %i", errno);
    } else {
        int flags0 = qt_pipe_fcntl(m_pipeEnds[0], F_GETFL);
        int flags1 = qt_pipe_fcntl(m_pipeEnds[1], F_GETFL);
        // We should check the error code here, but Open C has a bug that returns
        // failure even though the operation was successful.
        qt_pipe2_fcntl(m_pipeEnds[0], F_SETFL, flags0 | O_NONBLOCK);
        qt_pipe2_fcntl(m_pipeEnds[1], F_SETFL, flags1 | O_NONBLOCK);
    }
}

QSelectThread::~QSelectThread()
{
    qt_pipe_close(m_pipeEnds[1]);
    qt_pipe_close(m_pipeEnds[0]);
}

void QSelectThread::run()
{
    Q_D(QThread);

    m_mutex.lock();

    while (!m_quit) {
        fd_set readfds;
        fd_set writefds;
        fd_set exceptionfds;

        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptionfds);

        int maxfd = 0;
        maxfd = qMax(maxfd, updateSocketSet(QSocketNotifier::Read, &readfds));
        maxfd = qMax(maxfd, updateSocketSet(QSocketNotifier::Write, &writefds));
        maxfd = qMax(maxfd, updateSocketSet(QSocketNotifier::Exception, &exceptionfds));
        maxfd = qMax(maxfd, m_pipeEnds[0]);
        maxfd++;

        FD_SET(m_pipeEnds[0], &readfds);

        int ret;
        int savedSelectErrno;
        //do {
            ret = qt_socket_select(maxfd, &readfds, &writefds, &exceptionfds, 0);
            savedSelectErrno = errno;
        //} while (ret == 0);

        char buffer;

        while (::read(m_pipeEnds[0], &buffer, 1) > 0) {}

        if(ret == 0) {
         // do nothing
        } else if (ret < 0) {
            switch (savedSelectErrno) {
            case EBADF:
            case EINVAL:
            case ENOMEM:
            case EFAULT:
                qWarning("::select() returned an error: %i", savedSelectErrno);
                break;
            case ECONNREFUSED:
            case EPIPE:
                qWarning("::select() returned an error: %i (go through sockets)", savedSelectErrno);
                // prepare to go through all sockets
                // mark in fd sets both:
                //     good ones
                //     ones that return -1 in select
                // after loop update notifiers for all of them


                // clean @ start
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);
                FD_ZERO(&exceptionfds);
                {
                for (QHash<QSocketNotifier *, TRequestStatus *>::const_iterator i = m_AOStatuses.begin();
                        i != m_AOStatuses.end(); ++i) {

                    fd_set onefds;
                    FD_ZERO(&onefds);
                    FD_SET(i.key()->socket(), &onefds);
                    maxfd = i.key()->socket() + 1;

                    struct timeval timeout;
                    timeout.tv_sec = 0;
                    timeout.tv_usec = 0;

                    ret = 0;

                    if(i.key()->type() == QSocketNotifier::Read) {
                        ret = ::select(maxfd, &onefds, 0, 0, &timeout);
                        if(ret != 0) FD_SET(i.key()->socket(), &readfds);
                    } else if(i.key()->type() == QSocketNotifier::Write) {
                        ret = ::select(maxfd, 0, &onefds, 0, &timeout);
                        if(ret != 0) FD_SET(i.key()->socket(), &writefds);
                    } else { // must be exception fds then
                        ret = ::select(maxfd, 0, 0, &onefds, &timeout);
                        if(ret != 0) FD_SET(i.key()->socket(), &exceptionfds);
                    }

                } // end for
                }

                // traversed all, so update
                updateActivatedNotifiers(QSocketNotifier::Read, &readfds);
                updateActivatedNotifiers(QSocketNotifier::Write, &writefds);
                updateActivatedNotifiers(QSocketNotifier::Exception, &exceptionfds);

                break;
            case EINTR: // Should never occur on Symbian, but this is future proof!
            default:
                qWarning("::select() returned an unknown error: %i", savedSelectErrno);

                break;
            }
        } else {
            updateActivatedNotifiers(QSocketNotifier::Read, &readfds);
            updateActivatedNotifiers(QSocketNotifier::Write, &writefds);
            updateActivatedNotifiers(QSocketNotifier::Exception, &exceptionfds);
        }

        m_waitCond.wait(&m_mutex);
    }

    m_mutex.unlock();
}

void QSelectThread::requestSocketEvents ( QSocketNotifier *notifier, TRequestStatus *status )
{
    Q_D(QThread);

    if (!isRunning()) {
        start();
    }

    Q_ASSERT(!m_AOStatuses.contains(notifier));

    QSelectMutexGrabber lock(m_pipeEnds[1], &m_mutex);

    m_AOStatuses.insert(notifier, status);

    m_waitCond.wakeAll();
}

void QSelectThread::cancelSocketEvents ( QSocketNotifier *notifier )
{
    QSelectMutexGrabber lock(m_pipeEnds[1], &m_mutex);

    m_AOStatuses.remove(notifier);

    m_waitCond.wakeAll();
}

void QSelectThread::restart()
{
    QSelectMutexGrabber lock(m_pipeEnds[1], &m_mutex);

    m_waitCond.wakeAll();
}

int QSelectThread::updateSocketSet(QSocketNotifier::Type type, fd_set *fds)
{
    int maxfd = 0;
    for (QHash<QSocketNotifier *, TRequestStatus *>::const_iterator i = m_AOStatuses.begin();
            i != m_AOStatuses.end(); ++i) {
        if (i.key()->type() == type) {
            FD_SET(i.key()->socket(), fds);
            maxfd = qMax(maxfd, i.key()->socket());
        }
    }

    return maxfd;
}

void QSelectThread::updateActivatedNotifiers(QSocketNotifier::Type type, fd_set *fds)
{
    Q_D(QThread);

    QList<QSocketNotifier *> toRemove;
    for (QHash<QSocketNotifier *, TRequestStatus *>::const_iterator i = m_AOStatuses.begin();
            i != m_AOStatuses.end(); ++i) {
        if (i.key()->type() == type && FD_ISSET(i.key()->socket(), fds)) {
            toRemove.append(i.key());
            TRequestStatus *status = i.value();
            // Thread data is still owned by the main thread.
            QEventDispatcherSymbian::RequestComplete(d->threadData->symbian_thread_handle, status, KErrNone);
        }
    }

    for (int c = 0; c < toRemove.size(); ++c) {
        m_AOStatuses.remove(toRemove[c]);
    }
}

void QSelectThread::stop()
{
    m_quit = true;
    restart();
    wait();
}

QSocketActiveObject::QSocketActiveObject(QEventDispatcherSymbian *dispatcher, QSocketNotifier *notifier)
    : CActive(CActive::EPriorityStandard),
      m_dispatcher(dispatcher),
      m_notifier(notifier),
      m_inSocketEvent(false),
      m_deleteLater(false)
{
    CActiveScheduler::Add(this);
    iStatus = KRequestPending;
    SetActive();
}

QSocketActiveObject::~QSocketActiveObject()
{
    Cancel();
}

void QSocketActiveObject::DoCancel()
{
    if (iStatus.Int() & KRequestPending) {
        TRequestStatus *status = &iStatus;
        QEventDispatcherSymbian::RequestComplete(status, KErrNone);
    }
}

void QSocketActiveObject::RunL()
{
    m_dispatcher->socketFired(this);
}

void QSocketActiveObject::deleteLater()
{
    if (m_inSocketEvent) {
        m_deleteLater = true;
    } else {
        delete this;
    }
}

QEventDispatcherSymbian::QEventDispatcherSymbian(QObject *parent)
    : QAbstractEventDispatcher(parent),
      m_activeScheduler(0),
      m_wakeUpAO(0),
      m_completeZeroTimersAO(0),
      m_interrupt(false),
      m_wakeUpDone(0),
      m_noSocketEvents(false)
{
}

QEventDispatcherSymbian::~QEventDispatcherSymbian()
{
    m_processHandle.Close();
}

void QEventDispatcherSymbian::startingUp()
{
    if( !CActiveScheduler::Current() ) {
        m_activeScheduler = new(ELeave)CActiveScheduler();
        CActiveScheduler::Install(m_activeScheduler);
    }
    m_wakeUpAO = new(ELeave) QWakeUpActiveObject(this);
    // We already might have posted events, wakeup once to process them
    wakeUp();
}

void QEventDispatcherSymbian::closingDown()
{
    if (m_selectThread.isRunning()) {
        m_selectThread.stop();
    }
/*
    We do have bug in AO cancel mechanism, unfortunately it was too late
    to fix it for Garden release. This has to be fixed after Garden, see task: 246600
    delete m_wakeUpAO;
    if (m_activeScheduler) {
        CActiveScheduler::Install(NULL);
        delete m_activeScheduler;
    }
    */
}

bool QEventDispatcherSymbian::processEvents ( QEventLoop::ProcessEventsFlags flags )
{
    Q_D(QAbstractEventDispatcher);

    RThread &thread = d->threadData->symbian_thread_handle;

    bool block;
    if (flags & QEventLoop::WaitForMoreEvents) {
        block = true;
        emit aboutToBlock();
    } else {
        block = false;
    }

    bool handledAnyEvent = false;

    bool oldNoSocketEventsValue = m_noSocketEvents;
    if (flags & QEventLoop::ExcludeSocketNotifiers) {
        m_noSocketEvents = true;
    } else {
        m_noSocketEvents = false;
        handledAnyEvent = sendDeferredSocketEvents();
    }

    bool handledSymbianEvent = false;
    m_interrupt = false;

    /*
     * This QTime variable is used to measure the time it takes to finish
     * the event loop. If we take too long in the loop, other processes
     * may be starved and killed. After the first event has completed, we
     * take the current time, and if the remaining events take longer than
     * a preset time, we temporarily lower the priority to force a context
     * switch. For applications that do not take unecessarily long in the
     * event loop, the priority will not be altered.
     */
    QTime time;
    enum {
        FirstRun,
        SubsequentRun,
        TimeStarted
    } timeState = FirstRun;

    TProcessPriority priority;

    while (1) {
        if (block) {
            // This is where Qt will spend most of its time.
            CActiveScheduler::Current()->WaitForAnyRequest();
        } else {
            if (thread.RequestCount() == 0) {
                break;
            }
            // This one should return without delay.
            CActiveScheduler::Current()->WaitForAnyRequest();
        }

        if (timeState == SubsequentRun) {
            time.start();
            timeState = TimeStarted;
        }

        TInt error;
        handledSymbianEvent = CActiveScheduler::RunIfReady(error, CActive::EPriorityIdle);
        if (error) {
            qWarning("CActiveScheduler::RunIfReady() returned error: %i\n", error);
            CActiveScheduler::Current()->Error(error);
        }
        if (!handledSymbianEvent) {
            qFatal("QEventDispatcherSymbian::processEvents(): Caught Symbian stray signal");
        }
        handledAnyEvent = true;
        if (m_interrupt) {
            break;
        }
        block = false;
        if (timeState == TimeStarted && time.elapsed() > 500) {
            priority = m_processHandle.Priority();
            m_processHandle.SetPriority(EPriorityLow);
            time.start();
            // Slight chance of race condition in the next lines, but nothing fatal
            // will happen, just wrong priority.
            if (m_processHandle.Priority() == EPriorityLow) {
                m_processHandle.SetPriority(priority);
            }
        }
        if (timeState == FirstRun)
            timeState = SubsequentRun;
    };

    // Complete zero timers so that we get them next time.
    completeZeroTimers();

    emit awake();

    m_noSocketEvents = oldNoSocketEventsValue;

    return handledAnyEvent;
}

void QEventDispatcherSymbian::timerFired(int timerId)
{
    QHash<int, SymbianTimerInfoPtr>::iterator i = m_timerList.find(timerId);
    if (i == m_timerList.end()) {
        // The timer has been deleted. Ignore this event.
        return;
    }

    SymbianTimerInfoPtr timerInfo = *i;

    // Prevent infinite timer recursion.
    if (timerInfo->inTimerEvent) {
        return;
    }

    timerInfo->inTimerEvent = true;

    QTimerEvent event(timerInfo->timerId);
    QCoreApplication::sendEvent(timerInfo->receiver, &event);

    timerInfo->inTimerEvent = false;

    return;
}

void QEventDispatcherSymbian::socketFired(QSocketActiveObject *socketAO)
{
    if (m_noSocketEvents) {
        m_deferredSocketEvents.append(socketAO);
        return;
    }

    QEvent e(QEvent::SockAct);
    socketAO->m_inSocketEvent = true;
    QCoreApplication::sendEvent(socketAO->m_notifier, &e);
    socketAO->m_inSocketEvent = false;

    if (socketAO->m_deleteLater) {
        delete socketAO;
    } else {
        socketAO->iStatus = KRequestPending;
        socketAO->SetActive();
        reactivateSocketNotifier(socketAO->m_notifier);
    }
}

void QEventDispatcherSymbian::wakeUpWasCalled()
{
    // The reactivation should happen in RunL, right before the call to this function.
    // This is because m_wakeUpDone is the "signal" that the object can be completed
    // once more.
    // Also, by dispatching the posted events after resetting m_wakeUpDone, we guarantee
    // that no posted event notification will be lost. If we did it the other way
    // around, it would be possible for another thread to post an event right after
    // the sendPostedEvents was done, but before the object was ready to be completed
    // again. This could deadlock the application if there are no other posted events.
    m_wakeUpDone.fetchAndStoreOrdered(0);
    sendPostedEvents();
}

void QEventDispatcherSymbian::interrupt()
{
    m_interrupt = true;
    wakeUp();
}

void QEventDispatcherSymbian::wakeUp()
{
    Q_D(QAbstractEventDispatcher);

    if (m_wakeUpAO && m_wakeUpDone.testAndSetAcquire(0, 1)) {
        TRequestStatus *status = &m_wakeUpAO->iStatus;
        QEventDispatcherSymbian::RequestComplete(d->threadData->symbian_thread_handle, status, KErrNone);
    }
}

bool QEventDispatcherSymbian::sendPostedEvents()
{
    Q_D(QAbstractEventDispatcher);

    // moveToThread calls this and canWait == true -> Events will never get processed
    // if we check for d->threadData->canWait
    //
    // QCoreApplication::postEvent sets canWait = false, but after the object and events
    // are moved to a new thread, the canWait in new thread is true i.e. not changed to reflect
    // the flag on old thread. That's why events in a new thread will not get processed.
    // This migth be actually bug in moveToThread functionality, but because other platforms
    // do not check canWait in wakeUp (where we essentially are now) - decided to remove it from
    // here as well.

    //if (!d->threadData->canWait) {
        QCoreApplicationPrivate::sendPostedEvents(0, 0, d->threadData);
        return true;
    //}
    //return false;
}

void QEventDispatcherSymbian::completeZeroTimers()
{
    for (QHash<int, SymbianTimerInfoPtr>::iterator i = m_timerList.begin(); i != m_timerList.end(); ++i) {
        if ((*i)->interval == 0 && (*i)->timerAO->iStatus.Int() & KRequestPending) {
            TRequestStatus *status = &(*i)->timerAO->iStatus;
            QEventDispatcherSymbian::RequestComplete(status, KErrNone);
        }
    }

    // We do this because we want to return from processEvents. This is because
    // each invocation of processEvents should only run each zero timer once.
    // The active scheduler should run them continously, however.
    m_interrupt = true;
}

bool QEventDispatcherSymbian::sendDeferredSocketEvents()
{
    bool sentAnyEvents = false;
    while (!m_deferredSocketEvents.isEmpty()) {
        sentAnyEvents = true;
        socketFired(m_deferredSocketEvents.takeFirst());
    }

    return sentAnyEvents;
}

void QEventDispatcherSymbian::flush()
{
}

bool QEventDispatcherSymbian::hasPendingEvents()
{
    Q_D(QAbstractEventDispatcher);
    return (d->threadData->symbian_thread_handle.RequestCount() != 0
            || !d->threadData->canWait || !m_deferredSocketEvents.isEmpty());
}

void QEventDispatcherSymbian::registerSocketNotifier ( QSocketNotifier * notifier )
{
    QSocketActiveObject *socketAO = new (ELeave) QSocketActiveObject(this, notifier);
    m_notifiers.insert(notifier, socketAO);
    m_selectThread.requestSocketEvents(notifier, &socketAO->iStatus);
}

void QEventDispatcherSymbian::unregisterSocketNotifier ( QSocketNotifier * notifier )
{
    m_selectThread.cancelSocketEvents(notifier);
    if (m_notifiers.contains(notifier)) {
        QSocketActiveObject *sockObj = *m_notifiers.find(notifier);
        m_deferredSocketEvents.removeAll(sockObj);
        sockObj->deleteLater();
        m_notifiers.remove(notifier);
    }
}

void QEventDispatcherSymbian::reactivateSocketNotifier(QSocketNotifier *notifier)
{
    m_selectThread.requestSocketEvents(notifier, &m_notifiers[notifier]->iStatus);
}

void QEventDispatcherSymbian::registerTimer ( int timerId, int interval, QObject * object )
{
    if (interval < 0) {
        qWarning("Timer interval < 0");
        interval = 0;
    }

    SymbianTimerInfoPtr timer(new SymbianTimerInfo);
    timer->timerId      = timerId;
    timer->interval     = interval;
    timer->inTimerEvent = false;
    timer->receiver     = object;
    timer->dispatcher   = this;
    timer->timerAO      = new(ELeave) QTimerActiveObject(timer.data());
    m_timerList.insert(timerId, timer);

    if (interval == 0) {
        if (!m_completeZeroTimersAO) {
            m_completeZeroTimersAO = new (ELeave) QCompleteZeroTimersActiveObject(this);
        }
        m_completeZeroTimersAO->ref();
    }

    timer->timerAO->Start();
}

bool QEventDispatcherSymbian::unregisterTimer ( int timerId )
{
    if (!m_timerList.contains(timerId)) {
        return false;
    }

    SymbianTimerInfoPtr timerInfo = m_timerList.take(timerId);
    if (timerInfo->interval == 0) {
        if (!m_completeZeroTimersAO->deref()) {
            delete m_completeZeroTimersAO;
            m_completeZeroTimersAO = 0;
        }
    }

    return true;
}

bool QEventDispatcherSymbian::unregisterTimers ( QObject * object )
{
    QList<TimerInfo> idsToRemove = registeredTimers(object);

    if (idsToRemove.isEmpty())
        return false;

    for (int c = 0; c < idsToRemove.size(); ++c) {
        unregisterTimer(idsToRemove[c].first);
    }

    return true;
}

QList<QEventDispatcherSymbian::TimerInfo> QEventDispatcherSymbian::registeredTimers ( QObject * object ) const
{
    QList<TimerInfo> list;
    for (QHash<int, SymbianTimerInfoPtr>::const_iterator i = m_timerList.begin(); i != m_timerList.end(); ++i) {
        if ((*i)->receiver == object) {
            list.push_back(TimerInfo((*i)->timerId, (*i)->interval));
        }
    }

    return list;
}

QT_END_NAMESPACE

