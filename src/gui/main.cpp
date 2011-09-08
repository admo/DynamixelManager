#include "dynamixelmanager.h"
#include "selectserialport.h"

#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(icons);
    DynamixelManager w;

    w.show();
    return a.exec();
}
