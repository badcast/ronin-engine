#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace RoninEditor
{
    enum class ProjectTemplate
    {
        Empty2D = 0,
        Action2D = 1
    };

    struct ProjectConfig
    {
        std::string projectName;
        std::string projectVersion = "0.1.0";
        std::string engineVersion = "0.0.1-alpha";
        std::string projectGuid;
        std::string companyName = "DefaultCompany";
        std::string defaultSceneRelativePath = "Assets/Worlds/SampleWorld.world";

        std::filesystem::path rootPath;
        std::filesystem::path assetsPath;
        std::filesystem::path scriptsPath;
        std::filesystem::path projectSettingsPath;
        std::filesystem::path libraryPath;
        std::filesystem::path projectFilePath;

        void updateDerivedPaths()
        {
            assetsPath = rootPath / "Assets";
            scriptsPath = rootPath / "Scripts";
            projectSettingsPath = rootPath / "ProjectSettings";
            libraryPath = rootPath / "Library";
            projectFilePath = rootPath / "project.ronin";
        }
    };

    struct RecentProject
    {
        std::string name;
        std::string path;
        std::string lastOpened;
        bool exists = false;
    };

    class ProjectManager
    {
    public:
        static ProjectManager &GetInstance();

        // Project lifecycle
        bool CreateProject(const std::string &parentDir, const std::string &projectName,
                           ProjectTemplate tmpl, std::string &outError);
        bool LoadProject(const std::string &path, std::string &outError);
        bool SaveProject(std::string &outError);
        void CloseProject();

        bool HasOpenProject() const { return m_hasOpenProject; }
        const ProjectConfig &GetCurrentProject() const { return m_currentConfig; }
        ProjectConfig &GetCurrentProject() { return m_currentConfig; }

        // Path getters
        std::filesystem::path GetRootPath() const { return m_currentConfig.rootPath; }
        std::filesystem::path GetAssetsPath() const { return m_currentConfig.assetsPath; }
        std::filesystem::path GetScriptsPath() const { return m_currentConfig.scriptsPath; }
        std::filesystem::path GetProjectSettingsPath() const { return m_currentConfig.projectSettingsPath; }
        std::filesystem::path GetLibraryPath() const { return m_currentConfig.libraryPath; }
        std::filesystem::path GetDefaultScenePath() const;

        // Recent projects
        const std::vector<RecentProject> &GetRecentProjects();
        void AddRecentProject(const std::string &name, const std::string &path);
        void RemoveRecentProject(const std::string &path);
        void RefreshRecentProjects();

        // Project scaffolding generators
        static bool GenerateProjectScaffolding(const ProjectConfig &cfg, ProjectTemplate tmpl, std::string &outError);
        static bool GenerateDefaultScene(const std::filesystem::path &scenePath, ProjectTemplate tmpl, std::string &outError);
        static bool GenerateCMakeLists(const std::filesystem::path &scriptsDir, const std::string &projectName, std::string &outError);
        static bool GenerateSampleScript(const std::filesystem::path &scriptsDir, std::string &outError);
        static bool GenerateProjectSettings(const std::filesystem::path &settingsDir, const std::string &projectName, std::string &outError);
        static bool GenerateInputSettings(const std::filesystem::path &settingsDir, std::string &outError);
        static bool GenerateTimeSettings(const std::filesystem::path &settingsDir, std::string &outError);
        static bool GenerateProjectFile(const ProjectConfig &cfg, std::string &outError);

    private:
        ProjectManager();
        ~ProjectManager() = default;
        ProjectManager(const ProjectManager &) = delete;
        ProjectManager &operator=(const ProjectManager &) = delete;

        std::filesystem::path getRecentProjectsFilePath() const;
        void loadRecentProjects();
        void saveRecentProjects();

        ProjectConfig m_currentConfig;
        bool m_hasOpenProject = false;
        std::vector<RecentProject> m_recentProjects;
        bool m_recentLoaded = false;
    };
} // namespace RoninEditor
