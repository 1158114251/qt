#include "baselineserver.h"
#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QHostInfo>
#include <QTextStream>

QString BaselineServer::storage;

BaselineServer::BaselineServer(QObject *parent)
    : QTcpServer(parent)
{
    QFileInfo me(QCoreApplication::applicationFilePath());
    meLastMod = me.lastModified();
    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, SIGNAL(timeout()), this, SLOT(heartbeat()));
    heartbeatTimer->start(HEARTBEAT*1000);
}

QString BaselineServer::storagePath()
{
    if (storage.isEmpty()) {
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cdUp();
        storage =  dir.path() + QLatin1String("/storage/");
    }
    return storage;
}

void BaselineServer::incomingConnection(int socketDescriptor)
{
    qDebug() << "Server: New connection!";
    BaselineThread *thread = new BaselineThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void BaselineServer::heartbeat()
{
    // The idea is to exit to be restarted when modified, as soon as not actually serving
    QFileInfo me(QCoreApplication::applicationFilePath());
    if (me.lastModified() == meLastMod)
        return;

    // (could close() here to avoid accepting new connections, to avoid livelock)
    // also, could check for a timeout to force exit, to avoid hung threads blocking
    bool isServing = false;
    foreach(BaselineThread *thread, findChildren<BaselineThread *>()) {
        if (thread->isRunning()) {
            isServing = true;
            break;
        }
    }

    if (!isServing)
        QCoreApplication::exit();
}

BaselineThread::BaselineThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{
}

void BaselineThread::run()
{
    BaselineHandler handler(socketDescriptor);
    exec();
}


BaselineHandler::BaselineHandler(int socketDescriptor)
    : QObject(), connectionEstablished(false)
{
    runId = QDateTime::currentDateTime().toString(QLatin1String("MMMdd-hhmmss"));

    connect(&proto.socket, SIGNAL(readyRead()), this, SLOT(receiveRequest()));
    connect(&proto.socket, SIGNAL(disconnected()), this, SLOT(receiveDisconnect()));
    proto.socket.setSocketDescriptor(socketDescriptor);
}

QString BaselineHandler::logtime()
{
    return QTime::currentTime().toString(QLatin1String("mm:ss.zzz"));
}

void BaselineHandler::receiveRequest()
{
    if (!connectionEstablished) {
        if (!proto.acceptConnection(&plat)) {
            qWarning() << runId << logtime() << "Accepting new connection failed. " << proto.errorMessage();
            QThread::currentThread()->exit(1);
            return;
        }
        connectionEstablished = true;
        qDebug() << runId << logtime() << "Connection established with" << plat.hostname << "Qt version:" << plat.qtVersion << plat.buildKey;
        return;
    }

    QByteArray block;
    BaselineProtocol::Command cmd;
    if (!proto.receiveBlock(&cmd, &block)) {
        qWarning() << runId << logtime() << "Command reception failed. "<< proto.errorMessage();
        QThread::currentThread()->exit(1);
        return;
    }

    switch(cmd) {
    case BaselineProtocol::RequestBaselineChecksums:
        provideBaselineChecksums(block);
        break;
    case BaselineProtocol::RequestBaseline:
        provideBaseline(block);
        break;
    case BaselineProtocol::AcceptNewBaseline:
        storeImage(block, true);
        break;
    case BaselineProtocol::AcceptMismatch:
        storeImage(block, false);
        break;
    default:
        qWarning() << runId << logtime() << "Unknown command received. " << proto.errorMessage();
        QThread::currentThread()->exit(1);
    }
}


void BaselineHandler::provideBaselineChecksums(const QByteArray &itemListBlock)
{
    ImageItemList itemList;
    QDataStream ds(itemListBlock);
    ds >> itemList;
    qDebug() << runId << logtime() << "Received request for checksums for" << itemList.count() << "items";

    for (ImageItemList::iterator i = itemList.begin(); i != itemList.end(); ++i) {
        i->imageChecksum = 0;
        QString prefix = pathForItem(*i, true);
        QFile file(prefix + QLatin1String("metadata"));
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream ts(&file);
            ts >> i->imageChecksum;
            file.close();
            i->status = ImageItem::Ok;
        }
        if (!i->imageChecksum)
            i->status = ImageItem::BaselineNotFound;
    }

    QByteArray block;
    QDataStream ods(&block, QIODevice::WriteOnly);
    ods << itemList;
    proto.sendBlock(BaselineProtocol::Ack, block);
}


void BaselineHandler::provideBaseline(const QByteArray &caseId)
{
    qDebug() << runId << logtime() << "Received request for baseline" << caseId;

    //### rewrite to use ImageItem
    /*
    QFile file(pathForCaseId(caseId));
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << runId << logtime() << "baseline not found.";
        proto.sendBlock(BaselineProtocol::BaselineNotPresent, caseId);
        return;
    }

    proto.sendBlock(BaselineProtocol::AcceptBaseline, file.readAll());
    */
}


void BaselineHandler::storeImage(const QByteArray &itemBlock, bool isBaseline)
{
    QDataStream ds(itemBlock);
    ImageItem item;
    ds >> item;

    QString prefix = pathForItem(item, isBaseline);
    qDebug() << runId << logtime() << "Received" << (isBaseline ? "baseline" : "mismatched") << "image for:" << item.scriptName << "Storing in" << prefix;

    QString dir = prefix.section(QDir::separator(), 0, -2);
    QDir cwd;
    if (!cwd.exists(dir))
        cwd.mkpath(dir);
    item.image.save(prefix + QLatin1String(FileFormat), FileFormat);

    //# Could use QSettings or XML or even DB, could use common file for whole dir or even whole storage - but for now, keep it simple
    QFile file(prefix + QLatin1String("metadata"));
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream ts(&file);
    ts << hex << showbase << item.imageChecksum << reset << endl;
    file.close();

    QByteArray msg(isBaseline ? "Baseline" : "Mismatching" );
    msg += " image stored in "
           + QHostInfo::localHostName().toLatin1() + '.'
           + QHostInfo::localDomainName().toLatin1() + ':'
           + QFileInfo(file).absoluteFilePath().toLatin1();
    proto.sendBlock(BaselineProtocol::Ack, msg);
    qDebug() << runId << logtime() << "Storing done.";
}


void BaselineHandler::receiveDisconnect()
{
    qDebug() << runId << logtime() << "Client disconnected.";
    QThread::currentThread()->exit(0);
}


QString BaselineHandler::pathForItem(const ImageItem &item, bool isBaseline)
{
    QString storePath = BaselineServer::storagePath();
    storePath += plat.buildKey.section(QLatin1Char(' '), 1, 1) + QLatin1String("_Qt-")
                 + plat.qtVersion + QDir::separator();
    if (isBaseline)
        storePath += QLatin1String("baselines") + QDir::separator();
    else
        storePath += runId + QDir::separator();
    //#? QString itemName = item.scriptName.replace(item.scriptName.lastIndexOf('.'), '_');
    return storePath + item.scriptName + QLatin1Char('.');
}


// - transferring and comparing checksums instead of images
// - then we could now if multiple error/imgs are really the same (and just store it once)
// - e.g. using db
