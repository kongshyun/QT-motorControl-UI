// MotorControl - 모터 제어 메인 클래스
#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <QString>
#include <QDebug>
#include <memory>
#include "imotorcommand.h"
#include "serialhandler.h"

class MotorControl
{
public:
    MotorControl();
    
    void setCommandStrategy(std::unique_ptr<IMotorCommand> command);
    QString buildCommand(int rpm, int value, MotorDirection direction = MotorDirection::CW) const;
    bool isValidInput(int rpm, int value) const;

    bool processResponse(const QString &message); // true == 연결 성공(READY)

    void reset();

private:
    std::unique_ptr<IMotorCommand> commandStrategy;
    bool isReady = false;
};

#endif // MOTORCONTROL_H
