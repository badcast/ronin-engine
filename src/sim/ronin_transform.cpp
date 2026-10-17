#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    void hierarchy_childs_move(Transform *oldParent, Transform *newParent)
    {
        if(!oldParent || !newParent || oldParent == newParent)
            return;
        for(TransformRef& child : oldParent->hierarchy)
        {
            if(child)
                child->m_parent = newParent->GetRef<Transform>();
        }
        newParent->hierarchy.insert(newParent->hierarchy.end(), oldParent->hierarchy.begin(), oldParent->hierarchy.end());
        oldParent->hierarchy.clear();
    }

    void hierarchy_parent_change(TransformRef from, TransformRef newParent)
    {
        if(!from)
            return;
        if(newParent == from)
            return;
        for(TransformRef p = newParent; p; p = p->m_parent)
        {
            if(p == from)
                return;
        }
        TransformRef lastParent = from->m_parent;
        if(newParent && lastParent == newParent)
            return;
        if(lastParent)
        {
            hierarchy_child_remove(lastParent.ptr_, from.ptr_);
        }
        if(!newParent)
        {
            World *cw = World::GetCurrentWorld();
            if(cw && cw->irs && !cw->irs->mainObject.isNull() && cw->irs->mainObject->transform() != from)
                hierarchy_append(cw->irs->mainObject->transform(), from); // nullptr as Root
            else
                from->m_parent = nullptr;
        }
        else
        {
            from->m_parent = newParent->GetRef<Transform>();
            hierarchy_append(newParent, from);
        }
    }

    void hierarchy_child_remove(Transform *parent, Transform *who)
    {
        if(!parent || !who || who->m_parent.ptr_ != parent)
            return;
        auto iter = std::find(parent->hierarchy.begin(), parent->hierarchy.end(), who->GetRef<Transform>());
        if(iter == parent->hierarchy.end())
            return;
        parent->hierarchy.erase(iter);
        who->m_parent = nullptr; // not parent
    }

    void hierarchy_childs_remove(Transform *parent)
    {
        if(!parent)
            return;
        for(TransformRef &child : parent->hierarchy)
        {
            if(child)
                child->m_parent = nullptr;
        }
        parent->hierarchy.clear();
    }

    void hierarchy_append(TransformRef parent, TransformRef who)
    {
        if(!parent || !who || parent == who)
            return;
        auto iter = std::find(parent->hierarchy.begin(), parent->hierarchy.end(), who->GetRef<Transform>());
        if(iter == parent->hierarchy.end())
        {
            who->m_parent = parent;
            parent->hierarchy.emplace_back(who);
        }
    }

    void hierarchy_sibiling(Transform *parent, Transform *who, int index)
    {
        if(!parent || !who || index < 0 || static_cast<size_t>(index) >= parent->hierarchy.size())
            return;
        auto iter = std::find(parent->hierarchy.begin(), parent->hierarchy.end(), who->GetRef<Transform>());
        if (iter != parent->hierarchy.end())
        {
            auto target = parent->hierarchy.begin() + index;
            if (iter != target)
            {
                TransformRef val = *iter;
                parent->hierarchy.erase(iter);
                parent->hierarchy.insert(parent->hierarchy.begin() + index, val);
            }
        }
    }

    Transform::Transform() : Transform(DESCRIBE_AS_MAIN_OFF(Transform))
    {
    }

    Transform::Transform(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Transform)), m_parent(nullptr), _position(Vec2::zero), _angle_(0.0f)
    {
        DESCRIBE_AS_MAIN(Transform);
    }

    int Transform::childCount() const
    {
        return static_cast<int>(hierarchy.size());
    }

    TransformRef Transform::childOf(int index) const
    {
        if(index < 0 || childCount() <= index)
            throw std::out_of_range("index");
        return hierarchy[index];
    }

    bool Transform::childContain(TransformRef child) const
    {
        return std::find(std::begin(hierarchy), std::end(hierarchy), child) != std::end(hierarchy);
    }

    void Transform::childAdd(TransformRef child)
    {
        if(!child || child.ptr_ == this)
            return;
        hierarchy_parent_change(child, this->GetRef<Transform>());
    }

    void Transform::childRemove(TransformRef child)
    {
        hierarchy_child_remove(this, child.ptr_);
    }

    const std::vector<TransformRef>& Transform::GetChilds() const
    {
        return hierarchy;
    }

    TransformRef Transform::root()
    {
        if(currentWorld == nullptr || currentWorld->irs == nullptr || currentWorld->irs->mainObject.isNull())
            return nullptr;
        return currentWorld->irs->mainObject->transform();
    }

    void Transform::LookAt(TransformRef target)
    {
        if(!target)
            return;
        LookAt(target->_position, Vec2::up);
    }

    void Transform::LookAt(TransformRef target, Vec2 axis)
    {
        if(!target)
            return;
        LookAt(target->_position, axis);
    }

    void Transform::LookAt(Vec2 target)
    {
        LookAt(target, Vec2::up);
    }

    void Transform::LookAt(Vec2 target, Vec2 axis)
    {
        Vec2 dir = target - _position;
        if(dir == Vec2::zero)
            return;
        if(axis == Vec2::zero)
            axis = Vec2::up;

        float a = Vec2::Angle(axis, dir) * Math::rad2deg;
        float cross = axis.x * dir.y - axis.y * dir.x;
        if(cross > 0)
            a = -a;

        angle(a);
    }

    void Transform::LookAtLerp(Vec2 target, float t)
    {
        LookAtLerp(target, Vec2::up, t);
    }

    void Transform::LookAtLerp(Vec2 target, Vec2 axis, float t)
    {
        Vec2 dir = target - _position;
        if(dir == Vec2::zero)
            return;
        if(axis == Vec2::zero)
            axis = Vec2::up;

        float targetAngle = Vec2::Angle(axis, dir) * Math::rad2deg;
        float cross = axis.x * dir.y - axis.y * dir.x;
        if(cross > 0)
            targetAngle = -targetAngle;

        angle(Math::LerpAngle(_angle_, targetAngle, t));
    }

    void Transform::LookAtLerp(TransformRef target, float t)
    {
        if(!target)
            return;
        LookAtLerp(target->_position, Vec2::up, t);
    }

    void Transform::LookAtLerp(TransformRef target, Vec2 axis, float t)
    {
        if(!target)
            return;
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
        if(this->m_parent == nullptr || m_parent->hierarchy.empty())
            return;
        hierarchy_sibiling(m_parent.ptr_, this, static_cast<int>(m_parent->hierarchy.size()) - 1); // N-1 is last
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
        if(!target)
            return false;
        return LookOfAngle(target->_position, angle);
    }

    const bool Transform::LookOfAngle(TransformRef target, float angle) const
    {
        if(!target)
            return false;
        return LookOfAngle(target->_position, angle);
    }

    const bool Transform::LookOfAngle(Vec2 target, float angle) const
    {
        float _result = Vec2::Angle(target - this->_position, this->forward());
        return _result <= angle * Math::deg2rad;
    }

    Vec2 Transform::localPosition() const
    {
        if(!this->m_parent)
            return _position;
        Vec2 offset = _position - this->m_parent->_position;
        if(this->m_parent->_angle_ != 0.0f)
            offset = Vec2::Rotate(offset, this->m_parent->_angle_ * Math::deg2rad);
        return offset;
    }

    const Vec2 Transform::localPosition(Vec2 value)
    {
        if(!this->m_parent)
        {
            position(value);
        }
        else
        {
            Vec2 worldOffset = (this->m_parent->_angle_ != 0.0f)
                                   ? Vec2::Rotate(value, -this->m_parent->_angle_ * Math::deg2rad)
                                   : value;
            position(this->m_parent->_position + worldOffset);
        }
        return value;
    }

    Vec2 Transform::position() const
    {
        return _position;
    }

    const Vec2 Transform::position(Vec2 value)
    {
        if (_position == value)
            return value;

        Vec2 localPos = value - _position;

        // set the new position
        _position = value;

        if(_owner && _owner->isActiveInHierarchy())
            Matrix::matrix_update(this);

        if(localPos != Vec2::zero)
        {
            std::vector<TransformRef> descendants = hierarchy;
            for(size_t i = 0; i < descendants.size(); ++i)
            {
                TransformRef child = descendants[i];
                if(child.isNull())
                    continue;
                child->_position += localPos;
                if(child->_owner && child->_owner->isActiveInHierarchy())
                    Matrix::matrix_update(child.ptr_);
                for(TransformRef& gc : child->hierarchy)
                    descendants.push_back(gc);
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
        if(_angle_ == value)
            return;

        float diffAngle = _angle_ - value;
        float diffRad = diffAngle * Math::deg2rad;
        _angle_ = value;

        if(_owner && _owner->isActiveInHierarchy())
            Matrix::matrix_update(this);

        std::vector<TransformRef> descendants = hierarchy;
        for(size_t i = 0; i < descendants.size(); ++i)
        {
            TransformRef child = descendants[i];
            if(child.isNull())
                continue;
            
            // update child position rotated around parent
            child->_position = Vec2::RotateAround(_position, child->_position - _position, diffRad);
            // update child angle
            child->_angle_ -= diffAngle;
            
            if(child->_owner && child->_owner->isActiveInHierarchy())
                Matrix::matrix_update(child.ptr_);
            
            for(TransformRef& gc : child->hierarchy)
                descendants.push_back(gc);
        }
    }

    float Transform::localAngle() const
    {
        return m_parent ? _angle_ - m_parent->_angle_ : _angle_;
    }

    void Transform::localAngle(float value)
    {
        angle(value + (m_parent ? m_parent->_angle_ : 0.0f));
    }

    void Transform::parent_notify_active_state(GameObjectRef from)
    {
        bool isRoot = (currentWorld && currentWorld->irs && !currentWorld->irs->mainObject.isNull() &&
                       currentWorld->irs->mainObject->transform().ptr_ == this);
        if(!isRoot)
        {
            if(!from || !from->m_active || !_owner || !_owner->isActiveInHierarchy())
            {
                // Delete from matrix, for is not active object
                Matrix::matrix_remove(this);
            }
            else
            {
                // Add to matrix, for active object
                Matrix::matrix_update(this);
            }
        }
        // send in hierarchy
        for(TransformRef &chlid : hierarchy)
        {
            if(chlid)
                chlid->parent_notify_active_state(from);
        }
    }

    int Transform::layer() const
    {
        return _owner ? _owner->m_layer : 0;
    }

    void Transform::layer(int value)
    {
        if(!_owner || _owner->m_layer == value)
            return;

        // Delete from matrix, for is not active object
        if(_owner->isActiveInHierarchy())
            Matrix::matrix_remove(this);

        _owner->m_layer = value;

        // Add to matrix, for active object
        if(_owner->isActiveInHierarchy())
            Matrix::matrix_update(this);
    }

    int Transform::zOrder() const
    {
        return _owner ? _owner->m_zOrder : 0;
    }

    void Transform::zOrder(int value)
    {
        if(_owner)
            _owner->m_zOrder = value;
    }

    TransformRef Transform::parent() const
    {
        if(currentWorld && currentWorld->irs && !currentWorld->irs->mainObject.isNull() &&
           m_parent.get() == currentWorld->irs->mainObject->transform().get())
            return nullptr;

        return m_parent;
    }

    void Transform::setParent(TransformRef parent, bool worldPositionStays)
    {
        if(parent == nullptr && currentWorld && currentWorld->irs && !currentWorld->irs->mainObject.isNull())
            parent = currentWorld->irs->mainObject->transform();

        if(parent == this->GetRef<Transform>())
            return;

        for(TransformRef p = parent; p; p = p->m_parent)
        {
            if(p.ptr_ == this)
                return;
        }

        Vec2 oldLocalPos = localPosition();
        float oldLocalAngle = localAngle();

        hierarchy_parent_change(this->GetRef<Transform>(), parent);

        if(!worldPositionStays && parent)
        {
            localPosition(oldLocalPos);
            localAngle(oldLocalAngle);
        }
        if(this->_owner)
            parent_notify_active_state(this->_owner);
    }

    void Transform::Detach()
    {
        setParent(nullptr);
    }

    std::vector<TransformRef> Transform::GetAllTransforms()
    {
        std::vector<TransformRef> result;
        result.push_back(this->GetRef<Transform>());
        for(size_t i = 0; i < result.size(); ++i)
        {
            if(!result[i])
                continue;
            for(TransformRef& child : result[i]->hierarchy)
            {
                if(child)
                    result.push_back(child);
            }
        }
        return result;
    }

} // namespace RoninEngine::Runtime
