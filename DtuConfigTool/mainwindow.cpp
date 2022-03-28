#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "settingini.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include "pdhttpapi.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();
    //查看支持的版本
    qDebug()<<"OpenSSL version="<<QSslSocket::sslLibraryBuildVersionString();
    init_ui();
    defaultLableStyleSheet = ui->lb_result1->styleSheet();
    defaultBtnStyleSheet = ui->bt_start1->styleSheet();

    ui->tabWidget->setCurrentIndex(SettingINI::getInstance()->get(CURRENT_TAB_INDEX));

    connect(ui->bt_start1, SIGNAL(clicked()), this, SLOT(startDtuTest()));
    connect(ui->bt_start2, SIGNAL(clicked()), this, SLOT(startSignalTest()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(saveCurrentTabIndex(int)));
}



MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setTextBrowser(QString str,QColor bcol,QColor fcol,int fontW)
{
    if(str.isEmpty()||str.isNull())
    {
        return;
    }
    ui->textBrowser->setTextBackgroundColor(bcol);
    ui->textBrowser->setTextColor(fcol);
    ui->textBrowser->setFontPointSize(fontW);
    ui->textBrowser->append (str);
    ui->textBrowser->setTextBackgroundColor(QColor(255,255,255));
    ui->textBrowser->setTextColor(QColor(0,0,0));
//    this->setFixedSize(750,550);
}
void MainWindow::init_ui()
{
    QFont font;
    font.setFamily(QString::fromUtf8("\345\256\213\344\275\223")); //宋体
    font.setPointSize(18);
    font.setBold(true);

    ui->label->setText("PID编号：");
    ui->label->setStyleSheet("font-size:25px;color:rgb(10,10,10)");
    ui->lineEdit->setStyleSheet("QLineEdit{font-size:25px;background-color: rgb(255,255,255);color:rgb(55,100,255);height:50px;\
                                border:4px;solid:rgb(155,200,33);selection-color:pink}");//
    ui->pushButton->setText("确认");
    ui->pushButton->setStyleSheet(("background-color: rgb(22, 155, 219);font-size:25px"));// 70, 212, 255 color:#f5f5f5
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setText("清屏");
    ui->pushButton_2->setStyleSheet(("background-color: rgb(255,0,0);font-size:25px;color:#f5f5f5"));// 70, 212, 255

    ui->textBrowser->setStyleSheet(("background-color: rgb(255,255,255);font-size:25px;color:#f5f5f5"));

    setTextBrowser(QString("Building Time:") + QString(__DATE__) + QString("--") + \
                   QString(__TIME__), QColor(255, 255, 255), QColor(0, 255, 0), 20);

    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(setBtnState(QString)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(startDtuCfg()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(clearTexrBrowser()));
//     qDebug()<<"ui init = "<<QThread::currentThread();
//     mDtuCfgThread = new dtuCfgThread;
//     connect(mDtuCfgThread, QThread::finished, mDtuCfgThread, QThread::deleteLater);
//     connect(mDtuCfgThread, QThread::finished, this, dtuCfgStoped);
//     connect(mDtuCfgThread, dtuCfgThread::dtuCfgResult, this, dtuCfgResultShow);
//     connect(this,SIGNAL(SendLineEditToCfgThread(QString)),mDtuCfgThread,SLOT(getPidStartCfgDtu(QString)));

//     mDtuCfgThread->start();


}

void MainWindow::clearTexrBrowser()
{
    ui->textBrowser->clear();
}
void MainWindow::setBtnState(QString)
{
    if(!ui->lineEdit->text().isEmpty())
    {
        ui->pushButton->setEnabled(true);
    }
    else
    {
        ui->pushButton->setEnabled(false);
    }
}
void MainWindow::dtuCfgStoped()
{
    qDebug()<<"dtuCfgThread finished";
    delete mDtuCfgThread;
}
void MainWindow::startDtuCfg()
{
    //
    mDtuCfgThread = new dtuCfgThread();
    mDtuCfgThread->initPid(ui->lineEdit->text().toLocal8Bit());
    connect(mDtuCfgThread, QThread::finished, mDtuCfgThread, QThread::deleteLater);
    connect(mDtuCfgThread, QThread::finished, this, dtuCfgStoped);
    connect(mDtuCfgThread, dtuCfgThread::dtuCfgResult, this, dtuCfgResultShow);
//     connect(this,SIGNAL(SendLineEditToCfgThread(QString)),mDtuCfgThread,SLOT(getPidStartCfgDtu(QString)));

    ui->textBrowser->clear();
    setTextBrowser("PID验证中", QColor(255, 255, 255), QColor(0, 0, 0), 20);
    ui->pushButton->setEnabled(false); 
    qDebug()<<"btn clicked pid = "<<QThread::currentThread();
    mDtuCfgThread->start();

    //emit SendLineEditToCfgThread(ui->lineEdit->text());

}
void MainWindow::dtuCfgResultShow(QString showText,int errCode)
{

    if(dtuCfgThread::NoErr == errCode)
    {
        setTextBrowser(showText, QColor(255, 255, 255), QColor(0, 0, 0), 20);
        if(showText == "入库成功")
        {
//            QMessageBox msgBox;
//            msgBox.setText("DTU 配置成功.");
//            msgBox.exec();
            QMessageBox message(QMessageBox::Warning,"DtuConfigTool","<font size='35' color='green'>设备烧录成功</font>");
            message.exec();
            ui->textBrowser->clear();
            ui->lineEdit->clear();
            ui->pushButton->setEnabled(false);
        }
    }
    else
    {
        setTextBrowser(showText, QColor(255, 255, 255), QColor(255, 40, 37), 20);
//        QMessageBox msgBox;
//        msgBox.setText("DTU 配置失败.");
//        msgBox.exec();
        QMessageBox message(QMessageBox::Warning,"DtuConfigTool","<font size='35' color='red'>设备烧录失败</font>");
        message.exec();
        ui->textBrowser->clear();
        ui->lineEdit->clear();
        ui->pushButton->setEnabled(false);
    }
}

void MainWindow::startDtuTest()
{
    mIQCTestThread = new IQCTestThread;
    connect(mIQCTestThread, QThread::finished, mIQCTestThread, QThread::deleteLater);
    connect(mIQCTestThread, QThread::finished, this, dtuTestStoped);
    connect(mIQCTestThread, IQCTestThread::testResult, this, dtuTestResult);

    ui->bt_start1->setEnabled(false);
    ui->bt_start2->setEnabled(false);
    ui->lb_result1->clear();
    ui->lb_result1->setStyleSheet(defaultLableStyleSheet);
    ui->lb_result2->clear();
    ui->lb_result2->setStyleSheet(defaultLableStyleSheet);
    ui->textBrowser1->clear();
    ui->textBrowser2->clear();

    ui->textBrowser1->setText("正在查找设备，请稍候，串口波特率为9600bps");

    mIQCTestThread->start();
}

void MainWindow::dtuTestResult(QString portName1, QString portName2, int portNum, int signalStrength)
{
    bool uartResult = false;
    bool signalResult = false;
    qDebug() << "testResult:" << portNum << " port1:" << portName1 << " port2:" << portName2;
    qDebug() << "testResult: signalStrength = " << signalStrength;
    if (portNum < 2) {
        ui->textBrowser1->setText("查找设备失败，请确认是否已经把两根USB转串口线都连接电脑");
    }else if(portNum > 2){
        ui->textBrowser1->setText("请确认电脑是否接入了多个DTU");
    }else {
        ui->textBrowser1->setText("设备连接测试成功");
        uartResult = true;
    }

    if (signalStrength < 0 || signalStrength == 99) {
        ui->textBrowser1->append("网络信号强度获取失败, 请检查是否插入了电话卡");
    } else if (signalStrength < 16) {
        ui->textBrowser1->append(QString("网络信号强度为") + QString::number(signalStrength) + "db, 不合格！！！");
    } else {
        ui->textBrowser1->append(QString("网络信号强度为") + QString::number(signalStrength) + "db, 合格");
        signalResult = true;
    }

    if (uartResult && signalResult) {
        ui->lb_result1->setText("测试通过");
        ui->lb_result1->setStyleSheet("background:green;font: 22px \"新宋体\";");
    }else {
        ui->lb_result1->setText("测试失败");
        ui->lb_result1->setStyleSheet("background:red;font: 22px \"新宋体\";");
    }
}

void MainWindow::startSignalTest()
{
    ui->lb_result1->clear();
    ui->lb_result1->setStyleSheet(defaultLableStyleSheet);
    ui->lb_result2->clear();
    ui->lb_result2->setStyleSheet(defaultLableStyleSheet);
    ui->textBrowser1->clear();
    ui->textBrowser2->clear();

    //ui->bt_start2->setEnabled(false);
    if (ui->bt_start2->text() == "开始测试") {
        ui->bt_start1->setEnabled(false);
        ui->bt_start2->setText("停止测试");
        ui->bt_start2->setStyleSheet("background:yellow;font: 22px \"新宋体\";");
        mSignalStrengthTestThread = new SignalStrengthTestThread;
        mSignalStrengthTestThread->start();
        connect(mSignalStrengthTestThread, QThread::finished, mSignalStrengthTestThread, QThread::deleteLater);
        connect(mSignalStrengthTestThread, QThread::finished, this, signalTestStopped);
        connect(mSignalStrengthTestThread, SignalStrengthTestThread::signalStrength, this, signalTestResult);
    } else {
        mSignalStrengthTestThread->stop();
    }
}

void MainWindow::signalTestResult(int strength)
{
    qDebug() << "signalTestResult: " << strength;
    if (strength < 0) {
        ui->textBrowser2->setText("检测串口失败，请确认串口是否接入电脑，DTU是否恢复出厂设置");
        ui->lb_result2->setText("测试失败");
        ui->lb_result2->setStyleSheet("background:red;font: 22px \"新宋体\";");
    } else if (strength == 99) {
        ui->textBrowser2->setText("网络信号强度获取失败, 请检查是否插入了电话卡");
        ui->lb_result2->setText("测试失败");
        ui->lb_result2->setStyleSheet("background:red;font: 22px \"新宋体\";");
    } else if (strength < 16) {
        ui->textBrowser2->setText(QString("网络信号强度为") + QString::number(strength) + "db, 不合格！！！");
        ui->lb_result2->setText("测试失败");
        ui->lb_result2->setStyleSheet("background:red;font: 22px \"新宋体\";");
    } else {
        ui->textBrowser2->setText(QString("网络信号强度为") + QString::number(strength) + "db, 合格");
        ui->lb_result2->setText("测试通过");
        ui->lb_result2->setStyleSheet("background:green;font: 22px \"新宋体\";");
    }
}

void MainWindow::saveCurrentTabIndex(int index)
{
    SettingINI::getInstance()->save(CURRENT_TAB_INDEX, index);
}

void MainWindow::dtuTestStoped()
{
    ui->bt_start1->setEnabled(true);
    ui->bt_start2->setEnabled(true);
    delete mIQCTestThread;
}

void MainWindow::signalTestStopped()
{
    ui->bt_start2->setText("开始测试");
    ui->bt_start2->setStyleSheet(defaultBtnStyleSheet);
    ui->bt_start1->setEnabled(true);
    delete mSignalStrengthTestThread;
}
