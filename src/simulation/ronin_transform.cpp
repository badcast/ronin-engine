#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{
    static const Vec2 around_frwd { 1, 1 };

    // TODO: Using that function
    std::string get_hierarchy_ofstring(Runtime::Transform* target)
    {
        static char delim = 0x32;
        std::string delims;
        std::string result;

        std::list<Runtime::Transform*> stack;

        while (target) {
            for (auto& c : target->hierarchy) {
                stack.emplace_back(c);
            }

            result += delims;
            result += target->name();
            result += "\n";

            if (!stack.empty()) {
                target = stack.front();
                stack.pop_front();
            } else
                target = nullptr;
        }

        return result;
    }

    void hierarchy_parent_change(Transform* from, Transform* newParent)
    {
        Transform* lastParent = from->m_parent;

        if (newParent && lastParent == newParent)
            return;

        if (lastParent) {
            hierarchy_remove(lastParent, from);
        }

        if (!newParent)
            hierarchy_append(World::self()->internal_resources->main_object->transform(), from); // nullptr as Root
        else {
            from->m_parent = newParent;
            hierarchy_append(newParent, from);
        }
    }
    void hierarchy_remove(Transform* from, Transform* off)
    {
        if (off->m_parent != from)
            return;

        auto iter = std::find(from->hierarchy.begin(), from->hierarchy.end(), off);
        if (iter == from->hierarchy.end())
            return;
        from->hierarchy.erase(iter);
        off->m_parent = nullptr; // not parent
    }
    void hierarchy_remove_all(Transform* from)
    {
        for (auto t : from->hierarchy) {
            t->m_parent = nullptr;
        }

        from->hierarchy.clear();
    }
    void hierarchy_append(Transform* from, Transform* off)
    {
        auto iter = find_if(begin(from->hierarchy), end(from->hierarchy), [off](Transform* of) { return of == off; });
        if (iter == end(from->hierarchy)) {
            off->m_parent = from;
            from->hierarchy.emplace_back(off);
        }
    }
    bool hierarchy_sibiling(Transform* from, int index)
    {
        // TODO: Set sibling for Transform component
        return false;
    }

    Transform::Transform()
        : Transform(DESCRIBE_AS_MAIN_OFF(Transform))
    {
    }

    Transform::Transform(const std::string& name)
        : Component(DESCRIBE_AS_ONLY_NAME(Transform))
    {
        DESCRIBE_AS_MAIN(Transform);
        m_parent = nullptr;
        _angle_ = 0;
        layer = 1;
        // set as default
        Matrix::matrix_nature(this, Matrix::matrix_get_key(_localPosition + Vec2::one));
    }

    int Transform::child_count() const { return hierarchy.size(); }

    Transform* Transform::child_of(int index)
    {
        if (child_count() <= index || child_count() >= index)
            throw std::out_of_range("index");

        auto iter = begin(hierarchy);
        for (int x = 0; iter != end(hierarchy); ++x) {
            if (x == index) {
                return *iter;
                break;
            }
        }

        return nullptr;
    }

    void Transform::look_at(Transform* target) { look_at(target, Vec2::up); }

    void Transform::look_at(Transform* target, Vec2 axis) { look_at(target->position(), axis); }

    void Transform::look_at(Vec2 target) { look_at(target, Vec2::up); }

    void Transform::look_at(Vec2 target, Vec2 axis)
    {
        _angle_ = Vec2::angle(axis, target - position()) * Math::rad2deg;

        // normalize horz
        if (axis.x == 1) {
            if (_localPosition.y < target.y)
                _angle_ = -_angle_;
        } else if (axis.x == -1) {
            if (_localPosition.y > target.y)
                _angle_ = -_angle_;
        }
        // normalize vert
        if (axis.y == 1) {
            if (_localPosition.x > target.x)
                _angle_ = -_angle_;
        } else if (axis.y == -1) {
            if (_localPosition.x < target.x)
                _angle_ = -_angle_;
        }
    }

    void Transform::look_at_lerp(Vec2 target, float t)
    {
        Vec2 axis = Vec2::up;
        float a = Vec2::angle(axis, target - position()) * Math::rad2deg;
        // normalize
        if (axis.x == 1) {
            if (_localPosition.y < target.y)
                a = -a;
        } else if (axis.x == -1) {
            if (_localPosition.y > target.y)
                a = -a;
        }

        if (axis.y == 1) {
            if (_localPosition.x > target.x)
                a = -a;
        } else if (axis.y == -1) {
            if (_localPosition.x < target.x)
                a = -a;
        }

        local_angle(Math::lerp_angle(_angle_, a, t));
    }

    void Transform::look_at_lerp(Transform* target, float t) { look_at_lerp(target->position(), t); }

    void Transform::as_first_child()
    {
        if (this->m_parent == nullptr)
            return;
        hierarchy_sibiling(m_parent, 0); // 0 is first
    }

    bool Transform::child_has(Transform* child) { return std::find(std::begin(hierarchy), std::end(hierarchy), child) != std::end(hierarchy); }

    void Transform::child_append(Transform* child)
    {
        Transform* t = this;
        hierarchy_append(t, child);
    }
    void Transform::child_remove(Transform* child)
    {
        Transform* t = this;
        hierarchy_remove(this, child);
    }

    const Vec2 Transform::forward() const
    {
        Vec2 pos = position();
        return Vec2::rotate_around(pos, around_frwd, _angle_ * Math::deg2rad);
    }

    const Vec2 Transform::forward(float force) const
    {
        Vec2 pos = position();
        return Vec2::rotate_around(pos, around_frwd * force, _angle_ * Math::deg2rad);
    }

    const Vec2 Transform::back() const
    {
        Vec2 pos = position();
        return Vec2::rotate_around(pos, around_frwd, (_angle_ - 180) * Math::deg2rad);
    }

    const Vec2 Transform::back(float force) const
    {
        Vec2 pos = position();
        return Vec2::rotate_around(pos, around_frwd * force, (_angle_ - 180) * Math::deg2rad);
    }

    const Vec2 Transform::right() { return transform_direction(Vec2::right); }

    const Vec2 Transform::left() { return transform_direction(Vec2::left); }

    const Vec2 Transform::up() { return transform_direction(Vec2::up); }

    const Vec2 Transform::down() { return transform_direction(Vec2::down); }

    const Vec2 Transform::transform_direction(Vec2 direction)
    {
        Vec2 pos = position();
        return Vec2::rotate(pos + direction, _angle_);
    }

    const Vec2 Transform::transform_direction(float x, float y) { return transform_direction({ x, y }); }

    const bool Transform::look_of_angle(Transform* target, float maxAngle) const { return look_of_angle(target->position(), maxAngle); }

    const bool Transform::look_of_angle(Vec2 target, float maxAngle) const
    {
        float angle = Vec2::angle(this->position() - target, this->position() - this->forward());
        return angle <= maxAngle * Math::deg2rad;
    }

    Vec2 Transform::local_position() const { return _localPosition; }

    const Vec2& Transform::local_position(const Vec2& value)
    {
        if (value != _localPosition) {
            Vec2Int lastPoint = Matrix::matrix_get_key(position());
            _localPosition = value;
            if (game_object()->is_active())
                Matrix::matrix_nature(this, lastPoint);
        }
        return value;
    }

    Vec2 Transform::position() const { return (this->m_parent) ? this->m_parent->position() + _localPosition : _localPosition; }

    const Vec2& Transform::position(const Vec2& value)
    {
        if (value != position()) {
            Vec2 lastPoint = position();
            _localPosition = (this->m_parent) ? this->m_parent->position() + value : value; // set the position
            if (game_object()->is_active())
                Matrix::matrix_nature(this, Matrix::matrix_get_key(lastPoint));
            for (Transform* chlid : hierarchy)
                chlid->parent_notify(lastPoint);
        }
        return value;
    }

    void Transform::parent_notify(Vec2 lastParentPoint)
    {
        Vec2 lastPoint = lastParentPoint + _localPosition; // world cordinates
        if (game_object()->is_active())
            Matrix::matrix_nature(this, Matrix::matrix_get_key(lastPoint));
        for (Transform* chlid : hierarchy)
            chlid->parent_notify(lastPoint);
    }

    void Transform::parent_notify_active_state(GameObject* from)
    {
        Vec2Int pos = Matrix::matrix_get_key(this->position());
        if (!from->is_active()) {
            decltype(World::self()->internal_resources->matrix)::iterator iter = World::self()->internal_resources->matrix.find(pos);
            // Delete from matrix
            if (iter != std::end(World::self()->internal_resources->matrix)) {
                iter->second.erase(transform());
                if (iter->second.empty()) {
                    World::self()->internal_resources->matrix.erase(iter);
                }
            }
        } else {
            // Add to matrix
            World::self()->internal_resources->matrix[pos].insert(this);
        }
        // send in hierarchy
        for (Transform* chlid : hierarchy)
            chlid->parent_notify_active_state(from);
    }

    float Transform::angle() const { return (this->m_parent) ? this->m_parent->_angle_ + this->_angle_ : this->_angle_; }

    void Transform::angle(float value) { this->_angle_ = (this->m_parent) ? this->m_parent->angle() - value : value; }

    float Transform::local_angle() const { return this->_angle_; }

    void Transform::local_angle(float value)
    {
        while (value > 360)
            value -= 360;
        this->_angle_ = value;
    }
    Transform* Transform::parent() const { return m_parent; }

    void Transform::set_parent(Transform* parent, bool worldPositionStays)
    {
        if (this->m_parent == nullptr) {
            throw RoninEngine::Exception::ronin_transform_change_error();
        }
        Vec2 lastParentPoint = this->m_parent->position();
        // change children of the parent
        hierarchy_parent_change(this, parent);
        // change position child
        this->parent_notify(lastParentPoint);
        this->layer = parent->layer + 1;
    }

} // namespace RoninEngine::Runtime
