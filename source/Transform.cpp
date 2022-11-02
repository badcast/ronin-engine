#include "framework.h"

namespace RoninEngine::Runtime {

Transform::Transform() : Transform(typeid(*this).name()) {}

Transform::Transform(const std::string& name) : Component(name) {
    m_parent = nullptr;
    _angle = 0;
    layer = 1;
    // set as default
    Level::self()->matrix_nature(this, Vec2::RoundToInt(p + Vec2::one));
}

Transform::~Transform() {
    if (m_parent) {
        hierarchy_remove(m_parent, this);
    }
    hierarchy_removeAll(this);
}

int Transform::child_count() { return hierarchy.size(); }

Transform* Transform::child_of(int index) {
    Transform* tf = nullptr;
    int x;

    if (child_count() <= index || child_count() >= index) throw std::out_of_range("index");

    auto iter = begin(hierarchy);
    for (x = 0; iter != end(hierarchy); ++x) {
        if (x == index) {
            tf = *iter;
            break;
        }
    }

    return tf;
}

void Transform::LookAt(Transform* target) { LookAt(target, Vec2::up); }

void Transform::LookAt(Transform* target, Vec2 axis) { LookAt(target->p, axis); }

void Transform::LookAt(Vec2 target) { LookAt(target, Vec2::up); }

void Transform::LookAt(Vec2 target, Vec2 axis) {
    _angle = Vec2::Angle(axis, target - position()) * Math::Rad2Deg;

    // normalize horz
    if (axis.x == 1) {
        if (p.y < target.y) _angle = -_angle;
    } else if (axis.x == -1) {
        if (p.y > target.y) _angle = -_angle;
    }
    // normalize vert
    if (axis.y == 1) {
        if (p.x > target.x) _angle = -_angle;
    } else if (axis.y == -1) {
        if (p.x < target.x) _angle = -_angle;
    }
}

void Transform::LookAtLerp(Vec2 target, float t) {
    Vec2 axis = Vec2::up;
    float a = Vec2::Angle(axis, target - position()) * Math::Rad2Deg;
    // normalize
    if (axis.x == 1) {
        if (p.y < target.y) a = -a;
    } else if (axis.x == -1) {
        if (p.y > target.y) a = -a;
    }

    if (axis.y == 1) {
        if (p.x > target.x) a = -a;
    } else if (axis.y == -1) {
        if (p.x < target.x) a = -a;
    }

    localAngle(Math::LerpAngle(_angle, a, t));
}

void Transform::LookAtLerp(Transform* target, float t) { LookAtLerp(target->p, t); }

void Transform::as_first_child() {
    if (this->m_parent == nullptr) return;
    hierarchy_sibiling(m_parent, 0);  // 0 is first
}

bool Transform::child_has(Transform* child) {
    return std::find(std::begin(hierarchy), std::end(hierarchy), child) != std::end(hierarchy);
}

void Transform::child_append(Transform* child) {
    Transform* t = this;
    hierarchy_append(t, child);
}
void Transform::child_remove(Transform* child) {
    Transform* t = this;
    hierarchy_remove(t, child);
}

const Vec2 Transform::forward() { return transformDirection(Vec2::up); }

const Vec2 Transform::right() { return transformDirection(Vec2::right); }

const Vec2 Transform::left() { return transformDirection(Vec2::left); }

const Vec2 Transform::up() { return transformDirection(Vec2::up); }

const Vec2 Transform::down() { return transformDirection(Vec2::down); }

const Vec2 Transform::transformDirection(Vec2 direction) { return Vec2::RotateAround(p, direction, _angle * Math::Deg2Rad); }

const Vec2 Transform::transformDirection(float x, float y) { return transformDirection(Vec2(x, y)); }

Vec2 Transform::localPosition() { return p; }
void Transform::localPosition(const Vec2& value) {
    if (value == p) return;
    Vec2Int lastPoint = Vec2::RoundToInt(position());
    p = value;
    if (gameObject()->isActive()) Level::self()->matrix_nature(this, lastPoint);
}

Vec2 Transform::position() { return (this->m_parent) ? this->m_parent->position() + p : p; }

void Transform::position(const Vec2& value) {
    if (value == position()) return;
    Vec2 lastPoint = position();
    p = (this->m_parent) ? this->m_parent->position() + value : value;  // set the position
    if (gameObject()->isActive()) Level::self()->matrix_nature(this, Vec2::RoundToInt(lastPoint));
    for (Transform* chlid : hierarchy) chlid->parent_notify(lastPoint);
}

void Transform::parent_notify(Vec2 lastParentPoint) {
    Vec2 lastPoint = lastParentPoint + p;  // world cordinates
    if (gameObject()->isActive()) Level::self()->matrix_nature(this, Vec2::RoundToInt(lastPoint));
    for (Transform* chlid : hierarchy) chlid->parent_notify(lastPoint);
}

void Transform::parent_notify_activeState(GameObject* from) {
    Vec2Int pos = Vec2::RoundToInt(this->position());
    if (!from->isActive()) {
        decltype(Level::self()->matrixWorld)::iterator iter = Level::self()->matrixWorld.find(pos);
        // Delete from matrix
        if (iter != std::end(Level::self()->matrixWorld)) {
            iter->second.erase(transform());
            if (iter->second.empty()) {
                Level::self()->matrixWorld.erase(iter);
            }
        }
    } else {
        // Add to matrix
        Level::self()->matrixWorld[pos].insert(this);
    }
    // send in hierarchy
    for (Transform* chlid : hierarchy) chlid->parent_notify_activeState(from);
}

float Transform::angle() { return (this->m_parent) ? this->m_parent->_angle + this->_angle : this->_angle; }

void Transform::angle(float value) {
    this->_angle = (this->m_parent) ? this->m_parent->angle() - value : value;
}

float Transform::localAngle() { return this->_angle; }
void Transform::localAngle(float value) {
    while (value > 360) value -= 360;
    this->_angle = value;
}
Transform* Transform::parent() { return m_parent; }

void Transform::setParent(Transform* parent, bool worldPositionStays) {
    if (this->m_parent == nullptr) {
        throw std::runtime_error("This transform is not they are parent, or is main parent?");
    }
    Vec2 lastParentPoint = this->m_parent->position();
    // change children of the parent
    hierarchy_parent_change(this, parent);
    // change position child
    this->parent_notify(lastParentPoint);
    this->layer = parent->layer+1;
}

void Transform::hierarchy_parent_change(Transform* from, Transform* newParent) {
    Transform* lastParent = from->m_parent;

    if (newParent && lastParent == newParent) return;

    if (lastParent) {
        hierarchy_remove(lastParent, from);
    }

    if (!newParent)
        hierarchy_append(Level::self()->main_object->transform(),
                         from);  // nullptr as Root
    else {
        from->m_parent = newParent;
        hierarchy_append(newParent, from);
    }
}
void Transform::hierarchy_remove(Transform* from, Transform* off) {
    if (off->m_parent != from) return;

    auto iter = std::find(from->hierarchy.begin(), from->hierarchy.end(), off);
    if (iter == from->hierarchy.end()) return;
    from->hierarchy.erase(iter);
    from->hierarchy.shrink_to_fit();
    off->m_parent = nullptr;  // not parent
}
void Transform::hierarchy_removeAll(Transform* from) {
    for (auto t : from->hierarchy) {
        t->m_parent = nullptr;
    }

    from->hierarchy.clear();
    from->hierarchy.shrink_to_fit();
}
void Transform::hierarchy_append(Transform* from, Transform* off) {
    auto iter = find_if(begin(from->hierarchy), end(from->hierarchy), [off](Transform* of) { return of == off; });
    if (iter == end(from->hierarchy)) {
        off->m_parent = from;
        from->hierarchy.emplace_back(off);
    }
}
void Transform::hierarchy_sibiling(Transform* from, int index) {}
}  // namespace RoninEngine::Runtime
