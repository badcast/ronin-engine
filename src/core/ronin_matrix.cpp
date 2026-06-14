#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime::Matrix
{
    // AABB Calculation
    Rectf matrix_get_bounds(Transform* target)
    {
        Rectf bbox = {target->position().x, target->position().y, 0, 0};
        bool hasBounds = false;

        for(ComponentRef& comp : target->gameObject()->m_components)
        {
            if(!comp->_enable) continue;

            if(auto renderer = dynamic_cast<Renderer*>(comp.ptr_))
            {
                Vec2 size = renderer->getSize();
                // AABB is centered at position
                bbox.x = std::min(bbox.x, target->position().x - size.x);
                bbox.y = std::min(bbox.y, target->position().y - size.y);
                bbox.w = std::max(bbox.w, size.x * 2.f);
                bbox.h = std::max(bbox.h, size.y * 2.f);
                hasBounds = true;
            }
            else if(auto collider = dynamic_cast<Collision*>(comp.ptr_))
            {
                Vec2 size = collider->collideSize;
                Vec2 center = target->position();
                bbox.x = std::min(bbox.x, center.x - size.x);
                bbox.y = std::min(bbox.y, center.y - size.y);
                bbox.w = std::max(bbox.w, size.x * 2.f);
                bbox.h = std::max(bbox.h, size.y * 2.f);
                hasBounds = true;
            }
        }

        if(!hasBounds)
        {
            // Fallback for empty objects
            bbox.w = 0.1f;
            bbox.h = 0.1f;
        }

        return bbox;
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

    void matrix_update(Transform *target)
    {
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
        auto& matrix = currentWorld->irs->matrix;
        auto it = matrix.find(target->_owner->m_layer);
        if (it != matrix.end())
        {
            return it->second->Remove(target);
        }
        return false;
    }

    void matrix_nature(Transform *target)
    {
        matrix_update(target);
    }

    void matrix_get_intersects(const Rectf& rect, int layer, std::set<Transform*>& out)
    {
        auto& matrix = currentWorld->irs->matrix;
        auto it = matrix.find(layer);
        if (it != matrix.end())
        {
            it->second->Retrieve(out, rect);
        }
    }

    void matrix_get_intersects(const Rectf& rect, std::set<Transform*>& out)
    {
        auto& matrix = currentWorld->irs->matrix;
        for (auto& pair : matrix)
        {
            pair.second->Retrieve(out, rect);
        }
    }
} // namespace RoninEngine::Runtime::Matrix
