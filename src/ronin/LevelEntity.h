#pragma once
#include "dependency.h"

namespace RoninEngine::Levels {

void Level_Init();
int Load_Level(std::string levelName, RoninEngine::Runtime::Terrain2D* terrain2d);
const char* Get_LevelName(int levelId);
void Unload_Level(int levelId);
std::string getHierarchyString(RoninEngine::Runtime::Transform* target);
void Level_Free();
}  // namespace RoninEngine::Levels
