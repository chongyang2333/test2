#ifndef SERIADL_III_H
#define SERIADL_III_H

#include "QSerialPort" //串口访问
#include "QSerialPortInfo" //串口端口信息访问

class c_serail:public QObject
{
public://函数
    bool  serial_init(QString port_num,int baud_rate,int parity,int data_bits,int stop_bits,int i );//串口初始化主函数中调用
    void serial_close(QString port_num,int i);//关闭
    void serial_Read(int i);//串口数据读取
    void serial_write(QString data,int i);//串口数据发送
    int  StrHex(char * str, unsigned char  *data,int i);//字符串转为16进制
    char HexChar(char c);//单个字符转为16进制

public://参数
     QSerialPort SerialPort;
};

#endif
