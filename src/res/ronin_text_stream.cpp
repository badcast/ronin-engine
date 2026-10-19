#include "ronin.h"
#include <sstream>
#include <filesystem>

namespace RoninEngine::Runtime
{
    TextStream::TextStream()
        : m_mode(std::ios::in)
        , m_isOpen(false)
    {
    }

    TextStream::TextStream(const std::string &filePath, std::ios::openmode mode)
        : m_mode(mode)
        , m_isOpen(false)
    {
        Open(filePath, mode);
    }

    TextStream::~TextStream()
    {
        Close();
    }

    bool TextStream::Open(const std::string &filePath, std::ios::openmode mode)
    {
        Close();
        m_filePath = filePath;
        m_mode = mode;
        m_stream.open(filePath, mode);
        m_isOpen = m_stream.is_open();
        return m_isOpen;
    }

    void TextStream::Close()
    {
        if (m_stream.is_open())
        {
            m_stream.close();
        }
        m_isOpen = false;
    }

    bool TextStream::IsOpen() const
    {
        return m_isOpen && m_stream.is_open();
    }

    bool TextStream::IsEOF() const
    {
        if (!IsOpen())
            return true;
        return m_stream.eof();
    }

    std::string TextStream::ReadLine()
    {
        if (!IsOpen())
            return "";
        std::string line;
        if (std::getline(m_stream, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            return line;
        }
        return "";
    }

    std::string TextStream::ReadAll()
    {
        if (!IsOpen())
            return "";
        std::stringstream ss;
        ss << m_stream.rdbuf();
        return ss.str();
    }

    std::vector<std::string> TextStream::ReadAllLines()
    {
        std::vector<std::string> lines;
        if (!IsOpen())
            return lines;

        std::string line;
        while (std::getline(m_stream, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            lines.push_back(line);
        }
        return lines;
    }

    bool TextStream::Write(const std::string &text)
    {
        if (!IsOpen())
            return false;
        m_stream << text;
        return !m_stream.fail();
    }

    bool TextStream::WriteLine(const std::string &line)
    {
        if (!IsOpen())
            return false;
        m_stream << line << '\n';
        return !m_stream.fail();
    }

    bool TextStream::Flush()
    {
        if (!IsOpen())
            return false;
        m_stream.flush();
        return !m_stream.fail();
    }

    std::streampos TextStream::GetPosition()
    {
        if (!IsOpen())
            return -1;
        if (m_mode & std::ios::in)
            return m_stream.tellg();
        return m_stream.tellp();
    }

    bool TextStream::SetPosition(std::streampos pos)
    {
        if (!IsOpen())
            return false;
        if (m_mode & std::ios::in)
            m_stream.seekg(pos);
        if (m_mode & std::ios::out)
            m_stream.seekp(pos);
        return !m_stream.fail();
    }

    std::string TextStream::ReadAllText(const std::string &filePath)
    {
        std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
        if (!ifs.is_open())
            return "";
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }

    bool TextStream::WriteAllText(const std::string &filePath, const std::string &content)
    {
        std::filesystem::path p(filePath);
        std::error_code ec;
        if (p.has_parent_path())
        {
            std::filesystem::create_directories(p.parent_path(), ec);
        }
        std::ofstream ofs(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!ofs.is_open())
            return false;
        ofs << content;
        return ofs.good();
    }

    bool TextStream::AppendText(const std::string &filePath, const std::string &content)
    {
        std::filesystem::path p(filePath);
        std::error_code ec;
        if (p.has_parent_path())
        {
            std::filesystem::create_directories(p.parent_path(), ec);
        }
        std::ofstream ofs(filePath, std::ios::out | std::ios::app | std::ios::binary);
        if (!ofs.is_open())
            return false;
        ofs << content;
        return ofs.good();
    }

    std::vector<std::string> TextStream::ReadLines(const std::string &filePath)
    {
        std::vector<std::string> lines;
        std::ifstream ifs(filePath);
        if (!ifs.is_open())
            return lines;
        std::string line;
        while (std::getline(ifs, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            lines.push_back(line);
        }
        return lines;
    }

} // namespace RoninEngine::Runtime
