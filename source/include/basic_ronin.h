/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

static std::unordered_map<std::type_index, std::string> main_ronin_types;

#define DESCRIBE_TYPE(type) ( main_ronin_types[std::type_index(typeid(type))]=#type )
