#ifndef _RONIN_INSTANCES_H_
#define _RONIN_INSTANCES_H_ 1

#include "basic_ronin.h"

namespace RoninEngine::Runtime
{
    struct IObject_t
    {
        Object inst;
    };

    struct IGameObject_t
    {
        std::list<Component *> m_components;
        std::list<Event> ev_destroy;
        int m_layer;
        int m_zOrder;
        bool m_active;

        GameObject inst;
    };

    template <typename T, typename V>
    struct iget_instance
    {
        static T *operator()(V const &_obj) const
        {
            return static_cast<T *>(&_obj.inst);
        }
    };

    template <typename T>
    struct iget_header
    {
        static void operator()(T const *instance)
        {
            ;
        }
    };

    template <>
    struct iget_header<GameObject>
    {
        constexpr auto _instPos = offsetof(IGameObject_t, inst);

        static IGameObject_t *operator()(GameObject *ref)
        {
            return reinterpret_cast<IGameObject_t *>(static_cast<std::int8_t *>(ref) - _instPos);
        }
    };

} // namespace RoninEngine::Runtime

#endif
