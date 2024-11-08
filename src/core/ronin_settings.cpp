#include "ronin.h"

using namespace RoninEngine;

bool RoninSettings::Apply()
{
    return RoninSimulator::SetSettings(*this);
}

RoninSettings RoninSettings::GetCurrent()
{
    return RoninSimulator::GetSettings();
}
