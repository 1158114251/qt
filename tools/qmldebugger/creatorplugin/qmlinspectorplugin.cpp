#include <QtCore/QStringList>
#include <QtCore/QtPlugin>
#include <QtCore/QDebug>

#include <coreplugin/icore.h>

#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/project.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>

#include <extensionsystem/pluginmanager.h>

#include <QtDeclarative/qmldebug.h>
#include <QtDeclarative/qmldebugclient.h>

#include "runcontrol.h"
#include "qmlinspector.h"
#include "qmlinspectormode.h"
#include "inspectoroutputpane.h"
#include "qmlinspectorplugin.h"

QT_BEGIN_NAMESPACE


QmlInspectorPlugin::QmlInspectorPlugin()
    : m_inspectMode(0),
      m_runControl(0)
{
}

QmlInspectorPlugin::~QmlInspectorPlugin()
{
}

void QmlInspectorPlugin::shutdown()
{
    removeObject(m_inspectMode);
    delete m_inspectMode;
    m_inspectMode = 0;

    removeObject(m_outputPane);
    delete m_outputPane;
    m_outputPane = 0;
}

bool QmlInspectorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    Core::ICore *core = Core::ICore::instance();
    Core::UniqueIDManager *uidm = core->uniqueIDManager();

    QList<int> modeContext;
    modeContext.append(uidm->uniqueIdentifier(QmlInspector::Constants::C_INSPECTOR));

    QList<int> inspectorContext;
    inspectorContext.append(uidm->uniqueIdentifier(Core::Constants::C_EDITORMANAGER));
    inspectorContext.append(uidm->uniqueIdentifier(QmlInspector::Constants::C_INSPECTOR));
    inspectorContext.append(uidm->uniqueIdentifier(Core::Constants::C_NAVIGATION_PANE));

    m_inspectMode = new QmlInspectorMode(this);
    connect(m_inspectMode, SIGNAL(startViewer()), SLOT(startViewer()));
    connect(m_inspectMode, SIGNAL(stopViewer()), SLOT(stopViewer()));
    m_inspectMode->setContext(modeContext);
    addObject(m_inspectMode);

    m_outputPane = new InspectorOutputPane;
    addObject(m_outputPane);

    connect(m_inspectMode, SIGNAL(statusMessage(QString)),
            m_outputPane, SLOT(addInspectorStatus(QString)));

    m_runControlFactory = new QmlInspectorRunControlFactory(this);
    addAutoReleasedObject(m_runControlFactory);
    
    return true;
}

void QmlInspectorPlugin::extensionsInitialized()
{
}

void QmlInspectorPlugin::startViewer()
{
    stopViewer();
    
    ProjectExplorer::Project *project = 0;
    ProjectExplorer::ProjectExplorerPlugin *plugin = ProjectExplorer::ProjectExplorerPlugin::instance();
    if (plugin)
        project = plugin->currentProject();
    if (!project) {
        qDebug() << "No project loaded"; // TODO should this just run the debugger without a viewer?
        return;
    }
        
    QSharedPointer<ProjectExplorer::RunConfiguration> rc = project->activeRunConfiguration();

    QmlInspector::StartParameters sp;
    sp.port = m_inspectMode->viewerPort();

    m_runControl = m_runControlFactory->create(rc, ProjectExplorer::Constants::RUNMODE, sp);

    if (m_runControl) {
        connect(m_runControl, SIGNAL(started()), m_inspectMode, SLOT(connectToViewer()));
        connect(m_runControl, SIGNAL(finished()), m_inspectMode, SLOT(disconnectFromViewer()));

        connect(m_runControl, SIGNAL(addToOutputWindow(RunControl*,QString)),
                m_outputPane, SLOT(addOutput(RunControl*,QString)));
        connect(m_runControl, SIGNAL(addToOutputWindowInline(RunControl*,QString)),
                m_outputPane, SLOT(addOutputInline(RunControl*,QString)));
        connect(m_runControl, SIGNAL(error(RunControl*,QString)),
                m_outputPane, SLOT(addErrorOutput(RunControl*,QString)));
 
        m_runControl->start();
        m_outputPane->popup(false);
    }
    
}

void QmlInspectorPlugin::stopViewer()
{
    if (m_runControl) {
        m_runControl->stop();
        m_runControl->deleteLater();
        m_runControl = 0;
    }
}


Q_EXPORT_PLUGIN(QmlInspectorPlugin)

QT_END_NAMESPACE

