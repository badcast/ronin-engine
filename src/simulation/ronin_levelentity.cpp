#include "ronin.h"

#include <filesystem>

#include "just-parser.h"

const char extMAP[] = ".map";
const char extNAV[] = ".nav";

struct Level_t {
    std::string levelName;
    std::string levelDescription;
    std::uint8_t levelMisson;
    std::string levelMissonText;

    struct {
        float navScalePerNeuron;
    } level_terrain2D;
};

std::vector<Level_t> m_levels;

using namespace RoninEngine;

std::string RoninEngine::Levels::getHierarchyString(Runtime::Transform* target)
{
    static char delim = 0x32;
    std::string delims;
    std::string result;

    std::list<Runtime::Transform*> stack;

    while (target) {
        for (auto& c : target->hierarchy) {
            stack.emplace_back(c);
        }

        result += delims;
        result += target->name();
        result += "\n";

        if (!stack.empty()) {
            target = stack.front();
            stack.pop_front();
        } else
            target = nullptr;
    }

    return result;
}

void RoninEngine::Levels::Level_Init()
{
    if (!m_levels.empty())
        throw std::bad_exception();

    return;
    jno::jno_object_parser parser;
    Level_t levl;
    jno::jno_object_node* node;
    std::filesystem::directory_iterator dirIter(getDataFrom(Runtime::FolderKind::LEVELS));

    for (auto file : dirIter) {
        if (!file.is_directory() && file.path().extension() == extMAP) {
            // load map
            parser.deserialize(file.path().generic_string().c_str());
            node = parser.find_node("map/level_name");
            if (node)
                levl.levelName = node->toString();
            else
                levl.levelName = file.path().filename().generic_string();

            node = parser.find_node("map/level_description");
            if (node)
                levl.levelDescription = node->toString();

            node = parser.find_node("map/level_mission");
            if (node)
                levl.levelMisson = static_cast<decltype(levl.levelMisson)>(node->toNumber());
            else
                levl.levelMisson = static_cast<decltype(levl.levelMisson)>(m_levels.size() + 1);

            node = parser.find_node("map/level_mission_text");
            if (node)
                levl.levelMissonText = node->toString();

            node = parser.find_node("map/level_terrain2D/navScalePerNeuron");
            if (node)
                levl.level_terrain2D.navScalePerNeuron = static_cast<float>(node->toReal());

            m_levels.emplace_back(levl);
            levl = {};
        }
    }
}
