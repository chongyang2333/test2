#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "iqctestthread.h"
#include "signalstrengthtestthread.h"
#include "dtuCfgThread.h"
#include "dishwashersignalquerythread.h"
#include <QMainWindow>


#define SOFTVERSION  "2.0"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    /**
     * @brief SendLineEditToCfgThread
     * @param lineEditPid 输入的PID字符
     */
    void SendLineEditToCfgThread(QString lineEditPid);
    void startActiveDtu();
private slots:
    void startDtuTest();
    void dtuTestResult(QString portName1, QString portName2, int portNum, int signalStrength);
    void dtuTestStoped();
    void startSignalTest();
    void signalTestResult(int strength);
    void signalTestStopped();
    void saveCurrentTabIndex(int index);
    void setBtnState(QString);
    void setSignalQueryBtnState(QString);
    void startQueryDishwasherSignal();
    void startDtuCfg();
    void dtuCfgStoped();
    void dtusignalQueryThreadStoped();
    void dtuCfgResultShow(QString showText,int errCode);
    void dishwasherSignalResultShow(QString showText,int errCode);
    void clearTexrBrowser();
private:
    Ui::MainWindow *ui;
    IQCTestThread *mIQCTestThread;
    dtuCfgThread * mDtuCfgThread;
    dishwasherSignalQueryThread * mdishwasherSignalQueryThread;
    SignalStrengthTestThread *mSignalStrengthTestThread;
    int curDtuTypeIndex;
    QString defaultBtnStyleSheet;
    QString defaultLableStyleSheet;
    void setTextBrowser(QString str,QColor bcol,QColor fcol,int fontW);
    void setdishwasherSignalTextBrowser(QString str,QColor bcol,QColor fcol,int fontW);
    void init_ui();
};

#endif // MAINWINDOW_H
