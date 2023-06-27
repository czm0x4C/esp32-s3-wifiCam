/*
    网络摄像头数据转发服务器
    Date:2023/02/24
    更新内容:

 */

#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
