#include "dishwashersignalquerythread.h"
#include "pdhttpapi.h"
#include  "dtucfgthread.h"
dishwasherSignalQueryThread::dishwasherSignalQueryThread(QString pidPara)
{
   inputPid =  pidPara;
}

void  dishwasherSignalQueryThread::stopDishWasherSignalQueryThread()
{
    this->quit();
}


bool dishwasherSignalQueryThread::dishwasherSignalQueryByHttp()
{
    int tmp = 0;
    tmp = PdHttpApi::getInstance(this)->getdishwasherSignal(inputPid);
    if( 0 < tmp)
    {
        dishwasherSignalQueryResult("pid: " + inputPid + "\r\n查询信号值成功,信号值为：" + QString::number(tmp,10),dtuCfgThread::NoErr);
        return true;
    }
    else
    {
        dishwasherSignalQueryResult("pid: " + inputPid + ",查询信号值失败",dtuCfgThread::ErrQueryDevSignalFailed);
        return false;
    }    

}

void dishwasherSignalQueryThread::run()
{
    
    if(dishwasherSignalQueryByHttp())
    {
        qDebug()<<"dishwasherSignalQueryThread:get signal succeed";
        emit dishwasherSignalQueryResult("",dtuCfgThread::NoErr);
    }
    else
    {
        qDebug()<<"dishwasherSignalQueryThread:get signal failed";
        emit dishwasherSignalQueryResult("",dtuCfgThread::ErrCheckDevFailed);
    }
    inputPid.clear();

}
