#pragma once

#include "dependency.h"
#include "Terrain2D.h"

namespace RoninEngine::Levels {

	void Level_Init();
    int Load_Level(std::string levelName, RoninEngine::Runtime::Terrain2D * terrain2d);
	const char* Get_LevelName(int levelId);
	void Unload_Level(int levelId);
	void Level_Free();
}
