#include "mainwindow.h"
#include "tankitem.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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

    addWall(QRectF(sceneRect.topLeft() + QPointF(sceneRect.width() / 2.0 - 5.0, 0.0),
                   sceneRect.topLeft() + QPointF(sceneRect.width() / 2.0 + 5.0, 100.0)));
    addWall(QRectF(sceneRect.bottomLeft() + QPointF(sceneRect.width() / 2.0 - 5.0, 0.0),
                   sceneRect.bottomLeft() + QPointF(sceneRect.width() / 2.0 + 5.0, -100.0)));
    addWall(QRectF(sceneRect.topLeft() + QPointF(0.0, sceneRect.height() / 2.0 - 5.0),
                   sceneRect.topLeft() + QPointF(100.0, sceneRect.height() / 2.0 + 5.0)));
    addWall(QRectF(sceneRect.topRight() + QPointF(0.0, sceneRect.height() / 2.0 - 5.0),
                   sceneRect.topRight() + QPointF(-100.0, sceneRect.height() / 2.0 + 5.0)));


    QAction *addTankAction = menuBar()->addAction("&Add tank");   
    QAction *runGameAction = menuBar()->addAction("&Run game");    
    QAction *stopGameAction = menuBar()->addAction("&Stop game");        
    menuBar()->addSeparator();
    QAction *quitAction = menuBar()->addAction("&Quit");
    
    connect(addTankAction, SIGNAL(triggered()), this, SLOT(addTank()));
    connect(stopGameAction, SIGNAL(triggered()), this, SIGNAL(gameOver()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
        
    m_machine = new QStateMachine(this);
    m_machine->setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    
    QState *stoppedState = new QState(m_machine->rootState());    
    
    stoppedState->assignProperty(runGameAction, "enabled", true);
    stoppedState->assignProperty(stopGameAction, "enabled", false);
    m_machine->setInitialState(stoppedState);

    QState *spawnsAvailable = new QState(stoppedState);
    spawnsAvailable->assignProperty(addTankAction, "enabled", true);

    QState *noSpawnsAvailable = new QState(stoppedState);
    noSpawnsAvailable->assignProperty(addTankAction, "enabled", false);

    spawnsAvailable->addTransition(this, SIGNAL(mapFull()), noSpawnsAvailable);

    QHistoryState *hs = stoppedState->addHistoryState();
    hs->setDefaultState(spawnsAvailable);
    stoppedState->setInitialState(spawnsAvailable);

    m_runningState = new QState(QState::ParallelGroup, m_machine->rootState());
    m_runningState->assignProperty(addTankAction, "enabled", false);
    m_runningState->assignProperty(runGameAction, "enabled", false);
    m_runningState->assignProperty(stopGameAction, "enabled", true);
        
    stoppedState->addTransition(runGameAction, SIGNAL(triggered()), m_runningState);
    m_runningState->addTransition(this, SIGNAL(gameOver()), stoppedState);

    m_machine->start();

    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(runStep()));
    connect(m_runningState, SIGNAL(entered()), timer, SLOT(start()));    
    connect(m_runningState, SIGNAL(exited()), timer, SLOT(stop()));    

    m_time.start();
}   

void MainWindow::runStep()
{
    int elapsed = m_time.elapsed();
    if (elapsed > 0) {
        m_time.restart();
        qreal elapsedSecs = elapsed / 1000.0;
        QList<TankItem *> tankItems = qFindChildren<TankItem *>(this);
        foreach (TankItem *tankItem, tankItems) 
            tankItem->idle(elapsedSecs);        
    }
}

void MainWindow::addTank()
{
    Q_ASSERT(!m_spawns.isEmpty());

    QString fileName = QFileDialog::getOpenFileName(this, "Select plugin file", 
        "plugins/", "*.dll");
    QPluginLoader loader(fileName);
    
    Plugin *plugin = qobject_cast<Plugin *>(loader.instance());
    if (plugin != 0) {
        TankItem *tankItem = m_spawns.takeLast();
        m_scene->addItem(tankItem);
        if (m_spawns.isEmpty())
            emit mapFull();

        plugin->create(m_runningState, tankItem);
    }
}

