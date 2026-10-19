#pragma once

#include "begin.h"
#include "SharedPointer.h"

// Forward declaration of Mix_Chunk for SDL_mixer
struct Mix_Chunk;

namespace RoninEngine::Runtime
{
    class RONIN_API AudioClip : public RoninPointer
    {
    public:
        Mix_Chunk *mix_chunk;
#ifndef NDEBUG
        int used;
#endif

        std::string name;
        std::string guid;
        uint32_t assetId = 0;

        AudioClip();
        explicit AudioClip(Mix_Chunk *chunk);
        AudioClip(const std::string &name, Mix_Chunk *chunk);
        virtual ~AudioClip();

        Mix_Chunk *getMixChunk() const;
        void setMixChunk(Mix_Chunk *chunk);

        double getDuration() const;
        int getChannels() const;
        int getFrequency() const;
        uint32_t getLengthInBytes() const;
        bool valid() const;

        static AudioClipRef Create(Mix_Chunk *chunk);
        static AudioClipRef CreateFromFile(const std::string &path);
        static AudioClipRef CreateFromStream(std::istream &stream);
    };

} // namespace RoninEngine::Runtime
