#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QByteArray>

/**
 * @brief 保存三元组信息
 */
struct PdDeviceRegisterInfo {
    QByteArray productKey;
    QByteArray deviceName;
    QByteArray deviceSecret;
    QByteArray isp; //枚举值：aliyun aws emqx(云平台提供的接口)
};


struct PdDeviceTcpRsgisterInfo
{
    QByteArray tcpHostAdress;
    QByteArray tcpHostPort;
    QByteArray tcpHostSecret;
    QByteArray pid;

};
#endif // DEVICEINFO_H
