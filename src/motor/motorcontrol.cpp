// MotorControl - 모터 제어 메인 클래스 구현
#include "motorcontrol.h"
#include "rotationcommand.h"

MotorControl::MotorControl()
    : commandStrategy(std::make_unique<RotationCommand>())
{
}

void MotorControl::setCommandStrategy(std::unique_ptr<IMotorCommand> command)
{
    commandStrategy = std::move(command);
}

QString MotorControl::buildCommand(int rpm, int value, MotorDirection direction) const
{
    if (!commandStrategy) {
        return QString();
    }
    return commandStrategy->buildCommand(rpm, value, direction);
}

bool MotorControl::isValidInput(int rpm, int value) const
{
    if (!commandStrategy) {
        return false;
    }
    return commandStrategy->isValidInput(rpm, value);
}

bool MotorControl::processResponse(const QString &message)
{
    if (message == "READY") {
        qDebug()<<"수신 : READY";
        isReady = true;
        return true;
    }

    return false;
}

void MotorControl::reset()
{
    isReady = false;
}
