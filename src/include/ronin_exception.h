#pragma once

#include "basic_ronin.h"

namespace RoninEngine::Exception
{
    class ronin_base_error : public std::runtime_error
    {
    public:
        ronin_base_error(const char *error_str);
    };

    class ronin_init_error : public ronin_base_error
    {
    public:
        ronin_init_error() : ronin_base_error("Engine init fail")
        {
        }
    };

    class ronin_type_error : public ronin_base_error
    {
    public:
        ronin_type_error() : ronin_base_error("Type is invalid and not cast")
        {
        }
    };

    class ronin_implementation_error : public ronin_base_error
    {
    public:
        ronin_implementation_error() : ronin_base_error("Not implemented yet")
        {
        }
    };

    class ronin_load_world_error : public ronin_base_error
    {
    public:
        ronin_load_world_error() : ronin_base_error("World not loaded.")
        {
        }
    };

    class ronin_world_notloaded_error : public ronin_base_error
    {
    public:
        ronin_world_notloaded_error() : ronin_base_error("World is not loaded")
        {
        }
    };

    class ronin_watcher_error : public ronin_base_error
    {
    public:
        ronin_watcher_error() : ronin_base_error("Watcher stack cat not released")
        {
        }
    };

    class ronin_null_error : public ronin_base_error
    {
    public:
        ronin_null_error() : ronin_base_error("Object null referenced")
        {
        }
    };

    class ronin_conflict_component_error : public ronin_base_error
    {
    public:
        ronin_conflict_component_error() : ronin_base_error("Component has Owner GameObject")
        {
        }
    };

    class ronin_cant_release_error : public ronin_base_error
    {
    public:
        ronin_cant_release_error() : ronin_base_error("Object's not released")
        {
        }
    };

    class ronin_ui_layout_error : public ronin_base_error
    {
    public:
        ronin_ui_layout_error() : ronin_base_error("UI Layout not created")
        {
        }
    };

    class ronin_ui_group_parent_error : public ronin_base_error
    {
    public:
        ronin_ui_group_parent_error() : ronin_base_error("UI component not group parent")
        {
        }
    };

    class ronin_transform_change_error : public ronin_base_error
    {
    public:
        ronin_transform_change_error() : ronin_base_error("This transform is not they are parent, or is main parent?")
        {
        }
    };

    class ronin_invlaid_range_error : public ronin_base_error
    {
    public:
        ronin_invlaid_range_error() : ronin_base_error("Invalid range")
        {
        }
    };

    class ronin_uid_nocast_error : public ronin_base_error
    {
    public:
        ronin_uid_nocast_error() : ronin_base_error("UID no cast for it")
        {
        }
    };

    class ronin_out_of_range_error : public ronin_base_error
    {
    public:
        ronin_out_of_range_error() : ronin_base_error("Out of range!")
        {
        }
    };

    class ronin_out_of_mem : public ronin_base_error
    {
    public:
        ronin_out_of_mem() : ronin_base_error("Out Of Memory!")
        {
        }
    };

} // namespace RoninEngine::Exception
