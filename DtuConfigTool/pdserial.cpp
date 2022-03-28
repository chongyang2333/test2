#include "pdserial.h"

#include <QThread>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

PdSerial::PdSerial()
{

}

bool PdSerial::open(QString name, int baudRate)
{
    mSerialPort.setPortName(name);
    // 打开串口
    if (mSerialPort.open(QIODevice::ReadWrite)) //8,None,1
    {
        // 设置波特率
        mSerialPort.setBaudRate(baudRate);
        //设置数据位数
        mSerialPort.setDataBits(QSerialPort::Data8);
        // 设置校验位
        mSerialPort.setParity(QSerialPort::NoParity);

        // 设置流控制
        mSerialPort.setFlowControl(QSerialPort::NoFlowControl);
        //设置停止位
        mSerialPort.setStopBits(QSerialPort::OneStop);
        return true;
    }
    return false;
}

bool PdSerial::changeBaudRate(int baudRate)
{
    if (mSerialPort.isOpen()) {
        return mSerialPort.setBaudRate(baudRate);
    }
    return false;
}

void PdSerial::close()
{
    mSerialPort.close();
}

bool PdSerial::waitForReadReady(int msec)
{
    return mSerialPort.waitForReadyRead(msec);
}

QByteArray PdSerial::read(int len)
{
    QByteArray data;
    int timeout = 20; //20 * 5 = 100ms
    while (data.size() < len) {
        data.append(mSerialPort.read(len));
        timeout--;
        if(timeout<=0) { //如果timeout*5ms还没有读到len字节数据，说明len太长或者串口没有足够的数据可读
            return data;
        }
        QEventLoop loop;
        QTimer::singleShot(5, &loop, SLOT(quit()));
        loop.exec();
    }
    return data;
}

int PdSerial::write(QByteArray data)
{
    qDebug() << "write " << data << "to " << mSerialPort.portName();
    return mSerialPort.write(data);
}

void PdSerial::clear()
{
    mSerialPort.clear();
    mSerialPort.clearError();
}
