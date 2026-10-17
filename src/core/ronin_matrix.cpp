#include "ronin.h"
#include "ronin_matrix.h"
#include <algorithm>
#include <cmath>

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
                Vec2 offset = renderer->get_offset();
                Vec2 rotatedOffset = (angleRad != 0.0f) ? Vec2::Rotate(offset, -angleRad) : offset;
                Vec2 center = pos + rotatedOffset;
                float hw = size.x * 0.5f;
                float hh = size.y * 0.5f;
                float extX = std::max(hw * cosA + hh * sinA, hw);
                float extY = std::max(hw * sinA + hh * cosA, hh);
                minX = std::min({minX, center.x - extX, pos.x + offset.x - extX});
                minY = std::min({minY, center.y - extY, pos.y + offset.y - extY});
                maxX = std::max({maxX, center.x + extX, pos.x + offset.x + extX});
                maxY = std::max({maxY, center.y + extY, pos.y + offset.y + extY});
                hasBounds = true;
            }
            else if(auto collider = dynamic_cast<Collision*>(comp.ptr_))
            {
                Vec2 size = Vec2::Abs(collider->collideSize);
                float hw = size.x * 0.5f;
                float hh = size.y * 0.5f;
                float extX = std::max(hw * cosA + hh * sinA, size.x);
                float extY = std::max(hw * sinA + hh * cosA, size.y);
                minX = std::min(minX, pos.x - extX);
                minY = std::min(minY, pos.y - extY);
                maxX = std::max(maxX, pos.x + extX);
                maxY = std::max(maxY, pos.y + extY);
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

    QuadTree::QuadTree(int /*level*/, const Rectf& /*bounds*/)
    {
    }

    QuadTree::~QuadTree()
    {
        Clear();
    }

    void QuadTree::Clear()
    {
        entries.clear();
        cells.clear();
        largeObjects.clear();
    }

    void QuadTree::computeCellRange(const Rectf& rect, std::int32_t& minX, std::int32_t& minY, std::int32_t& maxX, std::int32_t& maxY, bool& isLarge) noexcept
    {
        float x0 = std::min(rect.x, rect.x + rect.w);
        float y0 = std::min(rect.y, rect.y + rect.h);
        float x1 = std::max(rect.x, rect.x + rect.w);
        float y1 = std::max(rect.y, rect.y + rect.h);

        minX = static_cast<std::int32_t>(std::floor(x0 * INV_CELL_SIZE));
        minY = static_cast<std::int32_t>(std::floor(y0 * INV_CELL_SIZE));
        maxX = static_cast<std::int32_t>(std::floor(x1 * INV_CELL_SIZE));
        maxY = static_cast<std::int32_t>(std::floor(y1 * INV_CELL_SIZE));

        std::int64_t spanX = static_cast<std::int64_t>(maxX) - static_cast<std::int64_t>(minX) + 1;
        std::int64_t spanY = static_cast<std::int64_t>(maxY) - static_cast<std::int64_t>(minY) + 1;
        isLarge = (spanX <= 0 || spanY <= 0 || spanX > MAX_CELL_SPAN || spanY > MAX_CELL_SPAN || (spanX * spanY) > MAX_CELL_SPAN);
    }

    void QuadTree::insertIntoCells(Transform* target, const ObjectEntry& entry)
    {
        if(entry.isLarge)
        {
            largeObjects.insert(target);
            return;
        }

        for(std::int32_t cy = entry.minY; cy <= entry.maxY; ++cy)
        {
            for(std::int32_t cx = entry.minX; cx <= entry.maxX; ++cx)
            {
                auto& bucket = cells[makeCellKey(cx, cy)];
                bucket.push_back(target);
            }
        }
    }

    void QuadTree::removeFromCells(Transform* target, const ObjectEntry& entry)
    {
        if(entry.isLarge)
        {
            largeObjects.erase(target);
            return;
        }

        for(std::int32_t cy = entry.minY; cy <= entry.maxY; ++cy)
        {
            for(std::int32_t cx = entry.minX; cx <= entry.maxX; ++cx)
            {
                std::uint64_t key = makeCellKey(cx, cy);
                auto cellIt = cells.find(key);
                if(cellIt != cells.end())
                {
                    auto& bucket = cellIt->second;
                    auto it = std::find(bucket.begin(), bucket.end(), target);
                    if(it != bucket.end())
                    {
                        *it = bucket.back();
                        bucket.pop_back();
                    }
                    if(bucket.empty())
                    {
                        cells.erase(cellIt);
                    }
                }
            }
        }
    }

    void QuadTree::Insert(Transform* target)
    {
        if(!target)
            return;

        ObjectEntry newEntry;
        newEntry.aabb = matrix_get_bounds(target);
        computeCellRange(newEntry.aabb, newEntry.minX, newEntry.minY, newEntry.maxX, newEntry.maxY, newEntry.isLarge);

        auto it = entries.find(target);
        if(it != entries.end())
        {
            ObjectEntry& oldEntry = it->second;
            if(oldEntry.isLarge == newEntry.isLarge &&
               (newEntry.isLarge ||
                (oldEntry.minX == newEntry.minX && oldEntry.minY == newEntry.minY &&
                 oldEntry.maxX == newEntry.maxX && oldEntry.maxY == newEntry.maxY)))
            {
                // Object stayed in the exact same grid cells: O(1) AABB update in-place
                oldEntry.aabb = newEntry.aabb;
                return;
            }

            removeFromCells(target, oldEntry);
            oldEntry = newEntry;
            insertIntoCells(target, oldEntry);
            return;
        }

        entries.emplace(target, newEntry);
        insertIntoCells(target, newEntry);
    }

    bool QuadTree::Remove(Transform* target)
    {
        if(!target)
            return false;

        auto it = entries.find(target);
        if(it == entries.end())
            return false;

        removeFromCells(target, it->second);
        entries.erase(it);
        return true;
    }

    bool QuadTree::Contains(Transform* target) const
    {
        return target != nullptr && entries.find(target) != entries.end();
    }

    void QuadTree::Retrieve(std::vector<Transform*>& returnObjects, const Rectf& pRect)
    {
        std::set<Transform*> uniqueSet;
        Retrieve(uniqueSet, pRect);
        returnObjects.insert(returnObjects.end(), uniqueSet.begin(), uniqueSet.end());
    }

    void QuadTree::Retrieve(std::set<Transform*>& returnObjects, const Rectf& pRect)
    {
        if(entries.empty())
            return;

        std::int32_t qMinX = 0, qMinY = 0, qMaxX = 0, qMaxY = 0;
        bool queryIsLarge = false;
        computeCellRange(pRect, qMinX, qMinY, qMaxX, qMaxY, queryIsLarge);

        std::int64_t spanX = static_cast<std::int64_t>(qMaxX) - static_cast<std::int64_t>(qMinX) + 1;
        std::int64_t spanY = static_cast<std::int64_t>(qMaxY) - static_cast<std::int64_t>(qMinY) + 1;
        std::int64_t queryCellCount = (spanX > 0 && spanY > 0 && spanX <= 65536 && spanY <= 65536) ? (spanX * spanY) : (static_cast<std::int64_t>(cells.size()) + 1);

        // If the query area spans more cells than exist in the world, scan entries directly
        if(static_cast<std::size_t>(queryCellCount) > cells.size())
        {
            for(const auto& pair : entries)
            {
                if(pair.second.aabb.Intersects(pRect))
                {
                    returnObjects.insert(pair.first);
                }
            }
            return;
        }

        for(Transform* obj : largeObjects)
        {
            auto it = entries.find(obj);
            if(it != entries.end() && it->second.aabb.Intersects(pRect))
            {
                returnObjects.insert(obj);
            }
        }

        for(std::int32_t cy = qMinY; cy <= qMaxY; ++cy)
        {
            for(std::int32_t cx = qMinX; cx <= qMaxX; ++cx)
            {
                auto cellIt = cells.find(makeCellKey(cx, cy));
                if(cellIt == cells.end())
                    continue;

                for(Transform* obj : cellIt->second)
                {
                    auto entryIt = entries.find(obj);
                    if(entryIt != entries.end() && entryIt->second.aabb.Intersects(pRect))
                    {
                        returnObjects.insert(obj);
                    }
                }
            }
        }
    }

    void QuadTree::GetAllObjects(std::set<Transform*>& out) const
    {
        for(const auto& pair : entries)
        {
            out.insert(pair.first);
        }
    }

    std::size_t QuadTree::CountAllObjects() const
    {
        return entries.size();
    }

    void matrix_update(Transform *target)
    {
        if(!currentWorld || !currentWorld->irs || !target || !target->_owner)
            return;

        if(!currentWorld->irs->mainObject.isNull() && currentWorld->irs->mainObject->transform().ptr_ == target)
            return;

        int layer = target->_owner->m_layer;
        auto& matrix = currentWorld->irs->matrix;

        auto it = matrix.find(layer);
        if(it == matrix.end() || it->second == nullptr)
        {
            matrix[layer] = new QuadTree();
            it = matrix.find(layer);
        }

        if(!it->second->Contains(target))
        {
            for(auto& pair : matrix)
            {
                if(pair.first != layer && pair.second)
                {
                    pair.second->Remove(target);
                }
            }
        }

        it->second->Insert(target);
    }

    bool matrix_remove(Transform *target)
    {
        if(!currentWorld || !currentWorld->irs || !target || !target->_owner)
            return false;
        auto& matrix = currentWorld->irs->matrix;
        auto it = matrix.find(target->_owner->m_layer);
        if (it != matrix.end() && it->second)
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
