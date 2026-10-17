#pragma once

#include "ronin.h"
#include <cstdint>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RoninEngine::Runtime::Matrix
{
    class QuadTree
    {
    public:
        struct ObjectEntry
        {
            Rectf aabb;
            std::int32_t minX;
            std::int32_t minY;
            std::int32_t maxX;
            std::int32_t maxY;
            bool isLarge;
        };

        QuadTree(int level = 0, const Rectf& bounds = Rectf());
        ~QuadTree();

        void Clear();
        void Insert(Transform* target);
        bool Remove(Transform* target);
        bool Contains(Transform* target) const;

        // Return a list of all objects that could intersect the given bounds
        void Retrieve(std::vector<Transform*>& returnObjects, const Rectf& bounds);
        void Retrieve(std::set<Transform*>& returnObjects, const Rectf& bounds);

        void GetAllObjects(std::set<Transform*>& out) const;
        std::size_t CountAllObjects() const;

    private:
        static constexpr float CELL_SIZE = 16.0f;
        static constexpr float INV_CELL_SIZE = 1.0f / CELL_SIZE;
        static constexpr std::int64_t MAX_CELL_SPAN = 64;

        static inline std::uint64_t makeCellKey(std::int32_t cx, std::int32_t cy) noexcept
        {
            return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) << 32) |
                   static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
        }

        static void computeCellRange(const Rectf& rect, std::int32_t& minX, std::int32_t& minY, std::int32_t& maxX, std::int32_t& maxY, bool& isLarge) noexcept;
        void insertIntoCells(Transform* target, const ObjectEntry& entry);
        void removeFromCells(Transform* target, const ObjectEntry& entry);

        std::unordered_map<Transform*, ObjectEntry> entries;
        std::unordered_map<std::uint64_t, std::vector<Transform*>> cells;
        std::unordered_set<Transform*> largeObjects;
    };

    // Calculate AABB for a Transform based on attached components
    Rectf matrix_get_bounds(Transform* target);

    void matrix_update(Transform *target);
    bool matrix_remove(Transform *target);

    // Additional matrix queries
    void matrix_nature(Transform *target);

    void matrix_get_intersects(const Rectf& rect, int layer, std::set<Transform*>& out);
    void matrix_get_intersects(const Rectf& rect, std::set<Transform*>& out);

} // namespace RoninEngine::Runtime::Matrix
