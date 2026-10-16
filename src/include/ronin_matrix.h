#pragma once

#include "ronin.h"
#include <set>
#include <vector>

namespace RoninEngine::Runtime::Matrix
{
    class QuadTree
    {
    public:
        QuadTree(int level, const Rectf& bounds);
        ~QuadTree();

        void Clear();
        void Insert(Transform* target);
        bool Remove(Transform* target);
        
        // Return a list of all objects that could intersect the given bounds
        void Retrieve(std::vector<Transform*>& returnObjects, const Rectf& bounds);
        void Retrieve(std::set<Transform*>& returnObjects, const Rectf& bounds);
        
        const std::set<Transform*>& GetObjects() const { return objects; }
        void GetAllObjects(std::set<Transform*>& out) const;
        std::size_t CountAllObjects() const;

    private:
        void split();
        int getIndex(const Rectf& pRect);

        static const int MAX_OBJECTS = 10;
        static const int MAX_LEVELS = 5;

        int level;
        std::set<Transform*> objects;
        Rectf bounds;
        QuadTree* nodes[4];
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
