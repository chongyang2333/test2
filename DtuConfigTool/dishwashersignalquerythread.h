#ifndef DISHWASHERSIGNALQUERYTHREAD_H
#define DISHWASHERSIGNALQUERYTHREAD_H
#include <QThread>
#include "pdhttpapi.h"

class dishwasherSignalQueryThread: public QThread
{
    Q_OBJECT    
public:
    dishwasherSignalQueryThread(QString pidPara);
    void stopDishWasherSignalQueryThread();    
private:
    QString inputPid;
    bool dishwasherSignalQueryByHttp();
protected:
    void run() Q_DECL_OVERRIDE;  //线程任务
signals:
    void dishwasherSignalQueryResult(QString signalValue,int errCode);
private slots:


};

#endif // DISHWASHERSIGNALQUERYTHREAD_H
