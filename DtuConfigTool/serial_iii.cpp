#include "serial_iii.h"


bool c_serail::serial_init(QString port_num, int baud_rate, int parity, int data_bits, int stop_bits, int i)
{
	// 设置串口号
	
	SerialPort.setPortName(port_num);
	// 打开串口
	if (SerialPort.open(QIODevice::ReadWrite))
	{
		// 设置波特率
		SerialPort.setBaudRate(baud_rate);
		//设置数据位数
		switch (data_bits)
		{
		case 5: SerialPort.setDataBits(QSerialPort::Data5); break;
		case 6: SerialPort.setDataBits(QSerialPort::Data6); break;
		case 7: SerialPort.setDataBits(QSerialPort::Data7); break;
		case 8: SerialPort.setDataBits(QSerialPort::Data8); break;
		default: break;
		}
		// 设置校验位
		//SerialPort->setParity(QSerialPort::NoParity);
		//设置奇偶校验
		switch (parity)
		{
		case 0: SerialPort.setParity(QSerialPort::NoParity); break;
		case 1: SerialPort.setParity(QSerialPort::OddParity); break;
		case 2: SerialPort.setParity(QSerialPort::EvenParity); break;
		default: break;
		}
		// 设置流控制
		SerialPort.setFlowControl(QSerialPort::NoFlowControl);
		//设置停止位
		switch (stop_bits)
		{
		case 1: SerialPort.setStopBits(QSerialPort::OneStop); break;
		case 2: SerialPort.setStopBits(QSerialPort::TwoStop); break;
		default: break;
		}
	}
	//打开串口
	else
	{
        return false;
	}
    return true;

}
//关闭串口
void c_serail::serial_close(QString port_num, int i)
{
	SerialPort.setPortName(port_num);
	SerialPort.close();
}

//串口数据发送
void  c_serail::serial_write(QString data, int i)
{
	SerialPort.write(data.toStdString().data());

}

char c_serail::HexChar(char c)

{

	if ((c >= '0') && (c <= '9'))

		return c - '0';//16进制中的，字符0-9转化成10进制，还是0-9

	else if ((c >= 'A') && (c <= 'F'))

		return c - 'A' + 10;//16进制中的A-F，分别对应着11-16

	else if ((c >= 'a') && (c <= 'f'))

		return c - 'a' + 10;//16进制中的a-f，分别对应也是11-16，不区分大小写

	else

		return 0x10;   // 其他返回0x10

}


int c_serail::StrHex(char * str, unsigned char  *data, int i)

{

	int t, t1;

	int rlen = 0, len = i;

	if (len == 1)

	{

		char h = str[0];

		t = HexChar(h);

		data[0] = (unsigned char)t;

		rlen++;

	}

	for (int i = 0; i < len;)

	{

		char l, h = str[i];

		if (h == ' ')

		{

			i++;

			continue;

		}

		i++;

		if (i >= len)

			break;

		l = str[i];

		t = HexChar(h);

		t1 = HexChar(l);

		if ((t == 16) || (t1 == 16))

			break;

		else

			t = t * 16 + t1;

		i++;

		data[rlen] = (unsigned char)t;

		rlen++;

	}

	return rlen;

}
