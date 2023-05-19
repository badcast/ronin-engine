#include "ronin.h"
namespace RoninEngine::Runtime
{

    Player::Player()
        : Player(DESCRIBE_TYPE(Player,this, &t))
    {
    }

    Player::Player(const std::string& name)
        : Behaviour(name)
    {
    }

    void Player::OnAwake()
    {
        playerCamera = gameObject()->get_component<Camera2D>();
        if (playerCamera == nullptr)
            playerCamera = gameObject()->add_component<Camera2D>();
        //spriteRenderer = gameObject()->add_component<SpriteRenderer>();
        speed = 0.1f / 2;
        // spotlight = gameObject()->add_component<Spotlight>();
    }
    Vec2 movementDir = Vec2::up;

    void Player::OnStart() { }

    void Player::OnUpdate()
    {

        float curSpeed = input::get_key(SDL_SCANCODE_LSHIFT) ? (speed * 2) : speed;

        if (playerCamera) {
            auto cameraPoint = playerCamera->transform()->position();
            auto point = cameraPoint;
            Vec2 axis = input::get_axis();

            if (axis != Vec2::zero && axis.x != axis.y) {
                if (axis.x - movementDir.x != 0 && axis.y - movementDir.y != 0)
                    movementDir = axis;
            }

            if (axis == Vec2::zero)
                return;
            movementDir = axis;
            // transform()->LookAtLerp(point + movementDir, .5f);
            transform()->position(Vec2::MoveTowards(point, point + movementDir, curSpeed));
        }
    }

    void Player::OnLateUpdate() { }
    void Player::OnDestroy() { }

} // namespace RoninEngine::Runtime
