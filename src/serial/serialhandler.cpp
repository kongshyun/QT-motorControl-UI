// SerialHandler - ESP32와 UART 시리얼 통신 관리 구현
#include "serialhandler.h"
#include <QDebug>

SerialHandler::SerialHandler(QObject *parent)
    : QObject(parent)
{
    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead, this, &SerialHandler::handleReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &SerialHandler::handleError);

}

SerialHandler::~SerialHandler()
{
    if (serial->isOpen())
        serial->close();
}

bool SerialHandler::openSerialPort(const QString &portName, qint32 baudRate)
{
    if (serial->isOpen()) {
        serial->close();  // 기존 포트를 먼저 닫음
    }
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Serial opened successfully.";
        return true;
    } else {
        qDebug() << "Failed to open serial port:" << serial->errorString();
        return false;
    }
}

void SerialHandler::closeSerialPort()
{
    if (serial->isOpen()) {
        serial->close();
        qDebug() << "Serial port closed.";
    }
}

void SerialHandler::sendCommand(const QString &command)
{
    if (serial->isOpen()) {
        QString cmd = command;
        if (!cmd.endsWith('\n')) {
            cmd += '\n';
        }
        serial->write(cmd.toUtf8());
        serial->flush(); // 즉시 전송 보장
    }
}
void SerialHandler::sendData(const QString &data)
{
    if (serial && serial->isOpen()) {
        serial->write(data.toUtf8());
        qDebug() << "Sent to ESP32:" << data;
    } else {
        qDebug() << "Serial port not open!";
    }
}

void SerialHandler::handleReadyRead()
{
    QByteArray data = serial->readAll();
    QString message = QString::fromUtf8(data).trimmed();
    emit dataReceived(message);
}

void SerialHandler::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Serial port error: Disconnected or unavailable";
        serial->close();
        emit dataReceived("ESP32 DISCONNECTED");
    }
}

bool SerialHandler::isOpen() const
{
    return serial->isOpen();
}
