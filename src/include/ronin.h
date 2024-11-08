#ifndef _RONIN_ENGINE_H_
#define _RONIN_ENGINE_H_

#include "ronin_std.h"

#include "ronin_sdl2.h"

// base headers
#include "begin.h"

// Set the protect to unlock
#define private public
#define protected public

#include "SharedPointer.h"
#include "Time.h"
#include "Controls.h"
#include "Math.h"
#include "Paths.h"
#include "InputSystem.h"
#include "RoninCursor.h"

// applications
#include "RoninSimulator.h"
#include "SystemInformation.h"
#include "Resources.h"

// objects
#include "Component.h"
#include "Transform.h"
#include "Behaviour.h"
#include "Camera.h"
#include "Camera2D.h"
#include "World.h"
#include "Light.h"
#include "Object.h"
#include "MoveController2D.h"
#include "Renderer.h"
#include "Spotlight.h"
#include "Sprite.h"
#include "SpriteRenderer.h"
#include "Terrain2D.h"
#include "AudioSource.h"
#include "MusicPlayer.h"
#include "UI.h"
#include "ParticleSystem.h"
#include "Atlas.h"
#include "Collision.h"
#include "GameObject.h"

#include "NavMesh.h"
#include "RenderUtility.h"
#include "Physics2D.h"

#include "end.h"

// Set the protect is normal
#undef private
#undef protected

#include "ronin_exception.h"
#include "basic_ronin.h"

#include "static.h"
#include "font.h"

#endif
