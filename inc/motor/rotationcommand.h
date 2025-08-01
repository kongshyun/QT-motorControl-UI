// RotationCommand - 회전수 기반 모터 명령
#ifndef ROTATIONCOMMAND_H
#define ROTATIONCOMMAND_H

#include "imotorcommand.h"

class RotationCommand : public IMotorCommand
{
public:
    QString buildCommand(int rpm, int rotations, MotorDirection direction = MotorDirection::CW) const override;
    bool isValidInput(int rpm, int rotations) const override;
    QString getValueLabel() const override;
};

#endif // ROTATIONCOMMAND_H