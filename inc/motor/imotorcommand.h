// IMotorCommand - 모터 명령 인터페이스 (Strategy Pattern)
#ifndef IMOTORCOMMAND_H
#define IMOTORCOMMAND_H

#include <QString>

enum class MotorMode {
    ROTATION,
    TIME
};

enum class MotorDirection {
    CW,   // Clockwise
    CCW   // Counter-clockwise
};

class IMotorCommand
{
public:
    virtual ~IMotorCommand() = default;
    virtual QString buildCommand(int rpm, int value, MotorDirection direction = MotorDirection::CW) const = 0;
    virtual bool isValidInput(int rpm, int value) const = 0;
    virtual QString getValueLabel() const = 0;
};

#endif // IMOTORCOMMAND_H