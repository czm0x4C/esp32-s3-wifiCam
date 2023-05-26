#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include <QTcpSocket>
#include <QUdpSocket>

class tcpSocketsInfo         /*将socket的信息和他们的IP和Port绑定*/
{
public:
    QTcpSocket* socket; //socket指针
    QString clientIP;   //ip端口字符串
    QString clientPORT; //端口字符串
    QString targetIP;   //TCP连接的目标对象IP
    QString targetPORT; //TCP连接的目标对象PORT
    char isTCPActivity;
};
class udpSocketsInfo         /*将socket的信息和他们的IP和Port绑定*/
{
public:
    QUdpSocket* socket; //socket指针
    QString clientIP;   //ip端口字符串
    QString clientPORT; //端口字符串
    QString targetIP;   //TCP连接的目标对象IP
    QString targetPORT; //TCP连接的目标对象PORT
    char isUDPActivity;
};
struct tcpCameraInfo
{
    QString cameraIP;
    QString cameraPORT;
    char isTCPActivity;
};
struct udpCameraInfo
{
    QString cameraIP;
    QString cameraPORT;
    char isUDPActivity;
};
#endif // MAIN_H
