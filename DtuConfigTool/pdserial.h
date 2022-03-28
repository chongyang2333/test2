#ifndef PDSERIAL_H
#define PDSERIAL_H


#include <QSerialPort>
#include <QByteArray>
#include <QString>

class PdSerial
{
public:
    PdSerial();
    ~PdSerial(){}
    /**
     * @brief open 打开串口
     * @param name 串口名称
     * @return
     */
    bool open(QString name, int baudRate);

    /**
     * @brief 关闭串口
     */
    void close();

    /**
     * @brief changeBaudRate
     * @param baudRate
     */
    bool changeBaudRate(int baudRate);

    /**
     * @brief waitForReadReady阻塞等待接口有数据可读，直到超时
     * @param msec 超时时间
     * @return 超时或者其他错误返回false，有数据可读返回true
     */
    bool waitForReadReady(int msec);

    /**
     * @brief read 读取串口数据
     * @param len需要读取到的字节数
     * @return 返回读取到数据
     */
    QByteArray read(int len);

    /**
     * @brief write 将数据写入串口
     * @param data 需要写入的数据
     * @return 写入的字节数
     */
    int write(QByteArray data);

    void clear();



public:
    QSerialPort mSerialPort;
};

#endif // PDSERIAL_H
