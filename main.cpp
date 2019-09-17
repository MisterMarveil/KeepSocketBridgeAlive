#include <QCoreApplication>
#include <socket.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Socket sock(&a);
    return a.exec();
}
