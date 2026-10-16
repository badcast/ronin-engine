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
        auto iter = std::find(parent->hierarchy.begin(), parent->hierarchy.end(), who->GetRef<Transform>());
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
        auto iter = std::find(parent->hierarchy.begin(), parent->hierarchy.end(), who->GetRef<Transform>());
        if(iter == parent->hierarchy.end())
        {
            who->m_parent = parent;
            parent->hierarchy.emplace_back(who);
        }
    }

    void hierarchy_sibiling(Transform *parent, Transform *who, int index)
    {
        if (index < 0 || index >= parent->hierarchy.size())
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
        if(index < 0 || childCount() <= index)
            throw std::out_of_range("index");
        return hierarchy[index];
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

    const std::vector<TransformRef>& Transform::GetChilds() const
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

        Vec2 dir = target - _position;
        float cross = axis.x * dir.y - axis.y * dir.x;
        if(cross > 0)
            a = -a;

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
        float _result = Vec2::Angle(target - this->_position, this->forward());
        return _result <= angle * Math::deg2rad;
    }

    Vec2 Transform::localPosition() const
    {
        return (this->m_parent ? _position - this->m_parent->_position : _position);
    }

    const Vec2 Transform::localPosition(Vec2 value)
    {
        position(this->m_parent ? this->m_parent->_position + value : value);
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

        if(_owner->m_active)
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
                if(child->_owner && child->_owner->m_active)
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

        if(_owner->m_active)
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
            
            if(child->_owner && child->_owner->m_active)
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
        if(!from->m_active)
        {
            // Delete from matrix, for is not active object
            Matrix::matrix_remove(this);
        }
        else
        {
            // Add to matrix, for active object
            Matrix::matrix_update(this);
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
            Matrix::matrix_update(this);
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
        if(parent == nullptr && currentWorld && currentWorld->irs)
            parent = currentWorld->irs->mainObject->transform();

        if(parent == this->GetRef<Transform>())
            return;

        Vec2 oldLocalPos = localPosition();

        hierarchy_parent_change(this->GetRef<Transform>(), parent);

        if(!worldPositionStays && parent)
        {
            localPosition(oldLocalPos);
        }
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
            for(TransformRef& child : result[i]->hierarchy)
            {
                result.push_back(child);
            }
        }
        return result;
    }

} // namespace RoninEngine::Runtime
