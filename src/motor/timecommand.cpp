// TimeCommand - 시간 기반 모터 명령 구현
#include "timecommand.h"

QString TimeCommand::buildCommand(int rpm, int duration, MotorDirection direction) const
{
    QString dirStr = (direction == MotorDirection::CW) ? "CW" : "CCW";
    return QString("RPM:%1 TIME:%2 DIR:%3").arg(rpm).arg(duration).arg(dirStr);
}

bool TimeCommand::isValidInput(int rpm, int duration) const
{
    return (rpm > 0 && duration > 0);
}

QString TimeCommand::getValueLabel() const
{
    return "시간(초)";
}