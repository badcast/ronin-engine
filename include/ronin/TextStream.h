#pragma once

#include "begin.h"
#include <string>
#include <vector>
#include <fstream>

namespace RoninEngine::Runtime
{
    enum class TextEncoding
    {
        UTF8,
        ASCII
    };

    class RONIN_API TextStream
    {
    private:
        std::fstream m_stream;
        std::string m_filePath;
        std::ios::openmode m_mode;
        bool m_isOpen;

    public:
        TextStream();
        explicit TextStream(const std::string &filePath, std::ios::openmode mode = std::ios::in);
        ~TextStream();

        bool Open(const std::string &filePath, std::ios::openmode mode = std::ios::in);
        void Close();
        bool IsOpen() const;
        bool IsEOF() const;

        // Reading
        std::string ReadLine();
        std::string ReadAll();
        std::vector<std::string> ReadAllLines();

        // Writing
        bool Write(const std::string &text);
        bool WriteLine(const std::string &line);
        bool Flush();

        // Navigation
        std::streampos GetPosition();
        bool SetPosition(std::streampos pos);

        const std::string &GetFilePath() const { return m_filePath; }

        // Static utilities
        static std::string ReadAllText(const std::string &filePath);
        static bool WriteAllText(const std::string &filePath, const std::string &content);
        static bool AppendText(const std::string &filePath, const std::string &content);
        static std::vector<std::string> ReadLines(const std::string &filePath);
    };

} // namespace RoninEngine::Runtime
