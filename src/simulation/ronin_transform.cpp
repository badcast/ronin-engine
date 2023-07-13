#include "ronin.h"
#include "ronin_matrix.h"

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
            hierarchy_append(World::self()->internal_resources->main_object->transform(), from); // nullptr as Root
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
    bool hierarchy_sibiling(Transform *from, int index)
    {
        // TODO: Set sibling for Transform component
        return false;
    }

    Transform::Transform() : Transform(DESCRIBE_AS_MAIN_OFF(Transform))
    {
    }

    Transform::Transform(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Transform))
    {
        DESCRIBE_AS_MAIN(Transform);
        m_parent = nullptr;
        _angle_ = 0;
        layer = 1;
        // create matrix-slot for transform object
        Matrix::matrix_nature(this, Matrix::matrix_get_key(local_position() + Vec2::one));
    }

    int Transform::child_count() const
    {
        return hierarchy.size();
    }

    Transform *Transform::child_of(int index)
    {
        if(child_count() <= index || child_count() >= index)
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

    void Transform::look_at(Transform *target)
    {
        look_at(target->_position, Vec2::up);
    }

    void Transform::look_at(Transform *target, Vec2 axis)
    {
        look_at(target->_position, axis);
    }

    void Transform::look_at(Vec2 target)
    {
        look_at(target, Vec2::up);
    }

    void Transform::look_at(Vec2 target, Vec2 axis)
    {
        _angle_ = Vec2::angle(axis, target - _position) * Math::rad2deg;
        // normalize horz
        if(axis.x == 1)
        {
            if(_position.y < target.y)
                _angle_ = -_angle_;
        }
        else if(axis.x == -1)
        {
            if(_position.y > target.y)
                _angle_ = -_angle_;
        }
        // normalize vert
        if(axis.y == 1)
        {
            if(_position.x > target.x)
                _angle_ = -_angle_;
        }
        else if(axis.y == -1)
        {
            if(_position.x < target.x)
                _angle_ = -_angle_;
        }
    }

    void Transform::look_at_lerp(Vec2 target, float t)
    {
        Vec2 axis = Vec2::up;
        float a = Vec2::angle(axis, target - _position) * Math::rad2deg;
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

        local_angle(Math::lerp_angle(_angle_, a, t));
    }

    void Transform::look_at_lerp(Transform *target, float t)
    {
        look_at_lerp(target->_position, t);
    }

    void Transform::as_first_child()
    {
        if(this->m_parent == nullptr)
            return;
        hierarchy_sibiling(m_parent, 0); // 0 is first
    }

    bool Transform::child_has(Transform *child)
    {
        return std::find(std::begin(hierarchy), std::end(hierarchy), child) != std::end(hierarchy);
    }

    void Transform::child_append(Transform *child)
    {
        hierarchy_append(this, child);
    }

    void Transform::child_remove(Transform *child)
    {
        hierarchy_remove(this, child);
    }

    const Vec2 Transform::forward() const
    {
        return Vec2::rotate_around(_position, around_frwd, _angle_ * Math::deg2rad);
    }

    const Vec2 Transform::forward(float force) const
    {
        return Vec2::rotate_around(_position, around_frwd * force, _angle_ * Math::deg2rad);
    }

    const Vec2 Transform::back() const
    {
        return Vec2::rotate_around(_position, around_frwd, (_angle_ - 180) * Math::deg2rad);
    }

    const Vec2 Transform::back(float force) const
    {
        return Vec2::rotate_around(_position, around_frwd * force, (_angle_ - 180) * Math::deg2rad);
    }

    const Vec2 Transform::right()
    {
        return transform_direction(Vec2::right);
    }

    const Vec2 Transform::left()
    {
        return transform_direction(Vec2::left);
    }

    const Vec2 Transform::up()
    {
        return transform_direction(Vec2::up);
    }

    const Vec2 Transform::down()
    {
        return transform_direction(Vec2::down);
    }

    const Vec2 Transform::transform_direction(Vec2 direction)
    {
        return Vec2::rotate(_position + direction, _angle_);
    }

    const Vec2 Transform::transform_direction(float x, float y)
    {
        return transform_direction({x, y});
    }

    const bool Transform::look_of_angle(Transform *target, float maxAngle) const
    {
        return look_of_angle(target->_position, maxAngle);
    }

    const bool Transform::look_of_angle(Vec2 target, float maxAngle) const
    {
        float angle = Vec2::angle(this->_position - target, this->_position - this->forward());
        return angle <= maxAngle * Math::deg2rad;
    }

    Vec2 Transform::local_position() const
    {
        return (this->m_parent ? _position - this->m_parent->_position : _position);
    }

    const Vec2 &Transform::local_position(const Vec2 &value)
    {
        Vec2Int lastPoint = Matrix::matrix_get_key(_position);
        _position = (this->m_parent ? this->m_parent->_position + value : value);
        if(game_object()->is_active())
            Matrix::matrix_nature(this, lastPoint);
        return value;
    }

    Vec2 Transform::position() const
    {
        return _position;
    }

    const Vec2 &Transform::position(const Vec2 &value)
    {
        Vec2 lastPosition = _position;
        _position = value; // set the position
        if(_owner->m_active)
        {
            Matrix::matrix_nature(this, Matrix::matrix_get_key(lastPosition));

            for(Transform *chlid : hierarchy)
                chlid->parent_notify(lastPosition);
        }
        return value;
    }

    void Transform::parent_notify(Vec2 lastParentPoint)
    {
        Vec2 last_position = _position;
        // calc new position, after parent change
        _position = m_parent->_position - (lastParentPoint - _position);
        Matrix::matrix_nature(this, Matrix::matrix_get_key(last_position));
        for(Transform *chlid : hierarchy)
            chlid->parent_notify(last_position);
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

    float Transform::angle() const
    {
        return (this->m_parent) ? this->m_parent->_angle_ + this->_angle_ : this->_angle_;
    }

    void Transform::angle(float value)
    {
        this->_angle_ = (this->m_parent) ? this->m_parent->angle() - value : value;
    }

    float Transform::local_angle() const
    {
        return this->_angle_;
    }

    void Transform::local_angle(float value)
    {
        while(value > 360)
            value -= 360;
        this->_angle_ = value;
    }

    Transform *Transform::parent() const
    {
        return m_parent;
    }

    void Transform::set_parent(Transform *parent, bool worldPositionStays)
    {
        // TODO: make worldPositionStays
        if(this->m_parent == nullptr)
        {
            throw RoninEngine::Exception::ronin_transform_change_error();
        }
        Vec2 lastParentPoint = this->m_parent->position();
        // change children of the parent
        hierarchy_parent_change(this, parent);
        // change position child
        this->parent_notify(lastParentPoint);
        this->layer = parent->layer << 1;
    }

} // namespace RoninEngine::Runtime
