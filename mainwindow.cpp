#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    refreshDPI(); //根据dpi调整窗体大小
    aboutDialog = new AboutDialog();
    port = new QSerialPort;
    recvDelayTimer = new QTimer();
    sendRepeatTimer = new QTimer();
    on_btnGetPortName_clicked(); // 刚运行软件时主动获取一次可用串口号
    // 连接信号与槽
    connect(ui->About,SIGNAL(triggered()),this,SLOT(showAboutDialog()));
    connect(recvDelayTimer,SIGNAL(timeout()),this,SLOT(recvData()));
    connect(sendRepeatTimer,SIGNAL(timeout()),this,SLOT(sendDataRepeat()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
// 显示about界面
void MainWindow::showAboutDialog()
{
    aboutDialog->exec();
}
// 接收数据
void MainWindow::recvData()
{
    recvDelayTimer->stop(); //先关闭接收延时计时
    QByteArray data = port->readAll();
    if(!data.isEmpty())
    {
        recvCnt += data.size();
        ui->labelRxCnt->setText("RX: "+ QString::number(recvCnt,10));
        showData(data,dataTypeRecv);
        data.clear();
    }
}
//定时重复发送数据
void MainWindow::sendDataRepeat()
{
    if(repeatCnt>0)
    {
        repeatCnt--;
        on_btnSendData_clicked();
    }
    else
        sendRepeatTimer->stop();
}
//显示数据
void MainWindow::showData(const QByteArray &data,  DataType type)
{
    QString strHex = dataToHexFormat(data) + "\n"; // 转换成16进制格式显示 30 31 32 33
    QString str = dataToStrFormat(data) + "\n"; // 字符串格式显示 "0123"
    //根据数据类型设置颜色、方向标识和索引号
    QString strPreFix;
    int color;
    if(type==dataTypeSend)
    {
        color = 0x0000ff;
        sendIndex++;
        strPreFix = strPreFix.insert(0,"<< ");
        if(ui->chkBoxShowIndex->isChecked())
            strPreFix = strPreFix.insert(0,QString("%1").arg(sendIndex,5,10,QLatin1Char('0')));
    }
    else
    {
        color = 0xff0000;
        recvIndex++;
        strPreFix = strPreFix.insert(0,">> ");
        if(ui->chkBoxShowIndex->isChecked())
            strPreFix = strPreFix.insert(0,QString("%1").arg(recvIndex,5,10,QLatin1Char('0')));
    }
    //根据设置添加时间标识
    if(ui->chkBoxShowTime->isChecked())
    {
        QString time = "[" +QDateTime::currentDateTime().toString("hh:mm:ss:zzz") + "]";
        strPreFix  = strPreFix.insert(0,time);
    }
    strHex  = strHex.insert(0,strPreFix);
    str = str.insert(0,strPreFix);
    //滚动条拉到末尾，确保数据被插入末尾
    ui->textRecvDataHex->moveCursor(QTextCursor::End);
    ui->textRecvData->moveCursor(QTextCursor::End);
    //显示数据
    ui->textRecvDataHex->insertHtml(strToHtml(strHex,color));
    ui->textRecvData->insertHtml(strToHtml(str,color));
    //再次滚动条拉到末尾
    ui->textRecvDataHex->moveCursor(QTextCursor::End);
    ui->textRecvData->moveCursor(QTextCursor::End);
}
// 串口接收数据延时, 延时时间可根据串口波特率计算，大于2个bit间隔
void MainWindow::portRecvDataDelay()
{
    recvDelayTimer->stop();
    recvDelayTimer->start(10);
}
//将字符串转换成带颜色的html格式字符串
QString MainWindow::strToHtml(const QString &str, int color)
{
    QString str2 = str;
    //先替换特殊字符
    str2.replace("&","&amp;");
    str2.replace(">","&gt;");
    str2.replace("<","&lt;");
    str2.replace("\"","&quot;");
    str2.replace("\'","&#39;");
    str2.replace(" ","&nbsp;");
    str2.replace("\n","<br>");
    str2.replace("\r","<br>");
    return QString("<span style=\" color:#%1;\">%2</span>").arg(color,6,16,QLatin1Char('0')).arg(str2);
}
// 数据转字符串格式，控制字符自动改为"."
QString MainWindow::dataToStrFormat(const QByteArray &data)
{
    QString str = str.fromLocal8Bit(data);
    return str;
}
// 数据转16进制格式字符串，每个字符中间加" "
QString MainWindow::dataToHexFormat(const QByteArray &data)
{
    QString strHex = data.toHex().data();
    for (int cnt=2;cnt <strHex.length();) // 插入空格
    {
        strHex.insert(cnt," ");
        cnt += 3;
    }
    return strHex;
}
//刷新dpi
void MainWindow::refreshDPI()
{
    //计算dpi
    QList<QScreen*> screens = QApplication::screens();
    QScreen* screen = screens[0];
    qreal dpi = screen->logicalDotsPerInch();
    //计算dpi对应的缩放比例
    double objectRate = dpi/96.0;
    changeObjectSize(*this, objectRate);
    qDebug()<<"width "<<width() << "height "<< height();
    resize(width()*objectRate,height()*objectRate);
}
//修改所有控件尺寸
void MainWindow::changeObjectSize(const QObject &o, double objectRate)
{
    for (int i=0; i<o.children().size(); ++i) {
        QWidget *pWidget = qobject_cast<QWidget *>(o.children().at(i));
        if (pWidget != nullptr) {
            qDebug() << pWidget->width() << pWidget->height();
            //pWidget->resize(pWidget->width()*objectRate, pWidget->height()*objectRate);
            pWidget->setGeometry(pWidget->x()*objectRate,pWidget->y()*objectRate,
                                 pWidget->width()*objectRate, pWidget->height()*objectRate);
            changeObjectSize(*(o.children().at(i)),objectRate);
        }
    }
}
// 获取当前可用的串口号
void MainWindow::on_btnGetPortName_clicked()
{
    const auto infos = QSerialPortInfo::availablePorts();
    ui->comboBoxPortName->clear();
    for(const QSerialPortInfo &info : infos)
    {
        QSerialPort port;
        port.setPort(info);
        if(port.open(QIODevice::ReadWrite))
        {
            ui->comboBoxPortName->addItem(info.portName());
            qDebug()<<info.portName();
            port.close();
        }
    }
}
// 打开或关闭串口
void MainWindow::on_btnOpenPort_clicked()
{
    if(ui->btnOpenPort->text()== tr("打开串口"))
    {
        port->setPortName(ui->comboBoxPortName->currentText());
        if(port->open(QIODevice::ReadWrite))
        {
            port->setBaudRate(ui->comboBoxBaudRate->currentText().toInt());
            port->setStopBits(QSerialPort::StopBits(ui->comboBoxStopBit->currentIndex()+1));
            port->setDataBits(QSerialPort::DataBits(ui->comboBoxDataBit->currentText().toInt()));
            if(ui->comboBoxParity->currentIndex()!=1) //自定义的校验方式，其实不存在
                port->setParity(QSerialPort::Parity(ui->comboBoxParity->currentIndex()));
            else
                port->setParity(QSerialPort::NoParity);
            //更新控件
            ui->btnOpenPort->setText("关闭串口");
            ui->btnGetPortName->setEnabled(false);
            //信号连接到接收延时计时
            //connect(port,&QSerialPort::readyRead,this,&MainWindow::recvData);
            connect(port,&QSerialPort::readyRead,this,&MainWindow::portRecvDataDelay);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), port->errorString());
        }
    }
    else
    {
        port->clear();
        port->close();
        //更新控件
        ui->btnOpenPort->setText("打开串口");
        ui->btnGetPortName->setEnabled(true);
    }

}
// 发送数据
void MainWindow::on_btnSendData_clicked()
{
    QByteArray data;
    if(ui->rbtnHexMode->isChecked()) //16进制模式下发送数据
    {
        QString strHex = ui->txtSendData->text();
        data = data.fromHex(strHex.toLatin1().data());
    }
    else //文本模式
        data = ui->txtSendData->text().toLocal8Bit();
    //串口发送数据，返回发送数据长度或错误信息
    qint64 result = port->write(data);
    if(result>=0)
    {
        sendCnt += result;
        ui->labelTxCnt->setText("TX: "+ QString::number(sendCnt,10));
        qDebug()<<ui->txtSendData->text();
        if(ui->chkBoxShowSendData->isChecked())
            showData(data, dataTypeSend);
    }
    else
        QMessageBox::critical(this,tr("Error"),port->errorString());
}
// 清除显示界面
void MainWindow::on_btnClearRecvData_clicked()
{
    ui->textRecvData->clear();
    ui->textRecvDataHex->clear();
    sendIndex = 0;
    recvIndex = 0;
}
// 清除RxTx计数
void MainWindow::on_btnRxTxClear_clicked()
{
    sendCnt = 0;
    recvCnt = 0;
    ui->labelRxCnt->setText("RX: 0");
    ui->labelTxCnt->setText("TX: 0");
}
//定时重复发送数据
void MainWindow::on_chkBoxSendRepeat_stateChanged(int arg1)
{
    if(arg1>0)
    {
        repeatCnt = ui->spinBoxRepeatCnt->value();
        int interval = ui->spinBoxRepeatInterval->value();
        if(interval<50)
            interval = 50;
        sendRepeatTimer->start(interval);
    }
    else
        sendRepeatTimer->stop();
}
//保存textBower数据到文件
void MainWindow::on_btnSaveData_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,"Open File","","Text File(*.txt)");
    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream text(&file);
            text.setCodec("UTF-8");
            text<<QString("----------文本模式数据 [<<]发送 [>>]接收----------\n").toUtf8()
                <<ui->textRecvData->toPlainText().toUtf8()
                <<QString("----------16进制模式数据 [<<]发送 [>>]接收----------\n").toUtf8()
                <<ui->textRecvDataHex->toPlainText().toUtf8();
            QMessageBox::warning(this,"Save","保存成功.");
            file.close();
        }
        else
            QMessageBox::warning(this,"Save","操作失败.");
    }
}

void MainWindow::on_btnClearSendData_clicked()
{
    ui->txtSendData->clear();
}
