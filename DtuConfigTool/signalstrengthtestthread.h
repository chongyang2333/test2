#ifndef SIGNALSTRENGTHTESTTHREAD_H
#define SIGNALSTRENGTHTESTTHREAD_H

#include "pdserial.h"

#include <QObject>
#include <QThread>
#include <QByteArray>

class SignalStrengthTestThread : public QThread
{
    Q_OBJECT
public:
    SignalStrengthTestThread();
    ~SignalStrengthTestThread();
    void stop();

signals:
    /**
     * @brief signalStrength
     * @param strength 信号强度，取值范围是0~31，一般16以上才算正常
     */
    void signalStrength(int strength);

protected:
    void run();

private:
    int getSignalStrength(PdSerial *serial);
    bool findValidSerialPort();
    QByteArray atCommandSend(PdSerial *serial, QByteArray data);

private:
    int interval;
    int dtuTypeIndex = 0;
    volatile bool stopFlag;
    PdSerial *mSerial;
};

#endif // SIGNALSTRENGTHTESTTHREAD_H
