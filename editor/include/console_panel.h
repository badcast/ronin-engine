#pragma once

#include "editor_state.h"

namespace RoninEditor
{
    class ConsolePanel
    {
    public:
        ConsolePanel();
        ~ConsolePanel();

        void Render(EditorState &state);

    private:
        bool m_showInfo = true;
        bool m_showWarning = true;
        bool m_showError = true;
        bool m_collapse = false;
        bool m_clearOnPlay = true;
        bool m_autoScroll = true;

        int m_selectedLogIndex = -1;
        char m_searchFilter[128] = "";
    };
} // namespace RoninEditor
