#ifndef _RONIN_SHARED_POINTER_H_
#define _RONIN_SHARED_POINTER_H_ 1

#include "Defines.h"

namespace RoninEngine
{
    namespace Runtime
    {
        template<typename T, typename U>
        Ref<T> GetRefMain(U*);
        template <typename T, typename U>
        Ref<T> StaticCast(Ref<U>);
        template <typename T, typename U>
        Ref<T> DynamicCast(Ref<U>);
        template <typename T, typename U>
        Ref<T> ReinterpretCastUnsafe(Ref<U>);

        class RONIN_API RoninPointer
        {
            template<typename T>
            friend class Ref;
        protected:
            int _handle;
            static void instance_end(RoninPointer*);
            static Ref<RoninPointer> GetRefMain(RoninPointer*);
            static void RefRegister(Ref<RoninPointer>);
        public:
            RoninPointer();
            virtual ~RoninPointer();
            bool isNull() const;

            operator bool() const;

            template<typename T>
            Ref<T> GetRef();
            Ref<RoninPointer> GetRef();
        };

        template <typename T>
        class RONIN_API Ref
        {
            template<typename A,typename B>
            friend Ref<A> StaticCast(Ref<B>);
            template<typename A,typename B>
            friend Ref<A> DynamicCast(Ref<B>);
            template<typename A,typename B>
            friend Ref<A> ReinterpretCastUnsafe(Ref<B>);
        public:
            Ref() noexcept;
            Ref(std::nullptr_t) noexcept;
            Ref(const Ref &other) noexcept;
            Ref(Ref &&other) noexcept;
            explicit Ref(T *ptr) noexcept;
            virtual ~Ref();
            bool isNull() const;
            T *get() const;
            Ref &operator=(const Ref &other);
            Ref &operator=(Ref &&other);
            Ref &operator=(std::nullptr_t);
            bool operator==(const Ref &rhs) const;
            bool operator==(Ref &&rhs) const;
            bool operator==(std::nullptr_t) const;
            bool operator!=(const Ref &rhs) const;
            bool operator!=(Ref &&rhs) const;
            bool operator!=(std::nullptr_t) const;
            bool operator<(const Ref &rhs) const;
            bool operator<(Ref &&rhs) const;
            bool operator>(const Ref &rhs) const;
            bool operator>(Ref &&rhs) const;
            T *operator->() const;
            T &operator*() const;
            explicit operator bool() const;
        private:
            T *ptr_;
            std::size_t *ref_count_;
            void release();
        };

    } // namespace Runtime
} // namespace RoninEngine
#endif
