#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    void hierarchy_childs_move(Transform *oldParent, Transform *newParent)
    {
        if(oldParent == newParent)
            return;
        for(TransformRef& child : oldParent->hierarchy)
        {
            child->m_parent = newParent->GetRef<Transform>();
        }
        newParent->hierarchy.merge(oldParent->hierarchy);
    }

    void hierarchy_parent_change(TransformRef from, TransformRef newParent)
    {
        // TODO: Fix it Hierarchy function
        TransformRef lastParent = from->m_parent;
        if(newParent && lastParent == newParent)
            return;
        if(lastParent)
        {
            hierarchy_child_remove(lastParent.ptr_, from.ptr_);
        }
        if(!newParent)
            hierarchy_append(World::GetCurrentWorld()->irs->mainObject->transform(), from); // nullptr as Root
        else
        {
            from->m_parent = newParent->GetRef<Transform>();
            hierarchy_append(newParent, from);
        }
    }

    void hierarchy_child_remove(Transform *parent, Transform *who)
    {
        if(who->m_parent.ptr_ != parent)
            return;
        std::list<TransformRef>::iterator iter = std::find(parent->hierarchy.begin(), parent->hierarchy.end(), who->GetRef<Transform>());
        if(iter == parent->hierarchy.end())
            return;
        parent->hierarchy.erase(iter);
        who->m_parent = nullptr; // not parent
    }

    void hierarchy_childs_remove(Transform *parent)
    {
        for(TransformRef &child : parent->hierarchy)
        {
            child->m_parent = nullptr;
        }
        parent->hierarchy.clear();
    }

    void hierarchy_append(TransformRef parent, TransformRef who)
    {
        auto iter = find_if(begin(parent->hierarchy), std::end(parent->hierarchy), std::bind(std::equal_to<TransformRef>(), std::placeholders::_1, who->GetRef<Transform>()));
        if(iter == end(parent->hierarchy))
        {
            who->m_parent = parent;
            parent->hierarchy.emplace_back(who);
        }
    }

    void hierarchy_sibiling(Transform *parent, Transform *who, int index)
    {
        // TODO: Set sibling for Transform component
        std::list<TransformRef>::iterator __off_pos, __target, __end = std::end(parent->hierarchy);
        __off_pos = __target = __end;
        for(auto iter = std::begin(parent->hierarchy); (__off_pos == __end || __target == __end) && iter != __end; ++iter)
        {
            if(__off_pos != __end)
            {
                if(index == 0)
                    __off_pos = iter;
                --index;
            }

            if(__target != __end)
            {
                if((*iter).ptr_ == who)
                    __target = iter;
            }
        }
        if(__target != __end && __off_pos != __end)
            std::swap(__off_pos, __target); // set sibling
    }

    Transform::Transform() : Transform(DESCRIBE_AS_MAIN_OFF(Transform))
    {
    }

    Transform::Transform(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Transform))
    {
        DESCRIBE_AS_MAIN(Transform);
        m_parent = nullptr;
        _angle_ = 0;
    }

    int Transform::childCount() const
    {
        return hierarchy.size();
    }

    TransformRef Transform::childOf(int index)
    {
        if(index < 0 || childCount() < index)
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

    bool Transform::childContain(TransformRef child)
    {
        return std::find(std::begin(hierarchy), std::end(hierarchy), child) != std::end(hierarchy);
    }

    void Transform::childAdd(TransformRef child)
    {
        hierarchy_append(this->GetRef<Transform>(), child);
    }

    void Transform::childRemove(TransformRef child)
    {
        hierarchy_child_remove(this, child.ptr_);
    }

    std::list<TransformRef> Transform::GetChilds() const
    {
        return hierarchy;
    }

    TransformRef Transform::root()
    {
        if(currentWorld == nullptr || currentWorld->irs == nullptr)
            return nullptr;
        return currentWorld->irs->mainObject->transform();
    }

    void Transform::LookAt(TransformRef target)
    {
        LookAt(target->_position, Vec2::down);
    }

    void Transform::LookAt(TransformRef target, Vec2 axis)
    {
        LookAt(target->_position, axis);
    }

    void Transform::LookAt(Vec2 target)
    {
        LookAt(target, Vec2::down);
    }

    void Transform::LookAt(Vec2 target, Vec2 axis)
    {
        float a = Vec2::Angle(axis, target - _position) * Math::rad2deg;

        // FIXME: Using Atan2 for replaced than

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
        LookAtLerp(target, Vec2::zero, t);
    }

    void Transform::LookAtLerp(Vec2 target, Vec2 axis, float t)
    {
        target.x -= _position.x - axis.x;
        target.y -= _position.y - axis.y;

        float newAngle = Math::Atan2(target.y, target.x);

        angle(Math::LerpAngle(_angle_, (Math::Pi / 2 - newAngle) * Math::rad2deg, t));
    }

    void Transform::LookAtLerp(TransformRef target, float t)
    {
        LookAtLerp(target, Vec2::zero, t);
    }

    void Transform::LookAtLerp(TransformRef target, Vec2 axis, float t)
    {
        LookAtLerp(target->_position, axis, t);
    }

    void Transform::AsFirstChild()
    {
        if(this->m_parent == nullptr)
            return;
        hierarchy_sibiling(m_parent.ptr_, this, 0); // 0 is first
    }

    void Transform::AsLastChild()
    {
        if(this->m_parent == nullptr)
            return;
        hierarchy_sibiling(m_parent.ptr_, this, m_parent->hierarchy.size() - 1); // N-1 is last
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
        return Vec2::RotateAround(Vec2::zero, direction, -_angle_ * Math::deg2rad);
    }

    const Vec2 Transform::TransformDirection(float x, float y) const
    {
        return TransformDirection({x, y});
    }

    void Transform::Translate(Vec2 translation)
    {
        position(_position + translation);
    }

    const bool Transform::LookOfAngle(Transform *target, float angle) const
    {
        return LookOfAngle(target->_position, angle);
    }

    const bool Transform::LookOfAngle(Vec2 target, float angle) const
    {
        float _result = Vec2::Angle(this->_position - target, this->_position - this->forward());
        return _result <= angle * Math::deg2rad;
    }

    Vec2 Transform::localPosition() const
    {
        return (this->m_parent ? _position - this->m_parent->_position : _position);
    }

    const Vec2 Transform::localPosition(Vec2 value)
    {
        Vec2Int lastPoint = Matrix::matrix_get_key(_position);
        _position = (this->m_parent ? this->m_parent->_position + value : value);
        if(_owner->m_active)
            Matrix::matrix_update(this, lastPoint);
        return value;
    }

    Vec2 Transform::position() const
    {
        return _position;
    }

    const Vec2 Transform::position(Vec2 value)
    {
        Vec2 lastPosition = _position;

        // set the new position
        _position = value;

        if(_owner->m_active)
        {
            Matrix::matrix_update(this, Matrix::matrix_get_key(lastPosition));

            Vec2 localPos = value - lastPosition;
            for(TransformRef& child : hierarchy)
            {
                child->position(child->_position + localPos);
            }
        }
        return value;
    }

    float Transform::angle() const
    {
        return this->_angle_;
    }

    void Transform::angle(float value)
    {
        // Formulas:
        // 1. For get local position  = child.position - parent.position
        // 2. For get absolute position = parent.position + (child.position - parent.position)
        float angle1;

        if(_angle_ == value || !_owner->m_active)
            return;

        angle1 = _angle_;
        _angle_ = value;

        // Difference
        value = angle1 - value;
        angle1 = value * Math::deg2rad;

        // FIXME: BUG! Hierarchy O(n^2) Frozing of the change position (Recursive)

        for(TransformRef &child : hierarchy)
        {
            // update child position
            child->position(Vec2::RotateAround(_position, child->_position - _position, angle1));

            // update child angle
            child->angle(child->_angle_ - value);
        }
    }

    float Transform::localAngle() const
    {
        return _angle_ - m_parent->_angle_;
    }

    void Transform::localAngle(float value)
    {
        angle(value + m_parent->_angle_);
    }

    void Transform::parent_notify_active_state(GameObjectRef from)
    {
        if(!from->m_active)
        {
            // Delete from matrix, for is not active object
            Matrix::matrix_remove(this);
        }
        else
        {
            // Add to matrix, for active object
            Matrix::matrix_update(this, Matrix::matrix_get_key(Vec2::infinity));
        }
        // send in hierarchy
        for(TransformRef &chlid : hierarchy)
            chlid->parent_notify_active_state(from);
    }

    int Transform::layer() const
    {
        return _owner->m_layer;
    }

    void Transform::layer(int value)
    {
        // Delete from matrix, for is not active object
        if(_owner->m_active)
            Matrix::matrix_remove(this);

        _owner->m_layer = value;

        // Add to matrix, for active object
        if(_owner->m_active)
            Matrix::matrix_update(this, Matrix::matrix_get_key(Vec2::infinity));
    }

    int Transform::zOrder() const
    {
        return _owner->m_zOrder;
    }

    void Transform::zOrder(int value)
    {
        _owner->m_zOrder = value;
    }

    TransformRef Transform::parent() const
    {
        if(m_parent.get() == currentWorld->irs->mainObject->transform().get())
            return nullptr;

        return m_parent;
    }

    void Transform::setParent(TransformRef parent, bool worldPositionStays)
    {
        if(parent == nullptr)
            parent = currentWorld->irs->mainObject->transform();

        // change children of the parent
        hierarchy_parent_change(this->GetRef<Transform>(), parent);

        if(worldPositionStays)
        {
            // change position child
            for(TransformRef &child : hierarchy)
            {
                // Set the world position to childs
                child->position(child->_position - parent->_position);
                // child->position(_position + child->_position);
            }
        }
        else
        {
            // angle(localAngle());
            position(_position + parent->_position);
        }
        parent_notify_active_state(this->_owner);
    }

    void Transform::Detach()
    {
        setParent(nullptr);
    }

    std::vector<TransformRef> Transform::GetAllTransforms()
    {
        TransformRef current;
        std::vector<TransformRef> result {};
        std::queue<TransformRef> queue {};
        queue.push(this->GetRef<Transform>());
        while(!queue.empty())
        {
            current = queue.front();
            queue.pop();

            result.push_back(current);

            for(TransformRef& child : current->hierarchy)
            {
                queue.push(child);
            }
        }
        return result;
    }

} // namespace RoninEngine::Runtime
