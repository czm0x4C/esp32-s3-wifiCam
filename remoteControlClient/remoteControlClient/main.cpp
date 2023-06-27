#include "mainwidget.h"

#include <QApplication>
/********************************/
//工程创建2023/2/2
//程序适用于单片机的网络摄像头
/********************************/
/*








*/


#include <QImage>
#include <QImageReader>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWidget w;
    w.show();
    //qDebug() << "support format: " << QImageReader::supportedImageFormats();
    return a.exec();
}
