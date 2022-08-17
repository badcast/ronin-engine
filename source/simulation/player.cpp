#include "ronin.h"
namespace RoninEngine::Runtime {

void Player::OnAwake() {
    playerCamera = gameObject()->addComponent<Camera2D>();
    spriteRenderer = gameObject()->addComponent<SpriteRenderer>();
    spotlight = gameObject()->addComponent<Spotlight>();
}
Vec2 movementDir = Vec2::up;

void Player::OnStart() { speed = 0.1f/2; }

void Player::OnUpdate() {
    float curSpeed = input::get_key(SDL_SCANCODE_LSHIFT) ? (speed * 2) : speed;

    if (playerCamera) {
        auto cameraPoint = playerCamera->transform()->position();
        auto point = cameraPoint;
        Vec2 axis = input::get_axis();

        if (axis != Vec2::zero && axis.x != axis.y) {
            if (axis.x - movementDir.x != 0 && axis.y - movementDir.y != 0) movementDir = axis;
        }

        if (axis == Vec2::zero) return;
        movementDir = axis;
        //transform()->LookAtLerp(point + movementDir, .5f);
        transform()->position(Vec2::MoveTowards(point, point + movementDir, curSpeed));
    }
}

void Player::OnLateUpdate() {}
void Player::OnDestroy() {}

}  // namespace RoninEngine::Runtime
