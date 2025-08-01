// TimeCommand - 시간 기반 모터 명령
#ifndef TIMECOMMAND_H
#define TIMECOMMAND_H

#include "imotorcommand.h"

class TimeCommand : public IMotorCommand
{
public:
    QString buildCommand(int rpm, int duration, MotorDirection direction = MotorDirection::CW) const override;
    bool isValidInput(int rpm, int duration) const override;
    QString getValueLabel() const override;
};

#endif // TIMECOMMAND_H