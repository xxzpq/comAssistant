#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QScreen>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "aboutdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum DataType {
        dataTypeSend = 0,
        dataTypeRecv = 1
    };
private slots:
    void showAboutDialog();
    void recvData();
    void sendDataRepeat();
    void showData(const QByteArray &data, DataType type);
    void portRecvDataDelay();
    QString strToHtml(const QString &str, int color);
    QString dataToStrFormat(const QByteArray &data);
    QString dataToHexFormat(const QByteArray &data);
    void refreshDPI();
    void changeObjectSize(const QObject &o, double objectRate);
    void on_btnGetPortName_clicked();
    void on_btnOpenPort_clicked();
    void on_btnSendData_clicked();
    void on_btnClearRecvData_clicked();
    void on_btnRxTxClear_clicked();
    void on_chkBoxSendRepeat_stateChanged(int arg1);
    void on_btnSaveData_clicked();
    void on_btnClearSendData_clicked();
private:
    Ui::MainWindow *ui;
    AboutDialog *aboutDialog;
    QSerialPort *port;
    QTimer *recvDelayTimer;
    QTimer *sendRepeatTimer;
    qint64 recvCnt = 0;
    qint64 sendCnt = 0;
    quint16 repeatCnt = 0;
    quint16 recvIndex = 0;
    quint16 sendIndex = 0;
};
#endif // MAINWINDOW_H
