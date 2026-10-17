#include <iostream>
#include <string>
#include <cstring>
#include "editor_app.h"
#include <ronin/RoninSimulator.h>

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--version") == 0 || std::strcmp(argv[i], "-v") == 0)
        {
            std::cout << "Ronin Editor v" << RONIN_EDITOR_VERSION_STRING
                      << " (RoninEngine v" << RoninEngine::RoninSimulator::GetVersionString() << ")"
                      << " [Dear ImGui Docking Backend]" << std::endl;
            return 0;
        }
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
        {
            std::cout << "Usage: RoninEditor [options] [project_path]\n"
                      << "Options:\n"
                      << "  -v, --version         Display editor and engine version\n"
                      << "  -h, --help            Show this help message\n"
                      << "  --project <path>      Directly open project at <path>\n"
                      << std::endl;
            return 0;
        }
    }

    auto &app = RoninEditor::EditorApp::GetInstance();
    if (!app.Init(argc, argv))
    {
        std::cerr << "Failed to initialize Ronin Editor application." << std::endl;
        return 1;
    }

    app.Run();
    return 0;
}
