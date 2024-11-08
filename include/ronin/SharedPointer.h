#ifndef _RONIN_SHARED_POINTER_H_
#define _RONIN_SHARED_POINTER_H_ 1

#include <cstddef>
#include "Defines.h"
#include "Declarations.h"

namespace RoninEngine
{
    namespace Runtime
    {
        template<typename T, typename U>
        Ref<T> GetRef(U*);

        template <typename T, typename U>
        Ref<T> StaticCast(Ref<U>);
        template <typename T, typename U>
        Ref<T> DynamicCast(Ref<U>);
        template <typename T, typename U>
        Ref<T> ReinterpretCast(Ref<U>);

        class RONIN_API RoninPointer
        {
        protected:
            void* _handle;

        public:
            RoninPointer();
            virtual ~RoninPointer();
            bool isNull() const;
            operator bool() const;

            template<typename T>
            Ref<T> GetRef();
        };

        template <typename T>
        class RONIN_API Ref
        {
            template <typename U>
            friend Ref<T> StaticCast(Ref<U>);
            template <typename U>
            friend Ref<T> DynamicCast(Ref<U>);
            template <typename U>
            friend Ref<T> ReinterpretCast(Ref<U>);

        public:
            Ref() noexcept;
            Ref(std::nullptr_t) noexcept;
            Ref(const Ref &other) noexcept;
            Ref(Ref &&other) noexcept;
            explicit Ref(T *ptr) noexcept;
            ~Ref();

            bool isNull() const;

            T *get() const;

            Ref &operator=(const Ref &other);
            Ref &operator=(Ref &&other);
            Ref &operator=(T* ptr);
            Ref &operator=(T*&& ptr);
            Ref &operator=(std::nullptr_t);
            bool operator==(const Ref &rhs) const;
            bool operator!=(const Ref &rhs) const;
            T *operator->() const;
            T &operator*() const;
            operator bool() const;

        private:
            T *ptr_;
            std::size_t *ref_count_;

            void release();
        };

    } // namespace Runtime
} // namespace RoninEngine

#include "SharedPointerEnd.h"
#endif
