/**************************************************************
 *
 * ASTAR Algorithm composed by RoninEngine (Navigation AI)
 * date: 18.01.2023
 * author: badcast <lmecomposer@gmail.com>
 * C++11 minimum required
 *
 **************************************************************/
#include <cstdint>
#include <list>
#include <cmath>
#include <cstring>
#include <stdexcept>

#ifndef _ACROSS_H_
#define _ACROSS_H_

namespace across
{
    enum class NavStatus { Undefined, Locked, Closed, Opened };

    enum class NavMethodRule { NavigationIntelegency, PlusMethod, SquareMethod, CrossMethod };

    struct NeuronPoint {
        int x;
        int y;
    };

    bool operator==(const NeuronPoint& a, const NeuronPoint& b);

    bool operator!=(const NeuronPoint& a, const NeuronPoint& b);

    struct NeuronMember {
        std::uint8_t flags;
        std::uint32_t h;
        std::uint32_t cost;
    };

    struct AcrossData {
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
        void* neurons;
    };

    template <typename AcrossMapType>
    struct NavResult {
        NavStatus status;
        typename AcrossMapType::TypePoint firstNeuron;
        typename AcrossMapType::TypePoint lastNeuron;
        std::list<typename AcrossMapType::TypePoint> RelativePaths;
        const AcrossMapType* map;
    };

    template <typename TpNeuronPoint = NeuronPoint, typename TpNeuronMember = NeuronMember>
    class basic_across_map
    {
    protected:
        int segmentOffset;
        TpNeuronMember* neurons;
        int widthSpace, heightSpace;

    public:
        typedef TpNeuronMember TypeNeuron;
        typedef TpNeuronPoint TypePoint;
        typedef NavResult<basic_across_map> NavigateionResult;

        explicit basic_across_map(int width, int height);

        ~basic_across_map();

        void clear(bool clearLocks = false);
        void fill(bool fillLocks = false);
        void randomize(int flagFilter = 0xdeadbef);
        void stress();

        int getWidth();
        int getHeight();

        TpNeuronMember* get(int x, int y);
        TpNeuronMember* get(const TpNeuronPoint& point);

        inline bool contains(const TpNeuronPoint& point);

        inline const TpNeuronPoint get_point(const TpNeuronMember* neuron);

        TpNeuronMember* front();
        TpNeuronMember* back();

        // pointer with Point
        inline bool neuronLocked(const TpNeuronPoint& point);
        inline std::uint8_t& neuronGetFlag(const TpNeuronPoint& point);
        inline std::uint32_t& neuronGetCost(const TpNeuronPoint& point);
        inline std::uint32_t& neuronHeuristic(const TpNeuronPoint& point);
        inline const int neuronGetWeight(const TpNeuronPoint& point);
        inline const std::uint32_t neuronGetTotal(const TpNeuronPoint& point);
        inline const bool neuronEmpty(const TpNeuronPoint& point);
        void neuronLock(const TpNeuronPoint& point, const bool state);

        // pointer with pointer
        bool neuronLocked(const TpNeuronMember* neuron);
        std::uint8_t& neuronGetFlag(const TpNeuronMember* neuron);
        std::uint32_t& neuronGetCost(const TpNeuronMember* neuron);
        std::uint32_t& neuronHeuristic(const TpNeuronMember* neuron);
        const int neuronGetWeight(const TpNeuronMember* neuron);
        const std::uint32_t neuronGetTotal(const TpNeuronMember* neuron);
        inline const bool neuronEmpty(const TpNeuronMember* neuron);
        void neuronLock(const TpNeuronMember* neuron, const bool state);

        void find(NavigateionResult& navResult, NavMethodRule method, TpNeuronMember* firstNeuron, TpNeuronMember* lastNeuron);

        void find(NavigateionResult& navResult, NavMethodRule method, TpNeuronPoint first, TpNeuronPoint last);

        void load(const AcrossData& AcrossData);

        void save(AcrossData* AcrossData);

        std::uint32_t get_cached_size();
    };

#include "across_impl.hpp"

    typedef basic_across_map<NeuronPoint, NeuronMember> AcrossMap;


}

#endif
