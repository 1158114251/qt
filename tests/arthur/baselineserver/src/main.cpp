#include <QtCore/QCoreApplication>
#include "baselineserver.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().contains(QLatin1String("-testmapping"))) {
        BaselineHandler h;
        h.testPathMapping();
        return 0;
    }

    BaselineServer server;
    if (!server.listen(QHostAddress::Any, BaselineProtocol::ServerPort)) {
        qWarning("Failed to listen!");
        return 1;
    }

    qDebug() << "Listening for connections";
    return a.exec();
}
