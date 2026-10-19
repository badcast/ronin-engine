#pragma once

#include "begin.h"
#include "SharedPointer.h"
#include <string>
#include <vector>
#include <cstdint>
#include <iosfwd>

namespace RoninEngine::Runtime
{
    struct AudioClipData;

    class RONIN_API AudioClip : public RoninPointer
    {
    private:
        AudioClipData *m_data;

    public:
#ifndef NDEBUG
        int used;
#endif

        std::string name;
        std::string guid;
        uint32_t assetId = 0;

        AudioClip();
        explicit AudioClip(const std::string &name);
        virtual ~AudioClip();

        AudioClipData *getData() const;
        void initDecodedData(const void *pcmFrames, uint64_t frameCount, uint32_t channels, uint32_t sampleRate, int format);

        // Backward compatibility stubs
        void *getMixChunk() const { return nullptr; }
        void setMixChunk(void *) {}

        double getDuration() const;
        int getChannels() const;
        int getFrequency() const;
        uint32_t getLengthInBytes() const;
        uint64_t getTotalFrames() const;
        bool valid() const;

        static AudioClipRef Create(void *chunk = nullptr);
        static AudioClipRef CreateFromFile(const std::string &path);
        static AudioClipRef CreateFromStream(std::istream &stream);
        static AudioClipRef CreateFromMemory(const void *data, size_t sizeInBytes);
    };

} // namespace RoninEngine::Runtime
