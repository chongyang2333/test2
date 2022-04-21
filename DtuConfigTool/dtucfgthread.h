#ifndef DTUCFGTHREAD_H
#define DTUCFGTHREAD_H
#include <QThread>
#include <QObject>
#include <QTimer>
#include "deviceinfo.h"
#include "serial_iii.h"
#include "pdhttpapi.h"
class dtuCfgThread : public QThread
{
    Q_OBJECT
public:
    dtuCfgThread();
    enum ErrID
    {
        NoErr = 0,
        ErrUnknownDtuType,
        ErrGetRegisterFailed,
        ErrSearchDevFailed,
        ErrCfgDevFailed,
        ErrReStartDevFailed,
        ErrActiveDevFailed,
        ErrCheckDevFailed,
        ErrQueryDevSignalFailed,
    };
    void initPid(QString pid)
    {
        dtuTcpRegisterInfo = new PdDeviceTcpRsgisterInfo();
        dtuTcpRegisterInfo->pid = pid.toLocal8Bit();
//        startCfgFlag = true;
    }
    void stopMainThread();
private:
    void dtuCfg();
    bool getRegisterInfoFromHttps(QString pid);
    bool searcDevFromCurSerial();
    bool cfgCurDev();
    bool reStartCurDev();
    bool checkCurDevByHttps(QString *devVersion);
    bool serialSendCmdCheckReturn(QString SendATCmd,QString ReceiveATCmd);
    void serialSendCfgAT();
    bool reStartCurDevBySerial();
    bool readCurDtuCfg();
    bool waitUntilSerialReadReadyOrTimeout(int timeoutValue);
    bool selfcheckCurDevByHttps(QString *devVersion);
    void cfgFailedHandling();
    bool sendSelfcheckResultToMesByHttps();
    bool startCfgFlag = false;
    bool startActiveFlag = false;
    c_serail  *myC_serial;
    QTimer    *serialDelayLoopTimer;
    QString    Serial_ch;
    int curDtuTypeIndex = 0;
//    PdHttpApi  *PdhttpApiAdress = PdHttpApi::getInstance(this);
    QByteArray PidNumLineInput;
    PdDeviceRegisterInfo *AliRegisterInfo;
    PdDeviceTcpRsgisterInfo *TcpRegisterInfo;
    PdDeviceTcpRsgisterInfo *dtuTcpRegisterInfo;
protected:
    void run() Q_DECL_OVERRIDE;  //线程任务
signals:
    //RegisterInfo_From_Https(int result);
    void dtuCfgResult(QString showText,int errCode);

private slots:
    void getPidStartCfgDtu(QString pidNum);
    void couldStartActivDev();
};

#endif // DTUCFGTHREAD_H
