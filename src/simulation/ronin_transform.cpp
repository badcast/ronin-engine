#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    static const Vec2 around_frwd {1, 1};

    void hierarchy_parent_change(Transform *from, Transform *newParent)
    {
        Transform *lastParent = from->m_parent;

        if(newParent && lastParent == newParent)
            return;

        if(lastParent)
        {
            hierarchy_remove(lastParent, from);
        }

        if(!newParent)
            hierarchy_append(World::self()->irs->main_object->transform(), from); // nullptr as Root
        else
        {
            from->m_parent = newParent;
            hierarchy_append(newParent, from);
        }
    }
    void hierarchy_remove(Transform *from, Transform *off)
    {
        if(off->m_parent != from)
            return;

        auto iter = std::find(from->hierarchy.begin(), from->hierarchy.end(), off);
        if(iter == from->hierarchy.end())
            return;
        from->hierarchy.erase(iter);
        off->m_parent = nullptr; // not parent
    }
    void hierarchy_remove_all(Transform *from)
    {
        for(auto t : from->hierarchy)
        {
            t->m_parent = nullptr;
        }

        from->hierarchy.clear();
    }
    void hierarchy_append(Transform *from, Transform *off)
    {
        auto iter = find_if(begin(from->hierarchy), end(from->hierarchy), std::bind2nd(std::equal_to<Transform *>(), off));
        if(iter == end(from->hierarchy))
        {
            off->m_parent = from;
            from->hierarchy.emplace_back(off);
        }
    }
    void hierarchy_sibiling(Transform *from, Transform *off, int index)
    {
        // TODO: Set sibling for Transform component
        decltype(from->hierarchy)::iterator __off_pos, __target, __end = std::end(from->hierarchy);
        __off_pos = __target = __end;
        for(auto iter = std::begin(from->hierarchy); (__off_pos == __end || __target == __end) && iter != __end; ++iter)
        {
            if(__off_pos != __end)
            {
                if(index == 0)
                    __off_pos = iter;
                --index;
            }

            if(__target != __end)
            {
                if((*iter) == off)
                    __target = iter;
            }
        }

        if(__target != __end && __off_pos != __end)
            std::swap(__off_pos, __target);
    }

    Transform::Transform() : Transform(DESCRIBE_AS_MAIN_OFF(Transform))
    {
    }

    Transform::Transform(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Transform))
    {
        DESCRIBE_AS_MAIN(Transform);
        m_parent = nullptr;
        _angle_ = 0;
        _layer_ = 0;
        // create matrix-slot for transform object
        Matrix::matrix_nature(this, Matrix::matrix_get_key(Vec2::infinity));
    }

    int Transform::childCount() const
    {
        return hierarchy.size();
    }

    Transform *Transform::childOf(int index)
    {
        if(childCount() <= index || childCount() >= index)
            throw std::out_of_range("index");

        auto iter = begin(hierarchy);
        for(int x = 0; iter != end(hierarchy); ++x)
        {
            if(x == index)
            {
                return *iter;
                break;
            }
        }

        return nullptr;
    }

    bool Transform::childContain(Transform *child)
    {
        return std::find(std::begin(hierarchy), std::end(hierarchy), child) != std::end(hierarchy);
    }

    void Transform::childAdd(Transform *child)
    {
        hierarchy_append(this, child);
    }

    void Transform::childRemove(Transform *child)
    {
        hierarchy_remove(this, child);
    }

    Transform *Transform::root()
    {
        return World::self()->irs->main_object->transform();
    }

    void Transform::LookAt(Transform *target)
    {
        LookAt(target->_position, Vec2::up);
    }

    void Transform::LookAt(Transform *target, Vec2 axis)
    {
        LookAt(target->_position, axis);
    }

    void Transform::LookAt(Vec2 target)
    {
        LookAt(target, Vec2::up);
    }

    void Transform::LookAt(Vec2 target, Vec2 axis)
    {
        float a = Vec2::Angle(axis, target - _position) * Math::rad2deg;
        // normalize
        if(axis.x == 1)
        {
            if(_position.y < target.y)
                a = -a;
        }
        else if(axis.x == -1)
        {
            if(_position.y > target.y)
                a = -a;
        }

        if(axis.y == 1)
        {
            if(_position.x > target.x)
                a = -a;
        }
        else if(axis.y == -1)
        {
            if(_position.x < target.x)
                a = -a;
        }

        angle(a);
    }

    void Transform::LookAtLerp(Vec2 target, float t)
    {
        Vec2 axis = Vec2::up;
        float a = Vec2::Angle(axis, target - _position) * Math::rad2deg;
        // normalize
        if(axis.x == 1)
        {
            if(_position.y < target.y)
                a = -a;
        }
        else if(axis.x == -1)
        {
            if(_position.y > target.y)
                a = -a;
        }

        if(axis.y == 1)
        {
            if(_position.x > target.x)
                a = -a;
        }
        else if(axis.y == -1)
        {
            if(_position.x < target.x)
                a = -a;
        }

        angle(Math::LerpAngle(_angle_, a, t));
    }

    void Transform::LookAtLerp(Transform *target, float t)
    {
        LookAtLerp(target->_position, t);
    }

    void Transform::AsFirstChild()
    {
        if(this->m_parent == nullptr)
            return;
        hierarchy_sibiling(m_parent, this, 0); // 0 is first
    }

    void Transform::AsLastChild()
    {
        if(this->m_parent == nullptr)
            return;
        hierarchy_sibiling(m_parent, this, m_parent->hierarchy.size() - 1); // N-1 is last
    }

    const Vec2 Transform::forward() const
    {
        return TransformDirection(Vec2::up);
    }

    const Vec2 Transform::back() const
    {
        return TransformDirection(Vec2::down);
    }

    const Vec2 Transform::right() const
    {
        return TransformDirection(Vec2::right);
    }

    const Vec2 Transform::left() const
    {
        return TransformDirection(Vec2::left);
    }

    const Vec2 Transform::up() const
    {
        return TransformDirection(Vec2::up);
    }

    const Vec2 Transform::down() const
    {
        return TransformDirection(Vec2::down);
    }

    const Vec2 Transform::TransformDirection(Vec2 direction) const
    {
        return Vec2::RotateAround(_position, direction, -_angle_ * Math::deg2rad);
    }

    const Vec2 Transform::TransformDirection(float x, float y) const
    {
        return TransformDirection({x, y});
    }

    const bool Transform::LookOfAngle(Transform *target, float maxAngle) const
    {
        return LookOfAngle(target->_position, maxAngle);
    }

    const bool Transform::LookOfAngle(Vec2 target, float maxAngle) const
    {
        float angle = Vec2::Angle(this->_position - target, this->_position - this->forward());
        return angle <= maxAngle * Math::deg2rad;
    }

    Vec2 Transform::localPosition() const
    {
        return (this->m_parent ? _position - this->m_parent->_position : _position);
    }

    const Vec2 Transform::localPosition(const Vec2 &value)
    {
        Vec2Int lastPoint = Matrix::matrix_get_key(_position);
        _position = (this->m_parent ? this->m_parent->_position + value : value);
        if(_owner->m_active)
            Matrix::matrix_nature(this, lastPoint);
        return value;
    }

    Vec2 Transform::position() const
    {
        return _position;
    }

    const Vec2 Transform::position(const Vec2 &value)
    {
        Vec2 lastPosition = _position;
        _position = value; // set the position
        if(_owner->m_active)
        {
            Matrix::matrix_nature(this, Matrix::matrix_get_key(lastPosition));

            Vec2 localPos = value - lastPosition;
            for(Transform *child : hierarchy)
            {
                child->position(child->_position + localPos);
            }
        }
        return value;
    }

    float Transform::localAngle() const
    {
        return _angle_ - m_parent->_angle_;
    }

    void Transform::localAngle(float value)
    {
        // Lerp range at 0 .. 360
        //        while(value > 360)
        //            value -= 360;
        angle(value + m_parent->_angle_);
    }

    void Transform::parent_notify_active_state(GameObject *from)
    {
        Vec2Int pos = Matrix::matrix_get_key(this->_position);
        if(not from->m_active)
        {
            // Delete from matrix, for is not active object
            Matrix::matrix_nature_pickup(this);
        }
        else
        {
            // Add to matrix, for active object
            Matrix::matrix_nature(this, Matrix::matrix_get_key(Vec2::infinity));
        }
        // send in hierarchy
        for(Transform *chlid : hierarchy)
            chlid->parent_notify_active_state(from);
    }

    int Transform::layer() const
    {
        return _layer_;
    }

    void Transform::layer(int value)
    {
        // TODO: Set the layer component, notify to matrix changed

        // Delete from matrix, for is not active object
        if(_owner->m_active)
            Matrix::matrix_nature_pickup(this);

        _layer_ = value;

        // Add to matrix, for active object
        if(_owner->m_active)
            Matrix::matrix_nature(this, Matrix::matrix_get_key(Vec2::infinity));
    }

    Transform *Transform::parent() const
    {
        if(m_parent == switched_world->irs->main_object->transform())
            return nullptr;

        return m_parent;
    }

    void Transform::setParent(Transform *parent, bool worldPositionStays)
    {
        // TODO: make worldPositionStays
        if(parent == nullptr)
        {
            parent = switched_world->irs->main_object->transform();
        }
        Vec2 lastParentPoint = this->m_parent->_position;
        // change children of the parent
        hierarchy_parent_change(this, parent);
        // change position child
        for(Transform *child : hierarchy)
        {
            // Пересчитаю глобальные координаты для дочернего объекта
            // Установлю новую глобальную позицию для дочернего объекта
            child->position(_position + child->_position);
        }
        // this->layer(parent->_layer_ | _layer_);
    }

    void Transform::detach()
    {
        setParent(nullptr);
    }

    float Transform::angle() const
    {
        return this->_angle_;
    }

    void Transform::angle(float value)
    {
        // Formulas:
        // 1. For get local positoin  = child.position - parent.position
        // 2. For get absolute position = parent.position + (child.position - parent.position)

        float lastAngle = _angle_;
        _angle_ = value;

        if(_owner->m_active)
        {
            value = value - lastAngle; // new diff angle from parent
            // send in hierarchy
            for(Transform *child : hierarchy)
            {
                // update child angle
                child->localAngle(value);
                // update child position
                child->position(Vec2::RotateAround(_position, child->_position - _position, -value * Math::deg2rad));
            }
        }
    }

} // namespace RoninEngine::Runtime
