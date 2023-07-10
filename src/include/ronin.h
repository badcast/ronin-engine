#ifndef RONINENGINE_H_
#define RONINENGINE_H_

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

// standard listing include
#include <initializer_list>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <tuple>

#include "ronin_native.h"

// base headers
#include "../../include/ronin/begin.h"

#include "just-parser.h"


// PROTECTIVE PRIVATE MEMBERs UNLOCK
#define private public
#define protected public

// applications
#include "../../include/ronin/Time.h"
#include "../../include/ronin/RoninSimulator.h"
#include "../../include/ronin/Resources.h"
#include "../../include/ronin/SystemInformation.h"

#include "../../include/ronin/Controls.h"
#include "../../include/ronin/Math.h"
#include "../../include/ronin/Paths.h"
#include "../../include/ronin/Texture.h"
#include "../../include/ronin/InputSystem.h"

// objects
#include "../../include/ronin/Object.h"
#include "../../include/ronin/Behaviour.h"
#include "../../include/ronin/Camera.h"
#include "../../include/ronin/Camera2D.h"
#include "../../include/ronin/Component.h"
#include "../../include/ronin/World.h"
#include "../../include/ronin/Light.h"
#include "../../include/ronin/Player.h"
#include "../../include/ronin/Renderer.h"
#include "../../include/ronin/Spotlight.h"
#include "../../include/ronin/Sprite.h"
#include "../../include/ronin/SpriteRenderer.h"
#include "../../include/ronin/Terrain2D.h"

#include "../../include/ronin/Transform.h"

#include "../../include/ronin/UI.h"
#include "../../include/ronin/Gizmos.h"
#include "../../include/ronin/NavMesh.h"
#include "../../include/ronin/Physics2D.h"
#include "../../include/ronin/Atlas.h"
#include "../../include/ronin/AudioSource.h"
#include "../../include/ronin/MusicPlayer.h"

#include "../../include/ronin/GameObject.h"

#undef private
#undef protected

#include "ronin_exception.h"
#include "basic_ronin.h"

#include "static.h"
#include "font.h"
#endif
