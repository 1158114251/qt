#include <QtCore/QStringList>
#include <QtCore/QtPlugin>
#include <QtCore/QDebug>

#include <QtGui/qtoolbutton.h>
#include <QtGui/qtoolbar.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qlabel.h>
#include <QtGui/qdockwidget.h>
#include <QtGui/qaction.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qlabel.h>
#include <QtGui/qspinbox.h>

#include <coreplugin/basemode.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/project.h>

#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>

#include <private/qmldebug_p.h>
#include <private/qmldebugclient_p.h>

#include "../standalone/objectpropertiesview.h"
#include "../standalone/objecttree.h"
#include "../standalone/watchtable.h"
#include "../standalone/canvasframerate.h"
#include "../standalone/expressionquerywidget.h"

#include "qmlinspector.h"
#include "qmlinspectormode.h"

QT_BEGIN_NAMESPACE


class EngineSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    struct EngineInfo
    {
        QString name;
        int id;
    };

    EngineSpinBox(QWidget *parent = 0);

    void addEngine(int engine, const QString &name);
    void clearEngines();

protected:
    virtual QString textFromValue(int value) const;
    virtual int valueFromText(const QString &text) const;

private:
    QList<EngineInfo> m_engines;
};

EngineSpinBox::EngineSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    setEnabled(false);
    setReadOnly(true);
    setRange(0, 0);
}

void EngineSpinBox::addEngine(int engine, const QString &name)
{
    EngineInfo info;
    info.id = engine;
    if (name.isEmpty())
        info.name = tr("Engine %1", "engine number").arg(engine);
    else
        info.name = name;
    m_engines << info;

    setRange(0, m_engines.count()-1);
}

void EngineSpinBox::clearEngines()
{
    m_engines.clear();
}

QString EngineSpinBox::textFromValue(int value) const
{
    for (int i=0; i<m_engines.count(); i++) {
        if (m_engines[i].id == value)
            return m_engines[i].name;
    }
    return QLatin1String("<None>");
}

int EngineSpinBox::valueFromText(const QString &text) const
{
    for (int i=0; i<m_engines.count(); i++) {
        if (m_engines[i].name == text)
            return m_engines[i].id;
    }
    return -1;
}


QmlInspectorMode::QmlInspectorMode(QObject *parent)
  : Core::BaseMode(parent),
    m_conn(0),
    m_client(0),
    m_engineQuery(0),
    m_contextQuery(0)
{    
    m_watchTableModel = new WatchTableModel(0, this);

    initActions();
    setWidget(createModeWindow());

    setName(tr("QML Inspect"));
    setIcon(QIcon(":/qmlinspector/images/logo.png"));
    setUniqueModeName("QML_INSPECT_MODE");    
}

quint16 QmlInspectorMode::viewerPort() const
{
    return m_portSpinBox->value();
}

void QmlInspectorMode::connectToViewer()
{
    if (m_conn && m_conn->state() != QAbstractSocket::UnconnectedState)
        return;

    delete m_client; m_client = 0;

    if (m_conn) {
        m_conn->disconnectFromHost();
        delete m_conn;
    }

    m_conn = new QmlDebugConnection(this);
    connect(m_conn, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            SLOT(connectionStateChanged()));
    connect(m_conn, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(connectionError()));
    m_conn->connectToHost(m_addressEdit->text(), m_portSpinBox->value());
}

void QmlInspectorMode::disconnectFromViewer()
{
    m_conn->disconnectFromHost();
}

void QmlInspectorMode::connectionStateChanged()
{
    switch (m_conn->state()) {
        default:
        case QAbstractSocket::UnconnectedState:
        {
            emit statusMessage(tr("[Inspector] disconnected.\n\n"));
            m_addressEdit->setEnabled(true);
            m_portSpinBox->setEnabled(true);

            delete m_engineQuery;
            m_engineQuery = 0;
            delete m_contextQuery;
            m_contextQuery = 0;
            break;
        }
        case QAbstractSocket::HostLookupState:
            emit statusMessage(tr("[Inspector] resolving host..."));
            break;
        case QAbstractSocket::ConnectingState:
            emit statusMessage(tr("[Inspector] connecting to debug server..."));
            break;
        case QAbstractSocket::ConnectedState:
        {
            emit statusMessage(tr("[Inspector] connected.\n"));
            m_addressEdit->setEnabled(false);
            m_portSpinBox->setEnabled(false);

            if (!m_client) {
                m_client = new QmlEngineDebug(m_conn, this);
                m_objectTreeWidget->setEngineDebug(m_client);
                m_propertiesWidget->setEngineDebug(m_client);
                m_watchTableModel->setEngineDebug(m_client);
                m_expressionWidget->setEngineDebug(m_client);
            }
            
            m_objectTreeWidget->clear();
            m_propertiesWidget->clear();
            m_expressionWidget->clear();
            m_watchTableModel->removeAllWatches();
            m_frameRateWidget->reset(m_conn);

            reloadEngines();
            break;
        }
        case QAbstractSocket::ClosingState:
            emit statusMessage(tr("[Inspector] closing..."));
            break;
    }
}

void QmlInspectorMode::connectionError()
{
    emit statusMessage(tr("[Inspector] error: (%1) %2", "%1=error code, %2=error message")
            .arg(m_conn->error()).arg(m_conn->errorString()));
}

void QmlInspectorMode::initActions()
{
    m_actions.startAction = new QAction(tr("Start Inspector"), this);
    m_actions.startAction->setIcon(QIcon(ProjectExplorer::Constants::ICON_RUN));
    
    m_actions.stopAction = new QAction(tr("Stop Inspector"), this);
    m_actions.stopAction->setIcon(QIcon(ProjectExplorer::Constants::ICON_STOP));

    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();
    Core::UniqueIDManager *uidm = core->uniqueIDManager();

    QList<int> context;
    context << uidm->uniqueIdentifier(QmlInspector::Constants::C_INSPECTOR);

    am->registerAction(m_actions.startAction, QmlInspector::Constants::RUN, context);
    connect(m_actions.startAction, SIGNAL(triggered()), SIGNAL(startViewer()));
    
    am->registerAction(m_actions.stopAction, QmlInspector::Constants::STOP, context);
    connect(m_actions.stopAction, SIGNAL(triggered()), SIGNAL(stopViewer()));
}    


QToolButton *QmlInspectorMode::createToolButton(QAction *action)
{
    QToolButton *button = new QToolButton;
    button->setDefaultAction(action);
    return button;
}

QWidget *QmlInspectorMode::createMainView()
{
    initWidgets();

    Utils::FancyMainWindow *mainWindow = new Utils::FancyMainWindow;
    mainWindow->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    mainWindow->setDocumentMode(true);

    QBoxLayout *editorHolderLayout = new QVBoxLayout;
    editorHolderLayout->setMargin(0);
    editorHolderLayout->setSpacing(0);

    QWidget *editorAndFindWidget = new QWidget;
    editorAndFindWidget->setLayout(editorHolderLayout);
    editorHolderLayout->addWidget(new Core::EditorManagerPlaceHolder(this));
    editorHolderLayout->addWidget(new Core::FindToolBarPlaceHolder(editorAndFindWidget));

    Utils::StyledBar *treeOptionBar = new Utils::StyledBar;
    QHBoxLayout *treeOptionBarLayout = new QHBoxLayout(treeOptionBar);
    treeOptionBarLayout->setContentsMargins(5, 0, 5, 0);
    treeOptionBarLayout->setSpacing(5);
    treeOptionBarLayout->addWidget(new QLabel(tr("QML engine:")));
    treeOptionBarLayout->addWidget(m_engineSpinBox);

    QWidget *treeWindow = new QWidget;
    QVBoxLayout *treeWindowLayout = new QVBoxLayout(treeWindow);
    treeWindowLayout->setMargin(0);
    treeWindowLayout->setSpacing(0);
    treeWindowLayout->addWidget(treeOptionBar);
    treeWindowLayout->addWidget(m_objectTreeWidget);    

    Core::MiniSplitter *documentAndTree = new Core::MiniSplitter;
    documentAndTree->addWidget(editorAndFindWidget);
    documentAndTree->addWidget(new Core::RightPanePlaceHolder(this));
    documentAndTree->addWidget(treeWindow);
    documentAndTree->setStretchFactor(0, 2);
    documentAndTree->setStretchFactor(1, 0);
    documentAndTree->setStretchFactor(2, 0);

    Utils::StyledBar *configBar = new Utils::StyledBar;
    configBar->setProperty("topBorder", true);

    QHBoxLayout *configBarLayout = new QHBoxLayout(configBar);
    configBarLayout->setMargin(0);
    configBarLayout->setSpacing(5);
    
    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();    
    configBarLayout->addWidget(createToolButton(am->command(QmlInspector::Constants::RUN)->action()));
    configBarLayout->addWidget(createToolButton(am->command(QmlInspector::Constants::STOP)->action()));
    configBarLayout->addWidget(m_addressEdit);
    configBarLayout->addWidget(m_portSpinBox);
    configBarLayout->addStretch();
       
    QWidget *widgetAboveTabs = new QWidget;
    QVBoxLayout *widgetAboveTabsLayout = new QVBoxLayout(widgetAboveTabs);
    widgetAboveTabsLayout->setMargin(0);
    widgetAboveTabsLayout->setSpacing(0);    
    widgetAboveTabsLayout->addWidget(documentAndTree);
    widgetAboveTabsLayout->addWidget(configBar); 
    
    Core::MiniSplitter *mainSplitter = new Core::MiniSplitter(Qt::Vertical);    
    mainSplitter->addWidget(widgetAboveTabs);
    mainSplitter->addWidget(createBottomWindow());
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 1);    

    QWidget *centralWidget = new QWidget;
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setMargin(0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(mainSplitter);

    mainWindow->setCentralWidget(centralWidget);
   
    return mainWindow;
}

QWidget *QmlInspectorMode::createBottomWindow()
{
    Utils::FancyMainWindow *win = new Utils::FancyMainWindow;
    win->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    win->setDocumentMode(true);
    win->setTrackingEnabled(true);

    Core::MiniSplitter *leftSplitter = new Core::MiniSplitter(Qt::Vertical);    
    leftSplitter->addWidget(m_propertiesWidget);
    leftSplitter->addWidget(m_expressionWidget);
    leftSplitter->setStretchFactor(0, 2);
    leftSplitter->setStretchFactor(1, 1);

    Core::MiniSplitter *propSplitter = new Core::MiniSplitter(Qt::Horizontal);    
    propSplitter->addWidget(leftSplitter);
    propSplitter->addWidget(m_watchTableView);
    propSplitter->setStretchFactor(0, 2);
    propSplitter->setStretchFactor(1, 1);
    propSplitter->setWindowTitle(tr("Properties and Watchers"));

    QDockWidget *propertiesDock = win->addDockForWidget(propSplitter);
    win->addDockWidget(Qt::TopDockWidgetArea, propertiesDock);

    QDockWidget *frameRateDock = win->addDockForWidget(m_frameRateWidget);
    win->addDockWidget(Qt::TopDockWidgetArea, frameRateDock);

    // stack the dock widgets as tabs
    win->tabifyDockWidget(frameRateDock, propertiesDock);

    return win;
}

QWidget *QmlInspectorMode::createModeWindow()
{
    // right-side window with editor, output etc.
    Core::MiniSplitter *mainWindowSplitter = new Core::MiniSplitter;
    mainWindowSplitter->addWidget(createMainView());
    mainWindowSplitter->addWidget(new Core::OutputPanePlaceHolder(this));
    mainWindowSplitter->setStretchFactor(0, 10);
    mainWindowSplitter->setStretchFactor(1, 0);
    mainWindowSplitter->setOrientation(Qt::Vertical);

    // navigation + right-side window
    Core::MiniSplitter *splitter = new Core::MiniSplitter;
    splitter->addWidget(new Core::NavigationWidgetPlaceHolder(this));
    splitter->addWidget(mainWindowSplitter);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    return splitter;
}

void QmlInspectorMode::initWidgets()
{
    m_objectTreeWidget = new ObjectTree;
    m_propertiesWidget = new ObjectPropertiesView;
    m_watchTableView = new WatchTableView(m_watchTableModel);
    m_frameRateWidget = new CanvasFrameRate;
    m_expressionWidget = new ExpressionQueryWidget(ExpressionQueryWidget::ShellMode);

    // FancyMainWindow uses widgets' window titles for tab labels
    m_objectTreeWidget->setWindowTitle(tr("Object Tree"));
    m_frameRateWidget->setWindowTitle(tr("Frame rate"));
    
    m_watchTableView->setModel(m_watchTableModel);
    WatchTableHeaderView *header = new WatchTableHeaderView(m_watchTableModel);
    m_watchTableView->setHorizontalHeader(header);

    connect(m_objectTreeWidget, SIGNAL(currentObjectChanged(QmlDebugObjectReference)),
            m_propertiesWidget, SLOT(reload(QmlDebugObjectReference)));
    connect(m_objectTreeWidget, SIGNAL(expressionWatchRequested(QmlDebugObjectReference,QString)),
            m_watchTableModel, SLOT(expressionWatchRequested(QmlDebugObjectReference,QString)));

    connect(m_propertiesWidget, SIGNAL(activated(QmlDebugObjectReference,QmlDebugPropertyReference)),
            m_watchTableModel, SLOT(togglePropertyWatch(QmlDebugObjectReference,QmlDebugPropertyReference)));

    connect(m_watchTableModel, SIGNAL(watchCreated(QmlDebugWatch*)),
            m_propertiesWidget, SLOT(watchCreated(QmlDebugWatch*)));

    connect(m_watchTableModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            m_watchTableView, SLOT(scrollToBottom()));

    connect(m_watchTableView, SIGNAL(objectActivated(int)),
            m_objectTreeWidget, SLOT(setCurrentObject(int)));    

    connect(m_objectTreeWidget, SIGNAL(currentObjectChanged(QmlDebugObjectReference)),
            m_expressionWidget, SLOT(setCurrentObject(QmlDebugObjectReference)));

    m_addressEdit = new QLineEdit;
    m_addressEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_addressEdit->setText("127.0.0.1");

    m_portSpinBox = new QSpinBox;
    m_portSpinBox->setMinimum(1024);
    m_portSpinBox->setMaximum(20000);
    m_portSpinBox->setValue(3768);

    m_engineSpinBox = new EngineSpinBox;
    m_engineSpinBox->setEnabled(false);
    connect(m_engineSpinBox, SIGNAL(valueChanged(int)),
            SLOT(queryEngineContext(int))); 
}

void QmlInspectorMode::reloadEngines()
{
    if (m_engineQuery) {
        emit statusMessage("[Inspector] Waiting for response to previous engine query");
        return;
    }

    m_engineSpinBox->setEnabled(false);

    m_engineQuery = m_client->queryAvailableEngines(this);
    if (!m_engineQuery->isWaiting())
        enginesChanged();
    else
        QObject::connect(m_engineQuery, SIGNAL(stateChanged(State)), 
                         this, SLOT(enginesChanged()));    
}

void QmlInspectorMode::enginesChanged()
{
    m_engineSpinBox->clearEngines();

    QList<QmlDebugEngineReference> engines = m_engineQuery->engines();
    delete m_engineQuery; m_engineQuery = 0;

    if (engines.isEmpty())
        qWarning("qmldebugger: no engines found!");

    m_engineSpinBox->setEnabled(true);

    for (int i=0; i<engines.count(); i++)
        m_engineSpinBox->addEngine(engines.at(i).debugId(), engines.at(i).name());

    if (engines.count() > 0) {
        m_engineSpinBox->setValue(engines.at(0).debugId());
        queryEngineContext(engines.at(0).debugId());
    }
}

void QmlInspectorMode::queryEngineContext(int id)
{
    if (id < 0)
        return;

    if (m_contextQuery) {
        delete m_contextQuery;
        m_contextQuery = 0;
    }

    m_contextQuery = m_client->queryRootContexts(QmlDebugEngineReference(id), this);
    if (!m_contextQuery->isWaiting())
        contextChanged();
    else
        QObject::connect(m_contextQuery, SIGNAL(stateChanged(State)),
                         this, SLOT(contextChanged()));
}

void QmlInspectorMode::contextChanged()
{
    //dump(m_contextQuery->rootContext(), 0);

    foreach (const QmlDebugObjectReference &object, m_contextQuery->rootContext().objects())
        m_objectTreeWidget->reload(object.debugId());

    delete m_contextQuery; m_contextQuery = 0;
}

QT_END_NAMESPACE

#include "qmlinspectormode.moc"

