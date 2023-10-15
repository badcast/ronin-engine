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
        playerCamera = gameObject()->GetComponent<Camera2D>();
        if(playerCamera == nullptr)
            playerCamera = gameObject()->AddComponent<Camera2D>();
        speed = 0.1f / 2;
    }
    Vec2 movementDir = Vec2::up;

    void MoveController2D::OnUpdate()
    {
        if(playerCamera)
        {
            float curSpeed = Input::GetKeyDown(KB_LSHIFT) ? (speed * 2) : speed;
            auto cameraPoint = playerCamera->transform()->position();
            auto point = cameraPoint;
            Vec2 axis = Input::GetAxis();

            if(axis != Vec2::zero && axis.x != axis.y)
            {
                if(axis.x - movementDir.x != 0 && axis.y - movementDir.y != 0)
                    movementDir = axis;
            }

            if(axis == Vec2::zero)
                return;
            movementDir = axis;
            transform()->position(Vec2::MoveTowards(point, point + movementDir, curSpeed));
        }
    }
} // namespace RoninEngine::Runtime
