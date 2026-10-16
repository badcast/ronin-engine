#include "ronin.h"
namespace RoninEngine::Runtime
{

    MoveController2D::MoveController2D() : MoveController2D(DESCRIBE_AS_MAIN_OFF(MoveController2D))
    {
    }

    MoveController2D::MoveController2D(const std::string &name) : Behaviour(DESCRIBE_AS_ONLY_NAME(MoveController2D)), speed(3.0f)
    {
        DESCRIBE_AS_MAIN(MoveController2D);
    }

    void MoveController2D::OnAwake()
    {
        playerCamera = gameObject()->GetComponent<Camera2D>();
        if(playerCamera == nullptr)
            playerCamera = gameObject()->AddComponent<Camera2D>();
        if(speed <= 0.0f)
            speed = 3.0f;
    }

    void MoveController2D::OnUpdate()
    {
        Vec2 axis = Input::GetAxis();
        if(axis == Vec2::zero)
            return;

        float dt = Time::deltaTime();
        if(dt <= 0.0f)
            dt = 1.0f / 60.0f;

        float curSpeed = Input::GetKeyDown(KB_LSHIFT) ? (speed * 2.0f) : speed;
        transform()->Translate(axis.normalized() * (curSpeed * dt));
    }
} // namespace RoninEngine::Runtime
