#include "iqctestthread.h"

#include <QSerialPortInfo>
#include <QDebug>
#include "pdhttpapi.h"
#include "pdserial.h"
#include "settingini.h"
IQCTestThread::IQCTestThread()
{
   dtuTypeIndex = 0;
}

void IQCTestThread::run()
{

    int signalStrength = -1;
    int timeout = 20; //20 * (100+100+100+Signal_100)ms

    dtuTypeIndex = SettingINI::getInstance()->getDtuTypeIndex(CURRENT_DTU_TYPE_INDEX);
    if(dtuTypeIndex == 0)
    {
        qDebug()<<"dtu type error";
        signalStrength = -3;
        emit testResult(serialPort1Name, serialPort2Name, serialPortNum, signalStrength);
        return;
    }

    while(1) {
        serialPortNum = 0;
        const auto ports = QSerialPortInfo::availablePorts();
        if(ports.isEmpty())
        {
            break;
        }
        for (const QSerialPortInfo &port : ports) {
            qDebug()<<"port ch ="<<port.portName();
            //先连接上串口
            PdSerial pdSerial;
            timeout--;
            if(pdSerial.open(port.portName(), 9600))
            {
                connect(&pdSerial.mSerialPort, &QSerialPort::readyRead, this, &IQCTestThread::readReady);
                QByteArray res = atCommandSend(&pdSerial, "+++");
                if (res.isEmpty()) {
                    qDebug() << port.portName() << " is not DTU serial port";
                    pdSerial.close();
                    continue;
                }
                qDebug() << "received: " << res;
                signalStrength = getSignalStrength(&pdSerial);
                if (QString(res).contains("OK")) {
                    if (serialPortNum == 0) {
                        serialPortNum++;
                        serialPort1Name = port.portName();
                    }else {
                        serialPortNum++;
                        serialPort2Name = port.portName();
                    }
                }
                //res = atCommandSend(&pdSerial, "ATO");
                //qDebug() << "received: " << res;
                pdSerial.close();
//                QThread::msleep(1000);
            }
        }

        if (serialPortNum >= 1) {
            qDebug() << "get ports number = " << serialPortNum;
            break;
        }

        if (timeout <= 0) {
            qDebug() << "timeout now";
            break;
        }
        QThread::msleep(100);
    }
    emit testResult(serialPort1Name, serialPort2Name, serialPortNum, signalStrength);
}


void IQCTestThread::readReady()
{
    qDebug() << "readReady";
}

int IQCTestThread::getSignalStrength(PdSerial *serial)
{
    QByteArray data = atCommandSend(serial, "AT+CSQ");
    qDebug() << "data: " << data;
    if (data.contains("OK")) {//+CSQ: 31,99\r\nOK
        QList<QByteArray> tmp = data.split(':');
        if (tmp.size() < 2) {
            qWarning() << "data format is invalid";
            emit signalStrength(-1);
            return -1;
        }
        qDebug() << "tmp[1] = " << tmp[1];
        QList<QByteArray> tmp2 = tmp[1].split(',');
        qDebug() << "SignalStrength = " << tmp2[0] << "(" << tmp2[0].toInt() << ")";
        emit signalStrength(tmp2[0].toInt());
        return tmp2[0].toInt();
    }

    emit signalStrength(-1);
    return -1;
}

QByteArray IQCTestThread::atCommandSend(PdSerial *serial, QByteArray data)
{
    //serial->clear();
    int size = serial->write(data);
//    qDebug() << "write " << size << " bytes";
    QByteArray ret;
    if (size <= 0) {
        qDebug() << "serial write error";
        return ret;
    }
    if (serial->waitForReadReady(100)) {
        ret = serial->read(32);
        qDebug() << "receive data = " << ret;
    }
    else
    {
        qDebug() << "timeout and has not receive data ";
    }

    return ret;
}
