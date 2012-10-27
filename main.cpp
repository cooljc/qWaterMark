#include <QtGui/QApplication>
#include "watermark.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("COOLJC");
    QCoreApplication::setOrganizationDomain("cooljc.co.uk");
    QCoreApplication::setApplicationName("qWaterMark");

    WaterMark w;
    w.show();
    
    return a.exec();
}
