/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt QML Debugger of the Qt Toolkit.
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
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>

#include <projectexplorer/applicationlauncher.h>
#include <projectexplorer/applicationrunconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>

#include "runcontrol.h"

using namespace ProjectExplorer;


QmlInspectorRunControlFactory::QmlInspectorRunControlFactory(QObject *parent)
    : ProjectExplorer::IRunControlFactory(parent)
{
}

bool QmlInspectorRunControlFactory::canRun(const QSharedPointer<RunConfiguration> &runConfiguration, const QString &mode) const
{
    Q_UNUSED(runConfiguration);
    if (mode != ProjectExplorer::Constants::RUNMODE)
        return false;
    return true;
}

ProjectExplorer::RunControl *QmlInspectorRunControlFactory::create(const QSharedPointer<RunConfiguration> &runConfiguration, const QString &mode)
{
    Q_UNUSED(mode);
    return new QmlInspectorRunControl(runConfiguration);
}

ProjectExplorer::RunControl *QmlInspectorRunControlFactory::create(const QSharedPointer<ProjectExplorer::RunConfiguration> &runConfiguration,
const QString &mode, const QmlInspector::StartParameters &sp)
{
    Q_UNUSED(mode);
    return new QmlInspectorRunControl(runConfiguration, sp);
}
                
QString QmlInspectorRunControlFactory::displayName() const
{
    return tr("Qml Inspector");
}

QWidget *QmlInspectorRunControlFactory::configurationWidget(const QSharedPointer<RunConfiguration> &runConfiguration)
{
    Q_UNUSED(runConfiguration);
    return 0;
}



QmlInspectorRunControl::QmlInspectorRunControl(const QSharedPointer<ProjectExplorer::RunConfiguration> &runConfiguration,
const QmlInspector::StartParameters &sp)
    : ProjectExplorer::RunControl(runConfiguration),
      m_running(false),
      m_viewerLauncher(0),
      m_startParams(sp)
{
}

QmlInspectorRunControl::~QmlInspectorRunControl()
{
}

void QmlInspectorRunControl::start()
{
    if (m_running || m_viewerLauncher)
        return;

    m_viewerLauncher = new ProjectExplorer::ApplicationLauncher(this);
    connect(m_viewerLauncher, SIGNAL(applicationError(QString)), SLOT(applicationError(QString)));
    connect(m_viewerLauncher, SIGNAL(processExited(int)), SLOT(viewerExited()));
    connect(m_viewerLauncher, SIGNAL(appendOutput(QString)), SLOT(appendOutput(QString)));
    connect(m_viewerLauncher, SIGNAL(bringToForegroundRequested(qint64)),
            this, SLOT(appStarted()));

    QSharedPointer<LocalApplicationRunConfiguration> rc =
        runConfiguration().objectCast<LocalApplicationRunConfiguration>();
    if (rc.isNull()) {  // TODO
        return;
    }

    ProjectExplorer::Environment env = rc->environment();
    env.set("QML_DEBUG_SERVER_PORT", QString::number(m_startParams.port));

    QStringList arguments = rc->commandLineArguments();
    arguments << QLatin1String("-stayontop");

    m_viewerLauncher->setEnvironment(env.toStringList());
    m_viewerLauncher->setWorkingDirectory(rc->workingDirectory());

    m_running = true;

    m_viewerLauncher->start(static_cast<ApplicationLauncher::Mode>(rc->runMode()),
                            rc->executable(), arguments);
}

void QmlInspectorRunControl::stop()
{
    if (m_viewerLauncher->isRunning())
        m_viewerLauncher->stop();
}

bool QmlInspectorRunControl::isRunning() const
{
    return m_running;
}

void QmlInspectorRunControl::appStarted()
{
    QTimer::singleShot(500, this, SLOT(delayedStart()));
}

void QmlInspectorRunControl::appendOutput(const QString &s)
{
    emit addToOutputWindow(this, s);
}

void QmlInspectorRunControl::delayedStart()
{
    emit started();
}

void QmlInspectorRunControl::viewerExited()
{
    m_running = false;
    emit finished();
    
    deleteLater();
}

void QmlInspectorRunControl::applicationError(const QString &s)
{
    emit error(this, s);
}
