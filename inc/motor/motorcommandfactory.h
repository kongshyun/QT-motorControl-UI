// MotorCommandFactory - 모터 명령 객체 생성 팩토리
#ifndef MOTORCOMMANDFACTORY_H
#define MOTORCOMMANDFACTORY_H

#include "imotorcommand.h"
#include <memory>

class MotorCommandFactory
{
public:
    static std::unique_ptr<IMotorCommand> createCommand(MotorMode mode);
};

#endif // MOTORCOMMANDFACTORY_H