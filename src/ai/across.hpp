/**************************************************************
 *
 * ASTAR Algorithm composed by RoninEngine (Navigation AI)
 * date: 18.01.2023
 * author: badcast <lmecomposer@gmail.com>
 * C++11 minimum required
 *
 **************************************************************/

#ifndef _ACROSS_H_
#define _ACROSS_H_

#include <cstdint>
#include <list>
#include <cmath>
#include <cstring>
#include <stdexcept>

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

        int get_width();
        int get_height();

        TpNeuronMember* get(int x, int y);
        TpNeuronMember* get(const TpNeuronPoint& point);

        inline bool contains(const TpNeuronPoint& point);

        TpNeuronMember* front();
        TpNeuronMember* back();

        // pointer with Point
        inline bool get_nlocked(const TpNeuronPoint& point);
        inline std::uint8_t& get_nflag(const TpNeuronPoint& point);
        inline std::uint32_t& get_ncost(const TpNeuronPoint& point);
        inline std::uint32_t& get_nheuristic(const TpNeuronPoint& point);
        inline const int get_nweight(const TpNeuronPoint& point);
        inline const std::uint32_t get_ntotal(const TpNeuronPoint& point);
        inline const bool get_nempty(const TpNeuronPoint& point);
        void set_nlock(const TpNeuronPoint& point, const bool state);

        // pointer with pointer
        bool get_nlocked(const TpNeuronMember* neuron);
        std::uint8_t& get_nflag(const TpNeuronMember* neuron);
        std::uint32_t& get_ncost(const TpNeuronMember* neuron);
        std::uint32_t& get_nheuristic(const TpNeuronMember* neuron);
        const int get_nweight(const TpNeuronMember* neuron);
        const std::uint32_t get_ntotal(const TpNeuronMember* neuron);
        inline const bool get_nempty(const TpNeuronMember* neuron);
        void set_nlock(const TpNeuronMember* neuron, const bool state);

        inline const TpNeuronPoint get_npoint(const TpNeuronMember* neuron);

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
