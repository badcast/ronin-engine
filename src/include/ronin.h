#ifndef _RONIN_ENGINE_H_
#define _RONIN_ENGINE_H_

// C++ STL
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <ctime>
#include <limits>
#include <cassert>
#include <algorithm>
#include <functional>
#include <cstring>
#include <random>

// standard listing include
#include <initializer_list>
#include <map>
#include <memory>
#include <numeric>
#include <deque>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <tuple>

#include "ronin_native.h"

// base headers
#include "begin.h"

#include "just-parser.h"

// PROTECTIVE PRIVATE MEMBERs UNLOCK
#define private public
#define protected public

// applications
#include "Time.h"
#include "RoninSimulator.h"
#include "Resources.h"
#include "SystemInformation.h"

#include "Controls.h"
#include "Math.h"
#include "Paths.h"
#include "Texture.h"
#include "InputSystem.h"

// objects
#include "Object.h"
#include "Behaviour.h"
#include "Camera.h"
#include "Camera2D.h"
#include "Component.h"
#include "World.h"
#include "Light.h"
#include "Player.h"
#include "Renderer.h"
#include "Spotlight.h"
#include "Sprite.h"
#include "SpriteRenderer.h"
#include "Terrain2D.h"

#include "Transform.h"

#include "UI.h"
#include "Gizmos.h"
#include "NavMesh.h"
#include "Physics2D.h"
#include "Atlas.h"
#include "AudioSource.h"
#include "MusicPlayer.h"

#include "GameObject.h"

#undef private
#undef protected

#include "ronin_exception.h"
#include "basic_ronin.h"

#include "static.h"
#include "font.h"
#endif
