#pragma once

#include "begin.h"

namespace RoninEngine::Levels {

extern void Level_Init();
extern int Load_Level(std::string levelName, RoninEngine::Runtime::Terrain2D* terrain2d);
extern const char* Get_LevelName(int levelId);
extern void Unload_Level(int levelId);
extern std::string getHierarchyString(RoninEngine::Runtime::Transform* target);
extern void Level_Free();
}  // namespace RoninEngine::Levels
