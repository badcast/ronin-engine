#pragma once

#include "editor_state.h"
#include <string>

namespace RoninEditor
{
    class HierarchyPanel
    {
    public:
        HierarchyPanel();
        ~HierarchyPanel();

        void Render(EditorState &state);

    private:
        void renderObjectTree(EditorState &state, int parentId);
        void renderObjectNode(EditorState &state, EditorObject &obj);
        void renderContextMenu(EditorState &state, EditorObject *targetObj);

        char m_searchFilter[128] = "";
        int m_renamingObjId = -1;
        char m_renameBuf[128] = "";
        std::vector<int> m_pendingDeleteIds;
        int m_lastSelectedId = -1;
    };
} // namespace RoninEditor
