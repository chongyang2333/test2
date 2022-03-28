#include "dtucfgthread.h"
#include "pdhttpapi.h"
#include "pdlog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QEventLoop>
dtuCfgThread::dtuCfgThread()
{

}

void dtuCfgThread::stopMainThread()
{
    this->quit();
}

//static QMetaObject::Connection connect(const QObject *sender, const QMetaMethod &signal,
//                    const QObject *receiver, const QMetaMethod &method,
//                    Qt::ConnectionType type = Qt::AutoConnection);


bool dtuCfgThread::waitUntilSerialReadReadyOrTimeout(int timeoutValue)
{
    QTimer tmpTimer;
    QEventLoop tmpEventLoop;

    connect(&myC_serial->SerialPort, &QSerialPort::readyRead, &tmpEventLoop, &QEventLoop::quit);
    connect(&tmpTimer, &QTimer::timeout, &tmpEventLoop, &QEventLoop::quit);

    tmpTimer.setSingleShot(true);
    //DelayLoopTimer->setInterval(500);
    tmpTimer.start(timeoutValue);
    tmpEventLoop.exec();
    if(tmpTimer.isActive())
    {
        tmpTimer.stop();
        return true;
    }
    else
    {
        return false;
    }
}

bool dtuCfgThread::serialSendCmdCheckReturn(QString SendATCmd,QString ReceiveATCmd)
{
    QString temp;
    //发送AT指令
    myC_serial->SerialPort.clear();
    //发送命令
    myC_serial->SerialPort.write(SendATCmd.toLocal8Bit());
    if(waitUntilSerialReadReadyOrTimeout(10*1000))
    {
        //不延时读到的数据总是缺少
        QEventLoop loop;
        QTimer::singleShot(300, &loop, SLOT(quit()));
        loop.exec();
        temp.append( myC_serial->SerialPort.readAll());
        myC_serial->SerialPort.clear();

        if(strstr(temp.toLocal8Bit(),ReceiveATCmd.toLocal8Bit()))
        {
            qDebug()<<"AT return succeed ,sendBuf = "<<SendATCmd <<"receiveBuf = "<<temp;
            return true;
        }
        else
        {
            qDebug()<<"AT return failed,sendBuf = "<<SendATCmd <<"receiveBuf = "<<temp;
            return false;
        }
    }
    else
    {
        qDebug()<<"AT return timeout";
        return false;
    }
}

void dtuCfgThread::cfgFailedHandling()
{
    int errCount = 0;   
    while(errCount <= 10)
    {
        if(!serialSendCmdCheckReturn("+++","OK"))
        {
            errCount++;
        }
        else
        {
            errCount = 0;
            break;
        }   
    }

    if(!errCount)
    {
        while(errCount <= 10)
        {
            if(!serialSendCmdCheckReturn("AT&F","OK"))
            {
                errCount++;
            }
            else
            {
                errCount = 0;
                break;
            } 
        } 
    }

    myC_serial->serial_close(Serial_ch.toLatin1(),1);
}

void dtuCfgThread::serialSendCfgAT()
{
    #if 0
    //阿里云烧录
    //DTU设备名配置成PID:AT+DEVICEID="69041C3101X0002"
    //DTU工作模式配置成阿里云: AT+DTUMODE=6,0,0,0
    //DTU配置阿里云三元组信息： AT+ALYMQTT=0,"a1jwF4WJzeM","b202a7150fd4022ff47d6adb9f96ed3f","69041C3101X0002",1
    //DTU配置阿里云服务器地址为上海：AT+ALYREGIONID="cn-shanghai",1
    //DTU配置MQTT订阅： AT+AUTOSUB=1,"/sys/a1jwF4WJzeM/69041C3101X0002/thing/model/down_raw",1,1
    //DTU配置MQTT推送： AT+AUTOPUB=1,"/sys/a1jwF4WJzeM/69041C3101X0002/thing/model/up_raw",1,1,1
    //DTU配置串口： AT+UARTCFG=38400,1,0,1
    //DTU保存刚才配置的参数
    serialSendCmdCheckReturn(QString("AT+DEVICEID=\"") + QString(AliRegisterInfo->deviceName) + QString("\""),"OK");
    serialSendCmdCheckReturn("AT+DTUMODE=6,1","OK");
    serialSendCmdCheckReturn(QString("AT+ALYMQTT=0,\"") + QString(AliRegisterInfo->productKey) + "\",\"" + QString(AliRegisterInfo->deviceSecret) + \
                         "\",\"" + QString(AliRegisterInfo->deviceName) + "\",1","OK");
    serialSendCmdCheckReturn("AT+ALYREGIONID=\"cn-shanghai\",1","OK");
    serialSendCmdCheckReturn(QString("AT+AUTOSUB=1,\"/sys/") + QString(AliRegisterInfo->productKey + "/" + QString(AliRegisterInfo->deviceName)+ \
                                                                   + "/thing/model/down_raw\",1,1") ,"OK");
    serialSendCmdCheckReturn("AT+AUTOPUB=1,\"/sys/" + AliRegisterInfo->productKey + "/" + AliRegisterInfo->deviceName + \
                         "/thing/model/up_raw\",1,1,1","OK");
    serialSendCmdCheckReturn("AT+UARTCFG=38400,1,0,2","OK");
    serialSendCmdCheckReturn("AT&W","OK");
    #else
    //TCP烧录
    //配置工作模式为TCP/UDP透传       ：AT+DTUMODE=1,1
    //配置modbus_RTU不转TCP功能      : AT+TCPMODBUS=0,1
    //配置服务器hex转换              ：AT+TCPHEX=0,1
    //配置心跳包                     ：AT+KEEPALIVE=0,0,"heart",1
    //配置备用服务器地址             ：AT+SECSERVER=0,0,0,0
    //配置注册包参数                 ：AT+DTUID=1,0,0,"2f8451c61af861a8ed54f09e86d1e882",1
    //配置TCP服务器地址              ：AT+DSCADDR=1,"TCP","dishwasher-oapi-dev.pudutech.com",9000
    //保存配置                       : AT&W

    serialSendCmdCheckReturn("AT+DTUMODE=1,1","OK");
    serialSendCmdCheckReturn("AT+TCPMODBUS=0,1","OK");
    serialSendCmdCheckReturn("AT+TCPHEX=0,1","OK");
    serialSendCmdCheckReturn("AT+KEEPALIVE=0,0,\"heart\",1","OK");
    serialSendCmdCheckReturn("AT+SECSERVER=0,0,0,0","OK");

    // dtuTcpRegisterInfo->tcpHostSecret = "bfa6ef7e62386401bed7ac7c47a02856";
    // dtuTcpRegisterInfo->pid           = "690421100180009";
    // dtuTcpRegisterInfo->tcpHostAdress = "dishwasher-oapi-dev.pudutech.com";
    // dtuTcpRegisterInfo->tcpHostPort   = "9000";
    serialSendCmdCheckReturn( QString("AT+DTUID=1,0,1,\"") + QString(dtuTcpRegisterInfo->pid).toLatin1().toHex() + QString("2E") +\
                              QString(dtuTcpRegisterInfo->tcpHostSecret).toLatin1().toHex() + QString("0A") + QString("\",1"),"OK");
    serialSendCmdCheckReturn( QString("AT+DSCADDR=1,\"TCP\",\"") + QString(dtuTcpRegisterInfo->tcpHostAdress) +\
                             QString("\",") + QString(dtuTcpRegisterInfo->tcpHostPort),"OK");
    serialSendCmdCheckReturn("AT+UARTCFG=38400,1,0,2","OK");
    serialSendCmdCheckReturn("AT&W","OK");
    #endif

}

bool dtuCfgThread::readCurDtuCfg()
{
    #if 1
    if(!serialSendCmdCheckReturn("AT+DTUMODE?\r\n",\
                                 "+DTUMODE: 1,0,0,0"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+TCPMODBUS?\r\n",\
                                 "+TCPMODBUS: 0,0,0,0"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+TCPHEX?\r\n",\
                                 "+TCPHEX: 0,0,0,0,0"))
    {
        return false;
    }

    if(!serialSendCmdCheckReturn("AT+KEEPALIVE?\r\n",\
                                 "+KEEPALIVE: 0,0,\"heart\",1"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+SECSERVER?\r\n",\
                                 "+SECSERVER: 0,0,0,0"))
    {
        return false;
    }   
    if(!serialSendCmdCheckReturn("AT+DTUID?\r\n",\
                                 QString("+DTUID: 1,0,1,\"") + QString(dtuTcpRegisterInfo->pid).toLatin1().toHex() + QString("2E") +\
                                 QString(dtuTcpRegisterInfo->tcpHostSecret).toLatin1().toHex() + QString("0A") + QString("\",1")))
    {
        return false;
    }  
     if(!serialSendCmdCheckReturn("AT+DSCADDR?\r\n",QString("+DSCADDR: 1,\"TCP\",\"") + QString(dtuTcpRegisterInfo->tcpHostAdress) \
                                   +QString("\",") + QString(dtuTcpRegisterInfo->tcpHostPort) ))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+UARTCFG?\r\n",\
                                  "+UARTCFG: 38400,1,0,2"))
    {
         return false;
    }
    return true;    
    #else
    if(!serialSendCmdCheckReturn("AT+DTUMODE?\r\n",\
                                 "+DTUMODE: 6,0,0,0"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+DEVICEID?\r\n",\
                                 "+DEVICEID: \"" + QString(AliRegisterInfo->deviceName)+"\""))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+ALYMQTT?\r\n",\
                                 QString("+ALYMQTT: 0,\"") + QString(AliRegisterInfo->productKey) + "\",\"" +\
                                 QString(AliRegisterInfo->deviceSecret) + "\",\"" + QString(AliRegisterInfo->deviceName) +"\",1"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+ALYREGIONID?\r\n",\
                                 "+ALYREGIONID: \"cn-shanghai\",1"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+AUTOSUB?\r\n",\
                                 "+AUTOSUB: 1,\"/sys/" + AliRegisterInfo->productKey + "/"+ \
                                 AliRegisterInfo->deviceName + "/thing/model/down_raw\",1,1"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+AUTOPUB?\r\n",\
                                 "+AUTOPUB: 1,\"/sys/" + AliRegisterInfo->productKey + "/" + AliRegisterInfo->deviceName + \
                                                          "/thing/model/up_raw\",1,1,1"))
    {
        return false;
    }
    if(!serialSendCmdCheckReturn("AT+UARTCFG?\r\n",\
                                 "+UARTCFG: 38400,1,0,2"))
    {
        return false;
    }
    return true;
    #endif
}

bool dtuCfgThread::getRegisterInfoFromHttps(QString pid)
{
    //根据输入的PID拿到TCP客户端信息
    dtuTcpRegisterInfo->tcpHostAdress = " ";
    dtuTcpRegisterInfo->tcpHostSecret = " ";
    dtuTcpRegisterInfo->tcpHostPort   = " ";
    PdDeviceTcpRsgisterInfo *info = PdHttpApi::getInstance(this->parent())->getTcpRegisterInfo(pid);
    if( info != nullptr && (info->tcpHostAdress != " ") \
            && (info->tcpHostSecret != " ") )
    {
        dtuTcpRegisterInfo->tcpHostAdress = info->tcpHostAdress;
        dtuTcpRegisterInfo->tcpHostSecret = info->tcpHostSecret;
        dtuTcpRegisterInfo->tcpHostPort   = info->tcpHostPort;
        dtuTcpRegisterInfo->pid           = info->pid;
        qDebug()<<"get tcpInfo succeed ";
        qDebug()<<"tcpHostAdress:"<<dtuTcpRegisterInfo->tcpHostAdress<<\
                 " tcpHostPort:"<<dtuTcpRegisterInfo->tcpHostPort<<\
                 " tcpHostSecret:"<<dtuTcpRegisterInfo->tcpHostSecret<<\
                 " pid:"<<dtuTcpRegisterInfo->pid;
        return true;
    }
    qDebug()<<"getRegisterInfoFromHttps failed";
    return false;
}
bool dtuCfgThread::searcDevFromCurSerial()
{
    int flag = 0;
    const auto port_cfg = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : port_cfg)
    {
        qDebug()<<"port ch ="<<info.portName().toLatin1();
        //先连接上串口
        if(myC_serial->serial_init(info.portName().toLatin1(),9600,0,8,1,1))
        {
            //发送AT命令
            if(serialSendCmdCheckReturn("+++","OK"))
            {
                //有返回,已经找到了设备
                //myC_serial->serial_close(info.portName().toLatin1(),1);
                Serial_ch = info.portName();
                flag = 1;
                break;
            }
            else
            {
                //没有返回
                myC_serial->serial_close(info.portName().toLatin1(),1);
            }
        }
        else
        {
          //连接串口失败
          continue;
        }
    }

    if(1 != flag)
    {
        return false;
    }

     return true;
}

bool dtuCfgThread::cfgCurDev()
{
    int errCount = 0;

    while(errCount <=3)
    {
        //通过AT配置
        serialSendCfgAT();
        //确保配置正确再读取一次
        if(readCurDtuCfg())
        {
            return true;
        }
        qDebug()<<"Cfg failed";
        errCount++;
    }
    cfgFailedHandling();
    return false;
}

bool dtuCfgThread::reStartCurDevBySerial()
{
    //发送重启AT
    int serialMsg = 0;
    QString tempBuf;
    QString lastTempBuf;

    if(!serialSendCmdCheckReturn("AT+CFUN=1,1","OK"))
    {
        QEventLoop loop;
        QTimer::singleShot(5000, &loop, SLOT(quit()));
        loop.exec();
        return false;
    }
    else
    {
        myC_serial->serial_close(myC_serial->SerialPort.portName().toLatin1(),1);
        QEventLoop loop;
        QTimer::singleShot(300, &loop, SLOT(quit()));
        loop.exec();
        qDebug()<<"Reset Cur DTU Dev and Serial reopen ,serial ch = "<< Serial_ch;
        myC_serial->serial_init(myC_serial->SerialPort.portName().toLatin1(),38400,0,8,1,1);
    }

    //等待数据返回,并判断是不是MQTT CONNECTED
    while(serialMsg <= 4)
    {
        if(waitUntilSerialReadReadyOrTimeout(25*1000))
        {
            //比较数据是不是MQTT，不是的话就略过
            QEventLoop loop;
            QTimer::singleShot(300, &loop, SLOT(quit()));
            loop.exec();
            lastTempBuf = tempBuf;
            tempBuf = ( myC_serial->SerialPort.readAll());
            qDebug()<< "cur at cmd: " << tempBuf << "last at cmd: " << lastTempBuf;
            if(tempBuf.size() >= 32)
            {
                if((strstr(tempBuf.toLocal8Bit(),"NET STATE REGISTERED")) && (strstr(tempBuf.toLocal8Bit(),"1, CONNECTED")))
                {
                    qDebug()<<"reStartDev succeed";
                    return true;
                }
                if((strstr(tempBuf.toLocal8Bit(),"NET STATE REGISTERED")) && (strstr(tempBuf.toLocal8Bit(),"1, CLOSED")))
                {
                    qDebug()<<"reStartDev failed:DisConnect";
                    return false;
                }
            }

            if((strstr(tempBuf.toLocal8Bit(),"STATUS")) && (strstr(lastTempBuf.toLocal8Bit(),"NET STATE REGISTERED")))
            {
                if(strstr(tempBuf.toLocal8Bit(),"1, CONNECTED"))
                {
                    qDebug()<<"reStartDev succeed";
                    return true;
                }
                else
                {
                    qDebug()<<"reStartDev failed:DisConnect";
                    return false;
                }
            }
            else
            {
                qDebug()<<"compare failed,serialMsg :"<<serialMsg;
                serialMsg++;
            }
        }
        else
        {
            //超时直接回复失败
            qDebug()<<"reStartDev failed:timeOut";
            return false;
        }
    }
    qDebug()<<"reStartDev falied:has not receive mqttMsg";
    return false;
}

bool dtuCfgThread::reStartCurDev()
{
    int errCount = 0;

    //最多会重启两次
    while(errCount <= 2)
    {
        //等待重启成功
        if(reStartCurDevBySerial())
        {
            
            return true;
        }
        else
        {
            errCount++;
        }
    }
    cfgFailedHandling();
    return false;
}


bool dtuCfgThread::activeCurDevByHttps()
{
    if(PdHttpApi::getInstance(this)->active(AliRegisterInfo->deviceName))
    {
        qDebug()<<"activeCurDevByHttps succeed";
        return true;
    }
    else
    {
       qDebug()<<"activeCurDevByHttps failed";
       return false;
    }

}

bool dtuCfgThread::sendSelfcheckResultToMesByHttps()
{
    if(PdHttpApi::getInstance(this)->sendSelfCheckStatusToMes(dtuTcpRegisterInfo->pid))
    {
        qDebug()<<"activeCurDevByHttps succeed";
        myC_serial->serial_close(myC_serial->SerialPort.portName().toLatin1(),1);
        return true;
    }
    else
    {
        qDebug()<<"activeCurDevByHttps failed";
        cfgFailedHandling();
        return false;
    }

}


bool dtuCfgThread::selfcheckCurDevByHttps()
{
    int tmp;
    tmp = PdHttpApi::getInstance(this)->getTcpClientSelfCheckResult(dtuTcpRegisterInfo->pid);
    // tmp = PdHttpApi::getInstance(this)->getTcpClientSelfCheckResult("");    // dtuTcpRegisterInfo->pid
    if(1 == tmp)
    {
        qDebug()<<"checkCurDevByHttps succeed";
        return true;
    }
    else if(0 == tmp)
    {
        qDebug()<<"checkCurDevByHttps failed: Selfcheck ";
        return false;
    }
    else if(-1 == tmp)
    {
        qDebug()<<"checkCurDevByHttps failed: exception err ";
        return false;
    }

    return false;
}
bool dtuCfgThread::checkCurDevByHttps()
{
    int errCount = 0;

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();
    //最多会自检三次
    while(errCount < 1)
    {
        //等待自检成功
        if(selfcheckCurDevByHttps())
        {
            return true;
        }
        else
        {
            QEventLoop loop;
            QTimer::singleShot(3000, &loop, SLOT(quit()));
            loop.exec();
            errCount++;
        }
    }
    cfgFailedHandling();
    return false;
}

void dtuCfgThread::dtuCfg()
{
    //云端获取设备PID
    if(getRegisterInfoFromHttps(dtuTcpRegisterInfo->pid))//
    {
        emit dtuCfgResult("PID验证成功\r\n开始搜寻本地设备",NoErr);
    }
    else
    {
        emit dtuCfgResult("PID验证失败",ErrGetRegisterFailed);
        return;
    }
    //搜寻当前电脑的DTU设备
    if(searcDevFromCurSerial())
    {
        emit dtuCfgResult("搜寻本地设备成功\r\n开始配置当前设备",NoErr);
    }
    else
    {
        emit dtuCfgResult("搜寻本地设备失败",ErrSearchDevFailed);
        return;
    }
    //配置当前的DTU设备
    if(cfgCurDev())
    {
        emit dtuCfgResult("配置当前设备成功\r\n开始重启当前设备",NoErr);
    }
    else
    {
        emit dtuCfgResult("配置当前设备失败",ErrCfgDevFailed);
        return;
    }
    //重启当前设备
    if(reStartCurDev())
    {
        emit dtuCfgResult("重启当前设备成功\r\n开始激活当前设备",NoErr);
    }
    else
    {
        emit dtuCfgResult("重启当前设备失败",ErrReStartDevFailed);
        return;
    }
    
    //当前设备进行自检
    if(checkCurDevByHttps())
    {
        emit dtuCfgResult("当前设备自检成功\r\n开始入库设备",NoErr);
    }
    else
    {
        emit dtuCfgResult("当前设备自检失败",ErrCheckDevFailed);
        return;
    }
    //通知云端设备质检成功
    if(sendSelfcheckResultToMesByHttps())
    {
        emit dtuCfgResult("入库成功",NoErr);
    }
    else
    {
        emit dtuCfgResult("入库失败",ErrActiveDevFailed);
        return;
    }

}

void dtuCfgThread::getPidStartCfgDtu(QString pidNum)
{
    startCfgFlag = true;
    AliRegisterInfo->deviceName = pidNum.toLocal8Bit();
    qDebug()<<"start cfgDtu,input pid ="<<AliRegisterInfo->deviceName;

}

void dtuCfgThread::run()
{

        // PdHttpApi::getInstance(this)->getTcpRegisterInfo("");
//    AliRegisterInfo = new PdDeviceRegisterInfo;
//    TcpRegisterInfo = new PdDeviceTcpRsgisterInfo;
    myC_serial = new c_serail;
//    dtuTcpRegisterInfo = new PdDeviceTcpRsgisterInfo;
//    dtuTcpRegisterInfo->pid = "720721146020015";
//     PdHttpApi::getInstance(this);

    // serialDelayLoopTimer = new QTimer();
    // startCfgFlag = false;
    qDebug()<<"dtucfgthread  = "<<QThread::currentThread();
    dtuCfg();
//    while(1)
//    {
//      if(startCfgFlag)
//      {
//          //执行DTU配置
//          dtuCfg();
//          startCfgFlag = false;
//      }
//      QThread::msleep(100);
//    }

}
