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
        std::list<GameObject::Event> ev_destroy;
        int m_layer;
        int m_zOrder;
        bool m_active;

        GameObject inst;
    };

    template <typename T, typename V>
    struct iget_instance
    {
        T *operator()(V &_obj) const
        {
            return static_cast<T *>(&_obj.inst);
        }
    };

    template <typename T>
    struct iget_header
    {
        void operator()(T const *instance) const
        {
            (void)instance;
        }
    };

    template <>
    struct iget_header<GameObject>
    {
        static constexpr auto _instPos = offsetof(IGameObject_t, inst);

        IGameObject_t *operator()(GameObject *ref) const
        {
            return reinterpret_cast<IGameObject_t *>(reinterpret_cast<std::int8_t *>(ref) - _instPos);
        }
    };

} // namespace RoninEngine::Runtime

#endif
