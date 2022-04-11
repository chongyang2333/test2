#include "signalstrengthtestthread.h"
#include "pdserial.h"
#include <QSerialPortInfo>
#include <QByteArray>
#include <QDebug>
#include "settingini.h"

SignalStrengthTestThread::SignalStrengthTestThread()
{
    interval = 200; //默认200ms读取一次信号强度
    stopFlag = false;
    mSerial = nullptr;
    dtuTypeIndex = 0;
}

SignalStrengthTestThread::~SignalStrengthTestThread()
{

}

void SignalStrengthTestThread::run()
{
    dtuTypeIndex = SettingINI::getInstance()->getDtuTypeIndex(CURRENT_DTU_TYPE_INDEX);
    if(dtuTypeIndex == 0)
    {
        qDebug()<<"dtu type error";
        emit signalStrength(-3);
        return;
    }

    if (!findValidSerialPort()) {
        emit signalStrength(-2);
        return;
    }

    int strength = -1;
    while (!stopFlag) {
        strength = getSignalStrength(mSerial);
        emit signalStrength(strength);
        QThread::msleep(interval);
    }
    mSerial->close();
    delete mSerial;
    qDebug()<<"SignalStrengthTestThread finished";
}

bool SignalStrengthTestThread::findValidSerialPort()
{
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        qDebug()<<"port ch ="<<port.portName();
        //先连接上串口
        PdSerial *pdSerial = new PdSerial;
        if(pdSerial->open(port.portName(), 9600))
        {
            QByteArray res = atCommandSend(pdSerial, "+++");
            if (res.isEmpty()) {
                qDebug() << port.portName() << " is not DTU serial port";
                pdSerial->close();
                delete pdSerial;
                continue;
            }
            qDebug() << "received: " << res;
            if (QString(res).contains("OK")) {
                mSerial = pdSerial;
                return true;
            }else {
                pdSerial->close();
                delete pdSerial;
            }
        }
    }

    return false;
}

int SignalStrengthTestThread::getSignalStrength(PdSerial *serial)
{
    QByteArray data = atCommandSend(serial, "AT+CSQ");
    qDebug() << "data: " << data;
    if (data.contains("OK")) {//+CSQ: 31,99\r\nOK
        QList<QByteArray> tmp = data.split(':');
        if (tmp.size() < 2) {
            qWarning() << "data format is invalid";
            //emit signalStrength(-1);
            return -1;
        }
        qDebug() << "tmp[1] = " << tmp[1];
        QList<QByteArray> tmp2 = tmp[1].split(',');
        qDebug() << "SignalStrength = " << tmp2[0] << "(" << tmp2[0].toInt() << ")";
        //emit signalStrength(tmp2[0].toInt());
        return tmp2[0].toInt();
    }

    //emit signalStrength(-1);
    return -1;
}

void SignalStrengthTestThread::stop()
{
    stopFlag = true;
}


QByteArray SignalStrengthTestThread::atCommandSend(PdSerial *serial, QByteArray data)
{
    //serial->clear();
    int size = serial->write(data);
    qDebug() << "write " << size << " bytes";
    QByteArray ret;
    if (size <= 0) {
        qDebug() << "serial write error";
        return ret;
    }
    if (serial->waitForReadReady(200)) {
        ret = serial->read(32);
    }

    return ret;
}

