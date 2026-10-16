#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime::Matrix
{
    // AABB Calculation
    Rectf matrix_get_bounds(Transform* target)
    {
        if(!target || !target->_owner)
            return Rectf(0.f, 0.f, 0.1f, 0.1f);

        Vec2 pos = target->position();
        float minX = pos.x;
        float minY = pos.y;
        float maxX = pos.x;
        float maxY = pos.y;
        bool hasBounds = false;

        float angleRad = target->angle() * Math::deg2rad;
        float cosA = std::fabs(std::cos(angleRad));
        float sinA = std::fabs(std::sin(angleRad));

        for(ComponentRef& comp : target->_owner->m_components)
        {
            if(!comp || !comp->_enable)
                continue;

            if(auto renderer = comp->get_renderer())
            {
                Vec2 size;
                if(auto sr = dynamic_cast<SpriteRenderer*>(renderer))
                {
                    size = (sr->getSprite() && currentWorld && currentWorld->irs)
                               ? Vec2::Scale(sr->getSprite()->size(), sr->getSize())
                               : sr->getSize();
                }
                else
                {
                    size = renderer->getSize();
                }
                size = Vec2::Abs(size);
                Vec2 center = pos + renderer->get_offset();
                float hw = size.x * 0.5f;
                float hh = size.y * 0.5f;
                float extX = std::max(hw * cosA + hh * sinA, size.x);
                float extY = std::max(hw * sinA + hh * cosA, size.y);
                minX = std::min(minX, center.x - extX);
                minY = std::min(minY, center.y - extY);
                maxX = std::max(maxX, center.x + extX);
                maxY = std::max(maxY, center.y + extY);
                hasBounds = true;
            }
            else if(auto collider = dynamic_cast<Collision*>(comp.ptr_))
            {
                Vec2 size = Vec2::Abs(collider->collideSize);
                minX = std::min(minX, pos.x - size.x);
                minY = std::min(minY, pos.y - size.y);
                maxX = std::max(maxX, pos.x + size.x);
                maxY = std::max(maxY, pos.y + size.y);
                hasBounds = true;
            }
            else if(auto light = comp->get_light_component())
            {
                float r = std::max(0.1f, light->range);
                minX = std::min(minX, pos.x - r);
                minY = std::min(minY, pos.y - r);
                maxX = std::max(maxX, pos.x + r);
                maxY = std::max(maxY, pos.y + r);
                hasBounds = true;
            }
        }

        if(!hasBounds || (maxX - minX < 0.1f && maxY - minY < 0.1f))
        {
            return Rectf(pos.x - 0.05f, pos.y - 0.05f, 0.1f, 0.1f);
        }

        return Rectf(minX, minY, maxX - minX, maxY - minY);
    }

    // QuadTree Implementation
    QuadTree::QuadTree(int level, const Rectf& bounds) : level(level), bounds(bounds)
    {
        for (int i = 0; i < 4; i++)
        {
            nodes[i] = nullptr;
        }
    }

    QuadTree::~QuadTree()
    {
        Clear();
    }

    void QuadTree::Clear()
    {
        objects.clear();
        for (int i = 0; i < 4; i++)
        {
            if (nodes[i] != nullptr)
            {
                nodes[i]->Clear();
                delete nodes[i];
                nodes[i] = nullptr;
            }
        }
    }

    void QuadTree::split()
    {
        float subWidth = bounds.w / 2.0f;
        float subHeight = bounds.h / 2.0f;
        float x = bounds.x;
        float y = bounds.y;

        nodes[0] = new QuadTree(level + 1, Rectf(x + subWidth, y, subWidth, subHeight));
        nodes[1] = new QuadTree(level + 1, Rectf(x, y, subWidth, subHeight));
        nodes[2] = new QuadTree(level + 1, Rectf(x, y + subHeight, subWidth, subHeight));
        nodes[3] = new QuadTree(level + 1, Rectf(x + subWidth, y + subHeight, subWidth, subHeight));
    }

    int QuadTree::getIndex(const Rectf& pRect)
    {
        int index = -1;
        double verticalMidpoint = bounds.x + (bounds.w / 2.0f);
        double horizontalMidpoint = bounds.y + (bounds.h / 2.0f);

        bool topQuadrant = (pRect.y < horizontalMidpoint && pRect.y + pRect.h < horizontalMidpoint);
        bool bottomQuadrant = (pRect.y > horizontalMidpoint);

        if (pRect.x < verticalMidpoint && pRect.x + pRect.w < verticalMidpoint)
        {
            if (topQuadrant)
            {
                index = 1;
            }
            else if (bottomQuadrant)
            {
                index = 2;
            }
        }
        else if (pRect.x > verticalMidpoint)
        {
            if (topQuadrant)
            {
                index = 0;
            }
            else if (bottomQuadrant)
            {
                index = 3;
            }
        }

        return index;
    }

    void QuadTree::Insert(Transform* target)
    {
        if (nodes[0] != nullptr)
        {
            int index = getIndex(matrix_get_bounds(target));

            if (index != -1)
            {
                nodes[index]->Insert(target);
                return;
            }
        }

        objects.insert(target);

        if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS)
        {
            if (nodes[0] == nullptr)
            {
                split();
            }

            auto i = objects.begin();
            while (i != objects.end())
            {
                int index = getIndex(matrix_get_bounds(*i));
                if (index != -1)
                {
                    nodes[index]->Insert(*i);
                    i = objects.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }

    bool QuadTree::Remove(Transform* target)
    {
        int index = getIndex(matrix_get_bounds(target));
        if (index != -1 && nodes[0] != nullptr)
        {
            if (nodes[index]->Remove(target)) return true;
        }

        auto it = objects.find(target);
        if (it != objects.end())
        {
            objects.erase(it);
            return true;
        }

        // Fallback: search all nodes if AABB was inaccurate during removal
        if (nodes[0] != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                if (nodes[i]->Remove(target)) return true;
            }
        }

        return false;
    }

    void QuadTree::Retrieve(std::vector<Transform*>& returnObjects, const Rectf& pRect)
    {
        int index = getIndex(pRect);
        if (index != -1 && nodes[0] != nullptr)
        {
            nodes[index]->Retrieve(returnObjects, pRect);
        }
        else if (nodes[0] != nullptr)
        {
            // Overlaps multiple quadrants
            for (int i = 0; i < 4; i++)
            {
                if (nodes[i]->bounds.Intersects(pRect))
                {
                    nodes[i]->Retrieve(returnObjects, pRect);
                }
            }
        }

        for (auto& obj : objects)
        {
            if (matrix_get_bounds(obj).Intersects(pRect))
            {
                returnObjects.push_back(obj);
            }
        }
    }

    void QuadTree::Retrieve(std::set<Transform*>& returnObjects, const Rectf& pRect)
    {
        int index = getIndex(pRect);
        if (index != -1 && nodes[0] != nullptr)
        {
            nodes[index]->Retrieve(returnObjects, pRect);
        }
        else if (nodes[0] != nullptr)
        {
            // Overlaps multiple quadrants
            for (int i = 0; i < 4; i++)
            {
                if (nodes[i]->bounds.Intersects(pRect))
                {
                    nodes[i]->Retrieve(returnObjects, pRect);
                }
            }
        }

        for (auto& obj : objects)
        {
            if (matrix_get_bounds(obj).Intersects(pRect))
            {
                returnObjects.insert(obj);
            }
        }
    }

    void QuadTree::GetAllObjects(std::set<Transform*>& out) const
    {
        out.insert(objects.begin(), objects.end());
        if (nodes[0] != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                if (nodes[i] != nullptr)
                {
                    nodes[i]->GetAllObjects(out);
                }
            }
        }
    }

    std::size_t QuadTree::CountAllObjects() const
    {
        std::size_t count = objects.size();
        if (nodes[0] != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                if (nodes[i] != nullptr)
                {
                    count += nodes[i]->CountAllObjects();
                }
            }
        }
        return count;
    }

    void matrix_update(Transform *target)
    {
        if(!currentWorld || !currentWorld->irs || !target || !target->_owner)
            return;
        matrix_remove(target);
        
        int layer = target->_owner->m_layer;
        auto& matrix = currentWorld->irs->matrix;
        
        if (matrix.find(layer) == matrix.end())
        {
            // Create root QuadTree with a large bound. It could be expanded dynamically, but fixed is easier for now.
            matrix[layer] = new QuadTree(0, Rectf(-100000.f, -100000.f, 200000.f, 200000.f));
        }
        
        matrix[layer]->Insert(target);
    }

    bool matrix_remove(Transform *target)
    {
        if(!currentWorld || !currentWorld->irs || !target || !target->_owner)
            return false;
        auto& matrix = currentWorld->irs->matrix;
        auto it = matrix.find(target->_owner->m_layer);
        if (it != matrix.end())
        {
            if (it->second->Remove(target))
                return true;
        }
        for (auto& pair : matrix)
        {
            if (pair.first != target->_owner->m_layer && pair.second && pair.second->Remove(target))
                return true;
        }
        return false;
    }

    void matrix_nature(Transform *target)
    {
        matrix_update(target);
    }

    void matrix_get_intersects(const Rectf& rect, int layer, std::set<Transform*>& out)
    {
        if(!currentWorld || !currentWorld->irs)
            return;
        auto& matrix = currentWorld->irs->matrix;
        auto it = matrix.find(layer);
        if (it != matrix.end() && it->second)
        {
            it->second->Retrieve(out, rect);
        }
    }

    void matrix_get_intersects(const Rectf& rect, std::set<Transform*>& out)
    {
        if(!currentWorld || !currentWorld->irs)
            return;
        auto& matrix = currentWorld->irs->matrix;
        for (auto& pair : matrix)
        {
            if (pair.second)
                pair.second->Retrieve(out, rect);
        }
    }
} // namespace RoninEngine::Runtime::Matrix
