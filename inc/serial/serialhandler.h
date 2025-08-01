// SerialHandler - ESP32와 UART 시리얼 통신 관리
#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>

class SerialHandler : public QObject
{
    Q_OBJECT
public:
    explicit SerialHandler(QObject *parent=nullptr);
    ~SerialHandler();

    bool openSerialPort(const QString &portName, qint32 baudRate = QSerialPort::Baud115200);
    void closeSerialPort();
    void sendCommand(const QString &command);
    void sendData(const QString &data);
    bool isOpen() const;

signals:
    void dataReceived(const QString &data);  // 수신된 데이터가 있을 때 signal

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serial;
};

#endif // SERIALHANDLER_H
