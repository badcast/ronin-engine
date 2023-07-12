#include "ronin.h"

using namespace RoninEngine::Runtime;

Behaviour::Behaviour() : Behaviour(typeid(*this).name())
{
}
Behaviour::Behaviour(const std::string &name) : Component(name)
{
}
