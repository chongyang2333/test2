#ifndef ATCOMMANDSENDTHREAD_H
#define ATCOMMANDSENDTHREAD_H

#include "pdserial.h"

#include <QObject>
#include <QThread>

enum ErrorCode {
    NoError,
    Timeout,
    SerialError
};

class AtCommandSendThread : public QThread
{
    Q_OBJECT
public:
    AtCommandSendThread(PdSerial *serial, QByteArray data);
    ~AtCommandSendThread();

protected:
    void run();

signals:
    void dataReceived(QByteArray data, enum ErrorCode err);

private:
    QByteArray mData;
    PdSerial *mPdSerial;
};

#endif // ATCOMMANDSENDTHREAD_H
