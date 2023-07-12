#include "ronin.h"
namespace RoninEngine::Runtime
{

    MoveController2D::MoveController2D() : MoveController2D(DESCRIBE_AS_MAIN_OFF(MoveController2D))
    {
    }

    MoveController2D::MoveController2D(const std::string &name) : Behaviour(DESCRIBE_AS_ONLY_NAME(MoveController2D))
    {
        DESCRIBE_AS_MAIN(MoveController2D);
    }

    void MoveController2D::OnAwake()
    {
        playerCamera = game_object()->get_component<Camera2D>();
        if(playerCamera == nullptr)
            playerCamera = game_object()->add_component<Camera2D>();
        speed = 0.1f / 2;
    }
    Vec2 movementDir = Vec2::up;

    void MoveController2D::OnStart()
    {
    }

    void MoveController2D::OnUpdate()
    {

        float curSpeed = Input::key_down(KB_LSHIFT) ? (speed * 2) : speed;

        if(playerCamera)
        {
            auto cameraPoint = playerCamera->transform()->position();
            auto point = cameraPoint;
            Vec2 axis = Input::get_axis();

            if(axis != Vec2::zero && axis.x != axis.y)
            {
                if(axis.x - movementDir.x != 0 && axis.y - movementDir.y != 0)
                    movementDir = axis;
            }

            if(axis == Vec2::zero)
                return;
            movementDir = axis;
            // transform()->LookAtLerp(point + movementDir, .5f);
            transform()->position(Vec2::move_towards(point, point + movementDir, curSpeed));
        }
    }

    void MoveController2D::OnLateUpdate()
    {
    }
    void MoveController2D::OnDestroy()
    {
    }

} // namespace RoninEngine::Runtime
