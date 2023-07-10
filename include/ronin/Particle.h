#pragma once
#include "Object.h"
#include "begin.h"
#include "Atlas.h"

namespace RoninEngine::Runtime
{

    class SHARK Particle : public Object
    {
    public:
        Particle();
        virtual ~Particle();

        bool enabled();
        void enable(bool state);

        void pause();
        void play();

        void stop();
        void seek(int);
        int length();

        void load(Atlas* atlas);
        void unload();

        void draw();
    };

}
