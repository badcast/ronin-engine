#pragma once

//#define USE_TRANSFORM_HIERARCHY

#include "Component.h"

namespace RoninEngine::Runtime {
class SHARK Transform : public Component {
    friend class RoninEngine::Level;
    friend class GameObject;
    friend class Camera;
    friend class Camera2D;
    friend class Physics2D;
    friend GameObject* Instantiate(GameObject* obj);

protected:
    std::vector<Transform*> hierarchy;

    Transform* m_parent;
    Vec2 p;
    float _angle;

    void parent_notify(Vec2 lastParentPoint);
    void parent_notify_activeState(GameObject* from);

    static void hierarchy_parent_change(Transform* from, Transform* newParent);
    static void hierarchy_remove(Transform* from, Transform* off);
    static void hierarchy_removeAll(Transform* from);
    static void hierarchy_append(Transform* from, Transform* off);
    static void hierarchy_sibiling(Transform* from, int index);

public:
    int layer;

    Transform();
    Transform(const std::string& name);
    ~Transform();

    Transform* parent();
    void setParent(Transform* parent, bool worldPositionStays = true);

    int child_count();
    Transform* child_of(int index);

    Transform* root();

    void LookAt(Vec2 target, Vec2 axis);
    void LookAt(Vec2 target);
    void LookAt(Transform* target, Vec2 axis);
    void LookAt(Transform* target);
    void LookAtLerp(Vec2 target, float t);
    void LookAtLerp(Transform* target, float t);

    void as_first_child();

    bool child_has(Transform* child);
    void child_append(Transform* child);
    void child_remove(Transform* child);

    const Vec2 forward();
    const Vec2 right();
    const Vec2 left();
    const Vec2 up();
    const Vec2 down();

    const Vec2 transformDirection(Vec2 direction);
    const Vec2 transformDirection(float x, float y);

    // get position in world space
    Vec2 position();
    // set position in world space
    void position(const Vec2& value);

    // get position in local space from parent
    Vec2 localPosition();
    // set position in local space from parent
    void localPosition(const Vec2& value);

    float angle();
    void angle(float value);
    float localAngle();
    void localAngle(float value);
};

} // namespace RoninEngine::Runtime
