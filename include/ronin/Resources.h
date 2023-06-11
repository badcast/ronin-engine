#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {

        enum { GCInvalidID = 0xffffffff };

        class RONIN_API Resources
        {
            friend class RoninEngine::Application;

        public:
            static int load_resource(const std::string& path);

            static Sprite* get_sprite(int resource, bool local = true);

            static SDL_Surface* get_surface(int resource, bool local = true);

            static AudioClip* get_audioclip(int resource, bool local = true);

            static AudioClip* load_clip(const std::string& path);
        };
    } // namespace Runtime
} // namespace RoninEngine
