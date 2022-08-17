#pragma once

#include "begin.h"

namespace RoninEngine {
namespace Runtime {

extern Transform* create_empty_transform();
extern GameObject* create_empty_gameobject();

extern GameObject* CreateGameObject();
extern GameObject* CreateGameObject(const std::string& name);

//Уничтожает объект после рендера.
extern void Destroy(Object* obj);

//Уничтожает объект после прошедшего времени.
extern void Destroy(Object* obj, float t);

//Уничтожает объект принудительно игнорируя все условия его существования.
extern void Destroy_Immediate(Object* obj);

//Проверка на существование объекта
extern bool instanced(Object* obj);

// clone a object
template <typename ObjectType>
ObjectType* Instantiate(ObjectType* obj);
//Клонирует объект
GameObject* Instantiate(GameObject* obj, Vec2 position, float angle = 0);
//Клонирует объект
GameObject* Instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionStay = true);

class RONIN_API Object {
    template <typename ObjectType>
    friend ObjectType* Instantiate(ObjectType* obj);
    friend GameObject* Instantiate(GameObject* obj);
    friend GameObject* Instantiate(GameObject* obj, Vec2 position, float angle);
    friend GameObject* Instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionState);

    friend void Destroy(Object* obj);
    friend void Destroy(Object* obj, float t);
    friend void Destroy_Immediate(Object* obj);
    std::size_t id;

   protected:
    std::string m_name;

   public:
    std::string& name(const std::string& newName);
    const std::string& name();

    std::size_t ID();

    Object();
    Object(const std::string& name);
    virtual ~Object() = default;

    void Destroy();

    /// Check a exist an object
    operator bool();
};

template <typename base, typename _derived>
constexpr bool object_base_of() {
    return std::is_base_of<base, _derived>();
}

template <typename base, typename _derived>
constexpr bool object_base_of(base* obj, _derived* compare) {
    return std::is_base_of<base, _derived>();
}
}  // namespace Runtime
}  // namespace RoninEngine
