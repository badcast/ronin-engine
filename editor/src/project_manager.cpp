#include "project_manager.h"
#include "asset_database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <ronin/RoninSimulator.h>

namespace RoninEditor
{
    using json = nlohmann::json;
    static std::string generateRandomGuid()
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        uint64_t part1 = dis(gen);
        uint64_t part2 = dis(gen);

        std::ostringstream oss;
        oss << std::hex << std::setfill('0')
            << std::setw(8) << (part1 >> 32) << "-"
            << std::setw(4) << ((part1 >> 16) & 0xFFFF) << "-"
            << std::setw(4) << ((part1 & 0x0FFF) | 0x4000) << "-"
            << std::setw(4) << (((part2 >> 48) & 0x3FFF) | 0x8000) << "-"
            << std::setw(12) << (part2 & 0xFFFFFFFFFFFFULL);
        return oss.str();
    }

    static std::string getCurrentIsoTime()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }

    static bool writeTextFile(const std::filesystem::path &filePath, const std::string &content, std::string &outError)
    {
        std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
        if (!ofs.is_open())
        {
            outError = "Failed to create file: " + filePath.string();
            return false;
        }
        ofs << content;
        ofs.close();
        return true;
    }

    static bool readTextFile(const std::filesystem::path &filePath, std::string &outContent, std::string &outError)
    {
        std::ifstream ifs(filePath);
        if (!ifs.is_open())
        {
            outError = "Failed to open file for reading: " + filePath.string();
            return false;
        }
        std::stringstream ss;
        ss << ifs.rdbuf();
        outContent = ss.str();
        return true;
    }

    ProjectManager &ProjectManager::GetInstance()
    {
        static ProjectManager instance;
        return instance;
    }

    ProjectManager::ProjectManager()
    {
        loadRecentProjects();
    }

    std::filesystem::path ProjectManager::GetDefaultScenePath() const
    {
        if (m_currentConfig.rootPath.empty())
            return {};
        std::filesystem::path primary = m_currentConfig.rootPath / m_currentConfig.defaultSceneRelativePath;
        if (std::filesystem::exists(primary))
            return primary;
        std::filesystem::path altWorld = m_currentConfig.rootPath / "Assets" / "Worlds" / "SampleWorld.world";
        if (std::filesystem::exists(altWorld))
            return altWorld;
        return primary;
    }

    std::filesystem::path ProjectManager::GetDefaultWorldPath() const
    {
        return GetDefaultScenePath();
    }

    bool ProjectManager::CreateProject(const std::string &parentDir, const std::string &projectName,
                                       ProjectTemplate tmpl, std::string &outError)
    {
        if (parentDir.empty())
        {
            outError = "Parent directory path cannot be empty.";
            return false;
        }

        if (projectName.empty())
        {
            outError = "Project name cannot be empty.";
            return false;
        }

        // Validate illegal path characters in project name
        const std::string invalidChars = "\\/:*?\"<>|";
        for (char c : invalidChars)
        {
            if (projectName.find(c) != std::string::npos)
            {
                outError = std::string("Project name contains illegal character: '") + c + "'";
                return false;
            }
        }

        std::filesystem::path parentPath(parentDir);
        std::error_code ec;
        if (!std::filesystem::exists(parentPath, ec))
        {
            if (!std::filesystem::create_directories(parentPath, ec))
            {
                outError = "Cannot create or access parent directory: " + parentDir;
                return false;
            }
        }

        std::filesystem::path projectRoot = parentPath / projectName;
        if (std::filesystem::exists(projectRoot, ec))
        {
            if (!std::filesystem::is_empty(projectRoot, ec))
            {
                outError = "Target directory already exists and is not empty: " + projectRoot.string();
                return false;
            }
        }
        else
        {
            if (!std::filesystem::create_directories(projectRoot, ec))
            {
                outError = "Failed to create project root directory: " + projectRoot.string();
                return false;
            }
        }

        ProjectConfig cfg;
        cfg.projectName = projectName;
        cfg.projectVersion = "0.1.0";
        cfg.engineVersion = RoninEngine::RoninSimulator::GetVersionString();
        cfg.projectGuid = generateRandomGuid();
        cfg.companyName = "DefaultCompany";
        cfg.defaultSceneRelativePath = "Assets/Worlds/SampleWorld.world";
        cfg.rootPath = std::filesystem::canonical(projectRoot, ec);
        if (ec)
            cfg.rootPath = projectRoot;
        cfg.updateDerivedPaths();

        if (!GenerateProjectScaffolding(cfg, tmpl, outError))
        {
            return false;
        }

        m_currentConfig = cfg;
        m_hasOpenProject = true;
        AssetDatabase::GetInstance().Initialize(cfg.rootPath);
        AddRecentProject(cfg.projectName, cfg.rootPath.string());
        return true;
    }

    bool ProjectManager::LoadProject(const std::string &path, std::string &outError)
    {
        if (path.empty())
        {
            outError = "Path to project cannot be empty.";
            return false;
        }

        std::error_code ec;
        std::filesystem::path p(path);
        if (!std::filesystem::exists(p, ec))
        {
            outError = "Project path does not exist: " + path;
            return false;
        }

        std::filesystem::path rootDir = p;
        if (std::filesystem::is_regular_file(p, ec))
        {
            rootDir = p.parent_path();
        }

        std::filesystem::path projectRoninPath = rootDir / "project.ronin";
        if (!std::filesystem::exists(projectRoninPath, ec))
        {
            outError = "Not a valid RoninEngine project (missing project.ronin): " + rootDir.string();
            return false;
        }

        std::string rawJson;
        if (!readTextFile(projectRoninPath, rawJson, outError))
        {
            return false;
        }

        json projJson = json::parse(rawJson, nullptr, false);
        if (projJson.is_discarded() || !projJson.is_object())
        {
            outError = "Failed to parse project.ronin JSON: " + projectRoninPath.string();
            return false;
        }

        ProjectConfig cfg;
        cfg.rootPath = std::filesystem::canonical(rootDir, ec);
        if (ec)
            cfg.rootPath = rootDir;
        cfg.updateDerivedPaths();

        if (projJson.contains("projectName") && projJson["projectName"].is_string())
            cfg.projectName = projJson["projectName"].get<std::string>();
        else
            cfg.projectName = rootDir.filename().string();

        if (projJson.contains("projectVersion") && projJson["projectVersion"].is_string())
            cfg.projectVersion = projJson["projectVersion"].get<std::string>();

        if (projJson.contains("engineVersion") && projJson["engineVersion"].is_string())
            cfg.engineVersion = projJson["engineVersion"].get<std::string>();

        if (projJson.contains("projectGuid") && projJson["projectGuid"].is_string())
            cfg.projectGuid = projJson["projectGuid"].get<std::string>();

        if (projJson.contains("companyName") && projJson["companyName"].is_string())
            cfg.companyName = projJson["companyName"].get<std::string>();

        if (projJson.contains("defaultScene") && projJson["defaultScene"].is_string())
            cfg.defaultSceneRelativePath = projJson["defaultScene"].get<std::string>();

        // Also check ProjectSettings/ProjectSettings.json if exists
        std::filesystem::path projSettingsPath = cfg.projectSettingsPath / "ProjectSettings.json";
        if (std::filesystem::exists(projSettingsPath, ec))
        {
            std::string sJson;
            if (readTextFile(projSettingsPath, sJson, outError))
            {
                json sObj = json::parse(sJson, nullptr, false);
                if (!sObj.is_discarded() && sObj.is_object())
                {
                    if (sObj.contains("defaultScene") && sObj["defaultScene"].is_string())
                        cfg.defaultSceneRelativePath = sObj["defaultScene"].get<std::string>();
                    if (sObj.contains("companyName") && sObj["companyName"].is_string())
                        cfg.companyName = sObj["companyName"].get<std::string>();
                }
            }
        }

        // Ensure required subdirectories exist (e.g. Library)
        std::filesystem::create_directories(cfg.libraryPath / "Cache", ec);
        std::filesystem::create_directories(cfg.assetsPath, ec);
        std::filesystem::create_directories(cfg.scriptsPath, ec);
        std::filesystem::create_directories(cfg.projectSettingsPath, ec);

        m_currentConfig = cfg;
        m_hasOpenProject = true;
        AssetDatabase::GetInstance().Initialize(cfg.rootPath);
        AddRecentProject(cfg.projectName, cfg.rootPath.string());
        return true;
    }

    bool ProjectManager::SaveProject(std::string &outError)
    {
        if (!m_hasOpenProject)
        {
            outError = "No active project to save.";
            return false;
        }
        return GenerateProjectFile(m_currentConfig, outError);
    }

    void ProjectManager::CloseProject()
    {
        m_hasOpenProject = false;
        m_currentConfig = ProjectConfig{};
    }

    bool ProjectManager::GenerateProjectScaffolding(const ProjectConfig &cfg, ProjectTemplate tmpl, std::string &outError)
    {
        std::error_code ec;

        // 1. Create directory hierarchy
        std::vector<std::filesystem::path> dirs = {
            cfg.assetsPath,
            cfg.assetsPath / "Worlds",
            cfg.assetsPath / "Textures",
            cfg.assetsPath / "Audio",
            cfg.assetsPath / "Fonts",
            cfg.assetsPath / "Prefabs",
            cfg.scriptsPath,
            cfg.projectSettingsPath,
            cfg.libraryPath,
            cfg.libraryPath / "Cache"
        };

        for (const auto &d : dirs)
        {
            if (!std::filesystem::create_directories(d, ec) && ec)
            {
                outError = "Failed to create directory: " + d.string();
                return false;
            }
        }

        // 2. Generate project.ronin
        if (!GenerateProjectFile(cfg, outError))
            return false;

        // 3. Generate ProjectSettings/ProjectSettings.json
        if (!GenerateProjectSettings(cfg.projectSettingsPath, cfg.projectName, outError))
            return false;

        // 4. Generate ProjectSettings/InputSettings.json
        if (!GenerateInputSettings(cfg.projectSettingsPath, outError))
            return false;

        // 5. Generate ProjectSettings/TimeSettings.json
        if (!GenerateTimeSettings(cfg.projectSettingsPath, outError))
            return false;

        // 6. Generate Scripts/CMakeLists.txt & Sample Component
        if (!GenerateCMakeLists(cfg.scriptsPath, cfg.projectName, outError))
            return false;
        if (!GenerateSampleScript(cfg.scriptsPath, outError))
            return false;

        // 7. Generate default world (Assets/Worlds/SampleWorld.world)
        std::filesystem::path worldPath = cfg.assetsPath / "Worlds" / "SampleWorld.world";
        if (!GenerateDefaultWorld(worldPath, tmpl, outError))
            return false;

        return true;
    }

    bool ProjectManager::GenerateProjectFile(const ProjectConfig &cfg, std::string &outError)
    {
        json obj = json::object();
        obj["formatVersion"] = 1;
        obj["projectName"] = cfg.projectName;
        obj["projectVersion"] = cfg.projectVersion;
        obj["engineVersion"] = cfg.engineVersion;
        obj["projectGuid"] = cfg.projectGuid;
        obj["companyName"] = cfg.companyName;
        obj["defaultScene"] = cfg.defaultSceneRelativePath;
        obj["lastModified"] = getCurrentIsoTime();

        return writeTextFile(cfg.projectFilePath, obj.dump(2), outError);
    }

    bool ProjectManager::GenerateProjectSettings(const std::filesystem::path &settingsDir,
                                                const std::string &projectName, std::string &outError)
    {
        json obj = json::object();
        obj["projectName"] = projectName;
        obj["companyName"] = "DefaultCompany";
        obj["productName"] = projectName;
        obj["version"] = "0.1.0";
        obj["defaultScene"] = "Assets/Worlds/SampleWorld.world";

        json res = json::object();
        res["defaultWidth"] = 1280;
        res["defaultHeight"] = 720;
        res["fullscreen"] = false;
        res["resizable"] = true;
        res["vsync"] = true;
        obj["resolution"] = res;

        json render = json::object();
        render["backend"] = "GPU";
        render["colorSpace"] = "Linear";
        render["pixelSnap"] = false;
        obj["rendering"] = render;

        return writeTextFile(settingsDir / "ProjectSettings.json", obj.dump(2), outError);
    }

    bool ProjectManager::GenerateInputSettings(const std::filesystem::path &settingsDir, std::string &outError)
    {
        json obj = json::object();
        json axes = json::array();

        auto addAxis = [&](const std::string &name, const std::string &pos, const std::string &neg,
                           const std::string &altPos = "", const std::string &altNeg = "",
                           float gravity = 3.0f, float dead = 0.001f, float sensitivity = 3.0f) {
            json ax = json::object();
            ax["name"] = name;
            ax["positiveButton"] = pos;
            ax["negativeButton"] = neg;
            if (!altPos.empty()) ax["altPositiveButton"] = altPos;
            if (!altNeg.empty()) ax["altNegativeButton"] = altNeg;
            ax["gravity"] = gravity;
            ax["dead"] = dead;
            ax["sensitivity"] = sensitivity;
            axes.push_back(ax);
        };

        addAxis("Horizontal", "right", "left", "d", "a");
        addAxis("Vertical", "up", "down", "w", "s");
        addAxis("Fire1", "left ctrl", "", "mouse 0", "");
        addAxis("Jump", "space", "");
        addAxis("Submit", "return", "");
        addAxis("Cancel", "escape", "");

        obj["axes"] = axes;
        return writeTextFile(settingsDir / "InputSettings.json", obj.dump(2), outError);
    }

    bool ProjectManager::GenerateTimeSettings(const std::filesystem::path &settingsDir, std::string &outError)
    {
        json obj = json::object();
        obj["fixedTimestep"] = 0.02;
        obj["maximumDeltaTime"] = 0.3333333;
        obj["timeScale"] = 1.0;
        obj["captureFramerate"] = 0;

        return writeTextFile(settingsDir / "TimeSettings.json", obj.dump(2), outError);
    }

    bool ProjectManager::GenerateCMakeLists(const std::filesystem::path &scriptsDir,
                                           const std::string &projectName, std::string &outError)
    {
        std::ostringstream ss;
        ss << "cmake_minimum_required(VERSION 3.16)\n"
           << "project(" << projectName << "Scripts CXX)\n\n"
           << "set(CMAKE_CXX_STANDARD 17)\n"
           << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n"
           << "# User gameplay scripts module\n"
           << "file(GLOB_RECURSE SCRIPT_SOURCES CONFIGURE_DEPENDS \"*.cpp\" \"*.h\")\n"
           << "add_library(" << projectName << "Scripts SHARED ${SCRIPT_SOURCES})\n\n"
           << "find_package(RoninEngine QUIET)\n"
           << "if(TARGET RoninEngine)\n"
           << "    target_link_libraries(" << projectName << "Scripts PRIVATE RoninEngine)\n"
           << "endif()\n";

        return writeTextFile(scriptsDir / "CMakeLists.txt", ss.str(), outError);
    }

    bool ProjectManager::GenerateSampleScript(const std::filesystem::path &scriptsDir, std::string &outError)
    {
        std::string header =
            "#pragma once\n\n"
            "#include <ronin/framework.h>\n\n"
            "class SamplePlayerController : public RoninEngine::Runtime::Behaviour\n"
            "{\n"
            "public:\n"
            "    float moveSpeed = 5.0f;\n\n"
            "    void OnAwake() override;\n"
            "    void OnStart() override;\n"
            "    void OnUpdate() override;\n"
            "};\n";

        std::string source =
            "#include \"SamplePlayerController.h\"\n\n"
            "void SamplePlayerController::OnAwake()\n"
            "{\n"
            "}\n\n"
            "void SamplePlayerController::OnStart()\n"
            "{\n"
            "}\n\n"
            "void SamplePlayerController::OnUpdate()\n"
            "{\n"
            "    using namespace RoninEngine::Runtime;\n"
            "    Vec2 move = Vec2::zero;\n"
            "    if (Input::GetKey(KeyCode::A) || Input::GetKey(KeyCode::LeftArrow))\n"
            "        move.x -= 1.0f;\n"
            "    if (Input::GetKey(KeyCode::D) || Input::GetKey(KeyCode::RightArrow))\n"
            "        move.x += 1.0f;\n"
            "    if (Input::GetKey(KeyCode::W) || Input::GetKey(KeyCode::UpArrow))\n"
            "        move.y += 1.0f;\n"
            "    if (Input::GetKey(KeyCode::S) || Input::GetKey(KeyCode::DownArrow))\n"
            "        move.y -= 1.0f;\n\n"
            "    if (move != Vec2::zero)\n"
            "    {\n"
            "        transform()->Translate(move.Normalized() * moveSpeed * Time::deltaTime());\n"
            "    }\n"
            "}\n";

        if (!writeTextFile(scriptsDir / "SamplePlayerController.h", header, outError))
            return false;
        return writeTextFile(scriptsDir / "SamplePlayerController.cpp", source, outError);
    }

    bool ProjectManager::GenerateDefaultScene(const std::filesystem::path &scenePath, ProjectTemplate tmpl, std::string &outError)
    {
        json scene = json::object();
        std::string name = scenePath.stem().string();
        scene["worldName"] = name;
        scene["sceneName"] = name;
        scene["formatVersion"] = 1;
        json objects = json::array();

        // 1. Main Camera
        {
            json cam = json::object();
            cam["id"] = 1;
            cam["name"] = "Main Camera";
            cam["tag"] = "MainCamera";
            cam["layer"] = 0;
            cam["zOrder"] = 0;
            cam["active"] = true;

            json tr = json::object();
            json pos = json::array();
            pos.push_back(0.0);
            pos.push_back(0.0);
            tr["position"] = pos;
            tr["angle"] = 0.0;
            tr["parentId"] = -1;
            cam["transform"] = tr;

            json comps = json::array();
            json cCam = json::object();
            cCam["type"] = "Camera2D";
            cCam["zoom"] = 1.0;
            cCam["mainCamera"] = true;
            comps.push_back(cCam);
            cam["components"] = comps;

            objects.push_back(cam);
        }

        // 2. Directional / Ambient Light
        {
            json light = json::object();
            light["id"] = 2;
            light["name"] = "World Light";
            light["tag"] = "Untagged";
            light["layer"] = 0;
            light["zOrder"] = 10;
            light["active"] = true;

            json tr = json::object();
            json pos = json::array();
            pos.push_back(0.0);
            pos.push_back(2.0);
            tr["position"] = pos;
            tr["angle"] = 0.0;
            tr["parentId"] = -1;
            light["transform"] = tr;

            json comps = json::array();
            json cLight = json::object();
            cLight["type"] = "Light";
            cLight["range"] = 8.0;
            json color = json::array();
            color.push_back(255);
            color.push_back(255);
            color.push_back(255);
            color.push_back(255);
            cLight["color"] = color;
            comps.push_back(cLight);
            light["components"] = comps;

            objects.push_back(light);
        }

        if (tmpl == ProjectTemplate::Action2D)
        {
            // 3. Player GameObject with SpriteRenderer & Script
            json player = json::object();
            player["id"] = 3;
            player["name"] = "Player Samurai";
            player["tag"] = "Player";
            player["layer"] = 0;
            player["zOrder"] = 10;
            player["active"] = true;

            json tr = json::object();
            json pos = json::array();
            pos.push_back(0.0);
            pos.push_back(-0.5);
            tr["position"] = pos;
            tr["angle"] = 0.0;
            tr["parentId"] = -1;
            player["transform"] = tr;

            json comps = json::array();
            json cSprite = json::object();
            cSprite["type"] = "SpriteRenderer";
            cSprite["primitiveShape"] = "Rectangle";
            json sSize = json::array();
            sSize.push_back(1.0);
            sSize.push_back(1.5);
            cSprite["size"] = sSize;
            json sColor = json::array();
            sColor.push_back(60);
            sColor.push_back(140);
            sColor.push_back(240);
            sColor.push_back(255);
            cSprite["color"] = sColor;
            comps.push_back(cSprite);

            json cCtrl = json::object();
            cCtrl["type"] = "SamplePlayerController";
            cCtrl["moveSpeed"] = 5.0;
            comps.push_back(cCtrl);

            player["components"] = comps;
            objects.push_back(player);

            // 4. Ground Terrain Surface
            json ground = json::object();
            ground["id"] = 4;
            ground["name"] = "Ground Surface";
            ground["tag"] = "Environment";
            ground["layer"] = 0;
            ground["zOrder"] = -20;
            ground["active"] = true;

            json gTr = json::object();
            json gPos = json::array();
            gPos.push_back(0.0);
            gPos.push_back(-3.0);
            gTr["position"] = gPos;
            gTr["angle"] = 0.0;
            gTr["parentId"] = -1;
            ground["transform"] = gTr;

            json gComps = json::array();
            json cTer = json::object();
            cTer["type"] = "Terrain2D";
            cTer["width"] = 16;
            cTer["length"] = 4;
            cTer["cellScale"] = 1.0;
            gComps.push_back(cTer);
            ground["components"] = gComps;

            objects.push_back(ground);
        }

        scene["objects"] = objects;
        return writeTextFile(scenePath, scene.dump(2), outError);
    }

    bool ProjectManager::GenerateDefaultWorld(const std::filesystem::path &worldPath, ProjectTemplate tmpl, std::string &outError)
    {
        return GenerateDefaultScene(worldPath, tmpl, outError);
    }

    std::filesystem::path ProjectManager::getRecentProjectsFilePath() const
    {
        std::filesystem::path configDir;
        const char *xdgConfig = std::getenv("XDG_CONFIG_HOME");
        if (xdgConfig && *xdgConfig)
        {
            configDir = std::filesystem::path(xdgConfig) / "ronin_editor";
        }
        else
        {
            const char *home = std::getenv("HOME");
            if (home && *home)
                configDir = std::filesystem::path(home) / ".config" / "ronin_editor";
            else
                configDir = std::filesystem::current_path() / ".config";
        }

        std::error_code ec;
        std::filesystem::create_directories(configDir, ec);
        return configDir / "recent_projects.json";
    }

    void ProjectManager::loadRecentProjects()
    {
        m_recentProjects.clear();
        m_recentLoaded = true;

        std::filesystem::path recentFile = getRecentProjectsFilePath();
        std::error_code ec;
        if (!std::filesystem::exists(recentFile, ec))
            return;

        std::string rawJson, err;
        if (!readTextFile(recentFile, rawJson, err))
            return;

        json root = json::parse(rawJson, nullptr, false);
        if (root.is_discarded() || !root.is_object() || !root.contains("recentProjects") || !root["recentProjects"].is_array())
            return;

        const auto &arr = root["recentProjects"];
        for (const auto &item : arr)
        {
            if (!item.is_object())
                continue;
            RecentProject rp;
            if (item.contains("name") && item["name"].is_string())
                rp.name = item["name"].get<std::string>();
            if (item.contains("path") && item["path"].is_string())
                rp.path = item["path"].get<std::string>();
            if (item.contains("lastOpened") && item["lastOpened"].is_string())
                rp.lastOpened = item["lastOpened"].get<std::string>();

            rp.exists = std::filesystem::exists(rp.path, ec);
            m_recentProjects.push_back(rp);
        }
    }

    void ProjectManager::saveRecentProjects()
    {
        std::filesystem::path recentFile = getRecentProjectsFilePath();
        json root = json::object();
        json arr = json::array();

        for (const auto &item : m_recentProjects)
        {
            json jItem = json::object();
            jItem["name"] = item.name;
            jItem["path"] = item.path;
            jItem["lastOpened"] = item.lastOpened;
            arr.push_back(jItem);
        }

        root["recentProjects"] = arr;
        std::string err;
        writeTextFile(recentFile, root.dump(2), err);
    }

    const std::vector<RecentProject> &ProjectManager::GetRecentProjects()
    {
        if (!m_recentLoaded)
            loadRecentProjects();
        RefreshRecentProjects();
        return m_recentProjects;
    }

    void ProjectManager::AddRecentProject(const std::string &name, const std::string &path)
    {
        if (!m_recentLoaded)
            loadRecentProjects();

        // Check if already in list
        auto it = std::find_if(m_recentProjects.begin(), m_recentProjects.end(),
                               [&path](const RecentProject &p) { return p.path == path; });

        RecentProject item;
        item.name = name;
        item.path = path;
        item.lastOpened = getCurrentIsoTime();
        std::error_code ec;
        item.exists = std::filesystem::exists(path, ec);

        if (it != m_recentProjects.end())
        {
            m_recentProjects.erase(it);
        }
        m_recentProjects.insert(m_recentProjects.begin(), item);

        // Keep at most 20 recent projects
        if (m_recentProjects.size() > 20)
            m_recentProjects.resize(20);

        saveRecentProjects();
    }

    void ProjectManager::RemoveRecentProject(const std::string &path)
    {
        if (!m_recentLoaded)
            loadRecentProjects();

        auto it = std::remove_if(m_recentProjects.begin(), m_recentProjects.end(),
                                 [&path](const RecentProject &p) { return p.path == path; });
        if (it != m_recentProjects.end())
        {
            m_recentProjects.erase(it, m_recentProjects.end());
            saveRecentProjects();
        }
    }

    void ProjectManager::RefreshRecentProjects()
    {
        std::error_code ec;
        for (auto &p : m_recentProjects)
        {
            p.exists = std::filesystem::exists(p.path, ec);
        }
    }
} // namespace RoninEditor
