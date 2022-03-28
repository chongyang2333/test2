#include "atcommandsendthread.h"

#include <QDebug>

AtCommandSendThread::AtCommandSendThread(PdSerial *serial, QByteArray data)
{
    mPdSerial = serial;
    mData = data;
    start();
}

AtCommandSendThread::~AtCommandSendThread()
{
    deleteLater();
}

void AtCommandSendThread::run()
{
    int size = mPdSerial->write(mData);
    QByteArray data;
    if (size < 0) {
        qDebug() << "serial write error";
        emit dataReceived(data, SerialError);
        return;
    }
    if (mPdSerial->waitForReadReady(2 * 1000)) {
        data = mPdSerial->read(128);
        emit dataReceived(data, NoError);
    }else {
        emit dataReceived(data, Timeout);//timeout
    }
}
