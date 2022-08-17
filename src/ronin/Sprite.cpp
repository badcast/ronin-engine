#include "framework.h"
#include "Sprite.h"
#include "Mathf.h"
#include "Texture.h"

Sprite* _empty = nullptr;

namespace RoninEngine::Runtime
{
    const RoninEngine::Runtime::Rect Sprite::rect() {
		return this->m_rect;
	}
    void Sprite::rect(RoninEngine::Runtime::Rect rect) {
		this->m_rect = rect;
	}

	void Sprite::Center(Vec2 center) {
		this->m_center = center;
	}
	const Vec2 Sprite::Center() {
		return m_center;
	}

	const bool Sprite::valid() {
		return !(m_rect.w - m_rect.x < 0 || m_rect.h - m_rect.y < 0 || !texture);
	}

	int Sprite::width() {
		return Mathf::max(0, m_rect.w - m_rect.x);
	}
	int Sprite::height() {
		return Mathf::max(0, m_rect.h - m_rect.y);
	}


	Sprite* Sprite::empty() {
		if (!_empty)
		{
			//todo: for empty sprite. The pixel 1x1 and fill solid color 
		}
		return _empty;
	}

}
