
#include "baselineprotocol.h"
#include <QLibraryInfo>
#include <QImage>
#include <QBuffer>
#include <QHostInfo>

PlatformInfo::PlatformInfo(bool useLocal)
{
    if (useLocal) {
        buildKey = QLibraryInfo::buildKey();
        qtVersion = QLatin1String(qVersion());
        hostname = QHostInfo::localHostName();
    }
}

QDataStream & operator<< (QDataStream &stream, const PlatformInfo &pinfo)
{
    stream << pinfo.buildKey << pinfo.qtVersion << pinfo.hostname;
    return stream;
}

QDataStream & operator>> (QDataStream& stream, PlatformInfo& pinfo)
{
    stream >> pinfo.buildKey >> pinfo.qtVersion >> pinfo.hostname;
    return stream;
}

ImageItem &ImageItem::operator=(const ImageItem &other)
{
    scriptName = other.scriptName;
    scriptChecksum = other.scriptChecksum;
    status = other.status;
    renderFormat = other.renderFormat;
    engine = other.engine;
    image = other.image;
    imageChecksum = other.imageChecksum;
    return *this;
}

quint64 ImageItem::computeChecksum(const QImage &image)
{
    //### Just fake it for now
    return qChecksum((const char *)image.constScanLine(50), image.bytesPerLine());
}

QDataStream & operator<< (QDataStream &stream, const ImageItem &ii)
{
    stream << ii.scriptName << ii.scriptChecksum << quint8(ii.status) << quint8(ii.renderFormat)
            << quint8(ii.engine) << ii.image << ii.imageChecksum;
    return stream;
}

QDataStream & operator>> (QDataStream &stream, ImageItem &ii)
{
    quint8 encFormat, encStatus, encEngine;
    stream >> ii.scriptName >> ii.scriptChecksum >> encStatus >> encFormat
            >> encEngine >> ii.image >> ii.imageChecksum;
    ii.renderFormat = QImage::Format(encFormat);
    ii.status = ImageItem::ItemStatus(encStatus);
    ii.engine = ImageItem::GraphicsEngine(encEngine);
    return stream;
}

BaselineProtocol::~BaselineProtocol()
{
    socket.close();
    if (socket.state() != QTcpSocket::UnconnectedState)
        socket.waitForDisconnected(Timeout);
}


bool BaselineProtocol::connect()
{
    errMsg.clear();
    //###TBD: determine server address; for now local devhost
    QLatin1String serverName("chimera.europe.nokia.com");

    socket.connectToHost(serverName, ServerPort);

    if (!socket.waitForConnected(Timeout)) {
        errMsg += QLatin1String("TCP connectToHost failed. Host:") + serverName + QLatin1String(" port:") + QString::number(ServerPort);
        return false;
    }

    PlatformInfo pi(true);
    QByteArray block;
    QDataStream ds(&block, QIODevice::ReadWrite);
    ds << pi;
    if (!sendBlock(AcceptPlatformInfo, block)) {
        errMsg += QLatin1String("Failed to send data to server.");
        return false;
    }

    Command cmd = Ack;
    if (!receiveBlock(&cmd, &block) || cmd != Ack) {
        errMsg += QLatin1String("Failed to get response from server.");
        return false;
    }

    return true;
}


bool BaselineProtocol::acceptConnection(PlatformInfo *pi)
{
    errMsg.clear();

    QByteArray block;
    Command cmd = AcceptPlatformInfo;
    if (!receiveBlock(&cmd, &block) || cmd != AcceptPlatformInfo)
        return false;

    if (pi) {
        QDataStream ds(block);
        ds >> *pi;
    }

    if (!sendBlock(Ack, QByteArray()))
        return false;
    return true;
}


bool BaselineProtocol::requestBaselineChecksums(ImageItemList *itemList)
{
    errMsg.clear();
    if (!itemList)
        return false;
    QByteArray block;
    QDataStream ds(&block, QIODevice::ReadWrite);
    ds << *itemList;
    if (!sendBlock(RequestBaselineChecksums, block))
        return false;
    Command cmd;
    if (!receiveBlock(&cmd, &block))
        return false;
    ds.device()->seek(0);
    ds >> *itemList;
    return true;
}


bool BaselineProtocol::submitNewBaseline(const ImageItem &item, QByteArray *serverMsg)
{
    Command cmd;
    return (sendItem(AcceptNewBaseline, item) && receiveBlock(&cmd, serverMsg) && cmd == Ack);
}


bool BaselineProtocol::submitMismatch(const ImageItem &item, QByteArray *serverMsg)
{
    Command cmd;
    return (sendItem(AcceptMismatch, item) && receiveBlock(&cmd, serverMsg) && cmd == Ack);
}


bool BaselineProtocol::sendItem(Command cmd, const ImageItem &item)
{
    errMsg.clear();
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    QDataStream ds(&buf);
    ds << item;
    if (!sendBlock(cmd, buf.data())) {
        errMsg.prepend(QLatin1String("Failed to submit image to server. "));
        return false;
    }
    return true;
}


bool BaselineProtocol::sendBlock(Command cmd, const QByteArray &block)
{
    QDataStream s(&socket);
    // TBD: set qds version as a constant
    s << quint16(ProtocolVersion) << quint16(cmd);
    s.writeBytes(block.constData(), block.size());
    return true;
}


bool BaselineProtocol::receiveBlock(Command *cmd, QByteArray *block)
{
    while (socket.bytesAvailable() < int(2*sizeof(quint16) + sizeof(quint32))) {
        if (!socket.waitForReadyRead(Timeout))
            return false;
    }
    QDataStream ds(&socket);
    quint16 rcvProtocolVersion, rcvCmd;
    ds >> rcvProtocolVersion >> rcvCmd;
    if (rcvProtocolVersion != ProtocolVersion) {
        // TBD: More resilient handling of this case; the server should accept client's version
        errMsg = QLatin1String("Server protocol version mismatch, received:") + QString::number(rcvProtocolVersion);

        return false;
    }
    if (cmd)
        *cmd = Command(rcvCmd);

    QByteArray uMsg;
    quint32 remaining;
    ds >> remaining;
    uMsg.resize(remaining);
    int got = 0;
    char* uMsgBuf = uMsg.data();
    do {
        got = ds.readRawData(uMsgBuf, remaining);
        remaining -= got;
        uMsgBuf += got;
    } while (remaining && got >= 0 && socket.waitForReadyRead(Timeout));

    if (got < 0)
        return false;

    if (block)
        *block = uMsg;

    return true;
}


QString BaselineProtocol::errorMessage()
{
    QString ret = errMsg;
    if (socket.error() >= 0)
        ret += QLatin1String(" Socket state: ") + socket.errorString();
    return ret;
}
