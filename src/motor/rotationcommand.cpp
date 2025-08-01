// RotationCommand - 회전수 기반 모터 명령 구현
#include "rotationcommand.h"

QString RotationCommand::buildCommand(int rpm, int rotations, MotorDirection direction) const
{
    QString dirStr = (direction == MotorDirection::CW) ? "CW" : "CCW";
    return QString("RPM:%1 ROT:%2 DIR:%3").arg(rpm).arg(rotations).arg(dirStr);
}

bool RotationCommand::isValidInput(int rpm, int rotations) const
{
    return (rpm > 0 && rotations > 0);
}

QString RotationCommand::getValueLabel() const
{
    return "회전수";
}