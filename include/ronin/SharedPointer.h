#ifndef _RONIN_SHARED_POINTER_H_
#define _RONIN_SHARED_POINTER_H_ 1

#include <cstdint>
#include <type_traits>
#include "Defines.h"

namespace RoninEngine::Runtime
{
    class RONIN_API RoninPointer
    {
    public:
        RoninPointer() = default;
        virtual ~RoninPointer() = default;

        bool isNull() const;

        operator bool() const;
    };

    template<typename T>
    class RONIN_API SharedPointer
    {
    public:
        SharedPointer();
        explicit SharedPointer(T* ptr);
        explicit SharedPointer(const T* ptr);
        SharedPointer(const SharedPointer & other);
        SharedPointer(SharedPointer && other) noexcept;
        ~SharedPointer();

        T* get() const;

        SharedPointer& operator =(const SharedPointer& other);
        SharedPointer& operator =(SharedPointer&& other);
        bool operator==(const SharedPointer &rhs);
        bool operator!=(const SharedPointer &rhs);
        T *operator->() const;
        T &operator*() const;
        operator bool() const;

    protected:
        T* ptr_;
        std::size_t* ref_count_;

        void release();
    };

    template<typename T>
    class RONIN_API SharedRoninObject : public SharedPointer<T>
    {
    public:
        //static_assert(std::is_base_of_v<RoninPointer, std::remove_pointer_t<T>>, "T is must be RoninPointer");

        // C++ owner by Base Constructor
        using SharedPointer<T>::SharedPointer;
        using ConstType = SharedRoninObject<const T>;

        bool isNull() const;

        template<typename X>
        SharedRoninObject<X> StaticCast();

        template<typename X>
        SharedRoninObject<X> DynamicCast();
    };

    template<typename T>
    template<typename X>
    inline SharedRoninObject<X> SharedRoninObject<T>::DynamicCast()
    {
        SharedRoninObject<X> result {};
        if((result.ptr_ = dynamic_cast<X*>(this->ptr_)) != nullptr)
        {
            result.ref_count_ = this->ref_count_;
            // Add ref
            (*result.ref_count_)++;
        }
        return result;
    }

    template<typename T>
    template<typename X>
    inline SharedRoninObject<X> SharedRoninObject<T>::StaticCast()
    {
        SharedRoninObject<X> result {};
        if((result.ptr_ = static_cast<X*>(this->ptr_)))
        {
            result.ref_count_ = this->ref_count_;
            (*result.ref_count_)++;
        }
        return result;
    }

    template<typename T>
    struct Bushido_t
    {
        using type = SharedRoninObject<std::conditional<std::is_pointer_v<T>, std::remove_pointer_t<T>, T>>;
    };

    template<typename T>
    using Bushido = typename Bushido_t<T>::type;

           //template <typename _Base, typename _Derived>
           //inline constexpr bool is_base_of_v = __is_base_of(_Base, _Derived);

}

#endif
