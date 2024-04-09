#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    /**
     * @brief Enumerates system cursor IDs for use with RoninCursor::GetSystemCursor().
     * Based from SDL2 API
     */
    enum class SystemCursorID
    {
        SystemArrow,     /**< Arrow cursor. */
        SystemIbeam,     /**< I-beam cursor. */
        SystemWait,      /**< Wait cursor. */
        SystemCrosshair, /**< Crosshair cursor. */
        SystemWaitArrow, /**< Small wait cursor (or Wait if not available). */
        SystemSizeNWSE,  /**< Double arrow pointing northwest and southeast. */
        SystemSizeNESW,  /**< Double arrow pointing northeast and southwest. */
        SystemSizeWE,    /**< Double arrow pointing west and east. */
        SystemSizeNS,    /**< Double arrow pointing north and south. */
        SystemSizeAll,   /**< Four pointed arrow pointing north, south, east, and west. */
        SystemNo,        /**< Slashed circle or crossbones cursor. */
        SystemHand       /**< Hand cursor. */
    };

    /**
     * @brief A class for managing cursors in the Ronin engine.
     */
    class RONIN_API RoninCursor
    {
    public:
        /**
         * @brief Sets the cursor to the specified custom cursor.
         *
         * @param cursor The custom cursor to set.
         */
        static void SetCursor(Cursor *cursor);

        /**
         * @brief Retrieves the currently set cursor.
         *
         * @return A pointer to the currently set cursor.
         */
        static Cursor *GetCursor();

        /**
         * @brief Retrieves the default cursor.
         *
         * @return  A pointer to the default cursor.
         */
        static Cursor *GetDefaultCursor();

        /**
         * @brief Shows or hides the cursor.
         *
         * @param toggle True to show the cursor, false to hide it.
         */
        static bool ShowCursor(bool toggle);

        /**
         * @brief Checks if the cursor is currently shown.
         *
         * @return True if the cursor is shown, false otherwise.
         */
        static bool IsCursorShown();

        /**
         * @brief Capture mouse on windows
         * @param capture The bool of the for set capturing.
         * @return True if the cursor is captured, false otherwise.
         */
        static bool CaptureMouse(bool capture);

        /**
         * @brief Retrieves a system cursor by its ID.
         *
         * @param id The ID of the system cursor to retrieve.
         * @return A pointer to the system cursor.
         */
        static Cursor *GetSystemCursor(SystemCursorID id);
    };
} // namespace RoninEngine::Runtime
