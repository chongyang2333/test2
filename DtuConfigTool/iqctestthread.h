#ifndef IQCTESTTHREAD_H
#define IQCTESTTHREAD_H

#include "pdserial.h"

#include <QObject>
#include <QThread>

class IQCTestThread : public QThread
{
    Q_OBJECT
public:
    IQCTestThread();
    ~IQCTestThread(){}

protected:
    void run();

signals:
    /**
     * @brief testResult 质检结果返回，先判断portNum是否为2，如果是，则正常，否则异常
     * @param portName1
     * @param portName2
     * @param portNum
     */
    void testResult(QString portName1, QString portName2, int portNum, int strength);

    /**
     * @brief signalStrength
     * @param strength 信号强度，取值范围是0~31，一般16以上才算正常
     */
    void signalStrength(int strength);

public slots:
    void readReady();

private:
    int getSignalStrength(PdSerial *serial);
    QByteArray atCommandSend(PdSerial *serial, QByteArray data);

private:
    int serialPortNum;
    int dtuTypeIndex;
    QString serialPort1Name;
    QString serialPort2Name;
};

#endif // IQCTESTTHREAD_H
