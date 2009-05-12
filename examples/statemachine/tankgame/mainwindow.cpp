#include "mainwindow.h"
#include "tankitem.h"
#include "rocketitem.h"
#include "plugin.h"

#include <QStateMachine>
#include <QGraphicsView>
#include <QAction>
#include <QMenuBar>
#include <QState>
#include <QHistoryState>
#include <QTimer>
#include <QFileDialog>
#include <QPluginLoader>
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_scene(0), m_machine(0), m_runningState(0), m_started(false)
{
    init();
}

MainWindow::~MainWindow()
{
}

void MainWindow::addWall(const QRectF &wall)
{
    QGraphicsRectItem *item = new QGraphicsRectItem;
    item->setRect(wall);
    item->setBrush(Qt::darkGreen);
    item->setPen(QPen(Qt::black, 0));

    m_scene->addItem(item);
}

void MainWindow::init()
{
    setWindowTitle("Pluggable Tank Game");

    QGraphicsView *view = new QGraphicsView(this);
    view->setRenderHints(QPainter::Antialiasing);
    setCentralWidget(view);

    m_scene = new QGraphicsScene(this);
    view->setScene(m_scene);

    QRectF sceneRect = QRectF(-200.0, -200.0, 400.0, 400.0);
    m_scene->setSceneRect(sceneRect);

    {
        TankItem *item = new TankItem(this);

        item->setPos(m_scene->sceneRect().topLeft() + QPointF(15.0, 15.0));
        item->setDirection(45.0);
        item->setColor(Qt::red);

        m_spawns.append(item);
    }

    {
        TankItem *item = new TankItem(this);

        item->setPos(m_scene->sceneRect().topRight() + QPointF(-15.0, 15.0));
        item->setDirection(135.0);
        item->setColor(Qt::green);

        m_spawns.append(item);
    }

    {
        TankItem *item = new TankItem(this);

        item->setPos(m_scene->sceneRect().bottomRight() + QPointF(-15.0, -15.0));
        item->setDirection(225.0);
        item->setColor(Qt::blue);

        m_spawns.append(item);
    }

    {
        TankItem *item = new TankItem(this);

        item->setPos(m_scene->sceneRect().bottomLeft() + QPointF(15.0, -15.0));
        item->setDirection(315.0);
        item->setColor(Qt::yellow);

        m_spawns.append(item);
    }

    QPointF centerOfMap = sceneRect.center();

    addWall(QRectF(centerOfMap + QPointF(-50.0, -60.0), centerOfMap + QPointF(50.0, -50.0)));
    addWall(QRectF(centerOfMap - QPointF(-50.0, -60.0), centerOfMap - QPointF(50.0, -50.0)));
    addWall(QRectF(centerOfMap + QPointF(-50.0, -50.0), centerOfMap + QPointF(-40.0, 50.0)));
    addWall(QRectF(centerOfMap - QPointF(-50.0, -50.0), centerOfMap - QPointF(-40.0, 50.0)));
   
    addWall(QRectF(sceneRect.topLeft() + QPointF(sceneRect.width() / 2.0 - 5.0, -10.0),
                   sceneRect.topLeft() + QPointF(sceneRect.width() / 2.0 + 5.0, 100.0)));
    addWall(QRectF(sceneRect.bottomLeft() + QPointF(sceneRect.width() / 2.0 - 5.0, 10.0),
                   sceneRect.bottomLeft() + QPointF(sceneRect.width() / 2.0 + 5.0, -100.0)));
    addWall(QRectF(sceneRect.topLeft() + QPointF(-10.0, sceneRect.height() / 2.0 - 5.0),
                   sceneRect.topLeft() + QPointF(100.0, sceneRect.height() / 2.0 + 5.0)));
    addWall(QRectF(sceneRect.topRight() + QPointF(10.0, sceneRect.height() / 2.0 - 5.0),
                   sceneRect.topRight() + QPointF(-100.0, sceneRect.height() / 2.0 + 5.0)));


    QAction *addTankAction = menuBar()->addAction("&Add tank");   
    QAction *runGameAction = menuBar()->addAction("&Run game");    
    runGameAction->setObjectName("runGameAction");
    QAction *stopGameAction = menuBar()->addAction("&Stop game");        
    menuBar()->addSeparator();
    QAction *quitAction = menuBar()->addAction("&Quit");
    
    connect(addTankAction, SIGNAL(triggered()), this, SLOT(addTank()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
        
    m_machine = new QStateMachine(this);    
    QState *stoppedState = new QState(m_machine->rootState());        
    stoppedState->setObjectName("stoppedState");
    stoppedState->assignProperty(runGameAction, "enabled", true);
    stoppedState->assignProperty(stopGameAction, "enabled", false);
    stoppedState->assignProperty(this, "started", false);
    m_machine->setInitialState(stoppedState);

    QState *spawnsAvailable = new QState(stoppedState);
    spawnsAvailable->setObjectName("spawnsAvailable");
    spawnsAvailable->assignProperty(addTankAction, "enabled", true);

    QState *noSpawnsAvailable = new QState(stoppedState);
    noSpawnsAvailable->setObjectName("noSpawnsAvailable");
    noSpawnsAvailable->assignProperty(addTankAction, "enabled", false);

    spawnsAvailable->addTransition(this, SIGNAL(mapFull()), noSpawnsAvailable);

    QHistoryState *hs = new QHistoryState(stoppedState);
    hs->setDefaultState(spawnsAvailable);

    stoppedState->setInitialState(hs);

//! [0]
    m_runningState = new QState(QState::ParallelStates, m_machine->rootState());
//! [0]
    m_runningState->setObjectName("runningState");
    m_runningState->assignProperty(addTankAction, "enabled", false);
    m_runningState->assignProperty(runGameAction, "enabled", false);
    m_runningState->assignProperty(stopGameAction, "enabled", true);
        
    stoppedState->addTransition(runGameAction, SIGNAL(triggered()), m_runningState);
    m_runningState->addTransition(stopGameAction, SIGNAL(triggered()), stoppedState);

    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(runStep()));
    connect(m_runningState, SIGNAL(entered()), timer, SLOT(start()));    
    connect(m_runningState, SIGNAL(exited()), timer, SLOT(stop()));

    m_machine->start();
    m_time.start();
}   

void MainWindow::runStep()
{
    if (!m_started) {
        m_time.restart();
        m_started = true;
    } else {
        int elapsed = m_time.elapsed();
        if (elapsed > 0) {
            m_time.restart();
            qreal elapsedSecs = elapsed / 1000.0;
            QList<QGraphicsItem *> items = m_scene->items();
            foreach (QGraphicsItem *item, items) {
                if (GameItem *gameItem = qgraphicsitem_cast<GameItem *>(item))
                    gameItem->idle(elapsedSecs);
            }
        }
    }
}

void MainWindow::addRocket()
{
    TankItem *tankItem = qobject_cast<TankItem *>(sender());
    if (tankItem != 0) {
        RocketItem *rocketItem = new RocketItem;

        QPointF s = tankItem->mapToScene(QPointF(tankItem->boundingRect().right() + 10.0, 
                                                 tankItem->boundingRect().center().y()));
        rocketItem->setPos(s);
        rocketItem->setDirection(tankItem->direction());
        m_scene->addItem(rocketItem);
    }
}

void MainWindow::addTank()
{
    Q_ASSERT(!m_spawns.isEmpty());

    QDir pluginsDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif

    pluginsDir.cd("plugins");

    QStringList itemNames;
    QList<Plugin *> items;
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *possiblePlugin = loader.instance();
        if (Plugin *plugin = qobject_cast<Plugin *>(possiblePlugin)) {
            QString objectName = possiblePlugin->objectName();
            if (objectName.isEmpty())
                objectName = fileName;

            itemNames.append(objectName);
            items.append(plugin);
        }
    }

    if (items.isEmpty()) {
        QMessageBox::information(this, "No tank types found", "Please build the errorstateplugins directory");
        return;
    }

    bool ok;
//! [1]
    QString selectedName = QInputDialog::getItem(this, "Select a tank type", "Tank types", 
        itemNames, 0, false, &ok);
//! [1]
    
    if (ok && !selectedName.isEmpty()) {
        int idx = itemNames.indexOf(selectedName);
        if (Plugin *plugin = idx >= 0 ? items.at(idx) : 0) {
            TankItem *tankItem = m_spawns.takeLast();
            m_scene->addItem(tankItem);
            connect(tankItem, SIGNAL(cannonFired()), this, SLOT(addRocket()));
            if (m_spawns.isEmpty())
                emit mapFull();
            
            QState *region = new QState(m_runningState);
//! [2]
            QState *pluginState = plugin->create(region, tankItem);
//! [2]
            region->setInitialState(pluginState);


            // If the plugin has an error it is disabled
            QState *errorState = new QState(region);
            errorState->assignProperty(tankItem, "enabled", false);
            pluginState->setErrorState(errorState);
        }
    }
}

