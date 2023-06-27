#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include <QTcpSocket>
#include <QUdpSocket>

class udpSocketsInfo         /*将socket的信息和他们的IP和Port绑定*/
{
public:
    QUdpSocket* socket; //socket指针
    QString clientIP;   //ip端口字符串
    QString clientPORT; //端口字符串
    QString targetIP;   //TCP连接的目标对象IP
    QString targetPORT; //TCP连接的目标对象PORT
    char isUDPActivity; /* 判断UDP连接是否持续 */

    char isOnReadRecordVideoThread;/* 是否进入视频回放任务线程 */

};

#endif // MAIN_H
