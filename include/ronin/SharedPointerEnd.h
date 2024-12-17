#pragma once

#include "SharedPointer.h"

namespace RoninEngine::Runtime
{
    template<typename T,typename U>
    Ref<T> ReinterpretCastUnsafe(Ref<U> input)
    {
        Ref<T> retval;
        if((retval.ptr_ = reinterpret_cast<T*>(input.ptr_)))
        {
            retval.ref_count_ = input.ref_count_;
            if(retval.ref_count_)
                (*retval.ref_count_)++;
        }
        return retval;
    }

    template<typename T,typename U>
    Ref<T> DynamicCast(Ref<U> input)
    {
        Ref<T> retval;
        if((retval.ptr_ = dynamic_cast<T*>(input.ptr_)))
        {
            retval.ref_count_ = input.ref_count_;
            if(retval.ref_count_)
                (*retval.ref_count_)++;
        }
        return retval;
    }

    template<typename T,typename U>
    Ref<T> StaticCast(Ref<U> input)
    {
        Ref<T> retval;
        if((retval.ptr_ = static_cast<T*>(input.ptr_)))
        {
            retval.ref_count_ = input.ref_count_;
            if(retval.ref_count_)
                (*retval.ref_count_)++;
        }
        return retval;
    }

    template <typename T>
    Ref<T>::Ref() noexcept : ptr_(nullptr), ref_count_(nullptr)
    {
    }

    template <typename T>
    Ref<T>::Ref(std::nullptr_t) noexcept : ptr_(nullptr), ref_count_(nullptr)
    {
    }

    template <typename T>
    Ref<T>::Ref(const Ref &other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_)
    {
        if(ref_count_)
            (*ref_count_)++;
    }

    template <typename T>
    Ref<T>::Ref(Ref &&other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_)
    {
        other.ptr_ = nullptr;
        other.ref_count_ = nullptr;
    }

    template<typename T>
    Ref<T>::Ref(T* ptr) noexcept : ptr_(ptr)
    {
        if(ptr)
        {
            ref_count_ = new std::size_t(1);
        }
    }

    template <typename T>
    Ref<T>::~Ref()
    {
        release();
    }

    template <typename T>
    T *Ref<T>::get() const
    {
        return ptr_;
    }

    template <typename T>
    Ref<T> &Ref<T>::operator=(const Ref &other)
    {
        if(this != &other)
        {
            release();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            if(ref_count_)
                ++(*ref_count_);
        }
        return *this;
    }

    template <typename T>
    Ref<T> &Ref<T>::operator=(Ref &&other)
    {
        if(this != &other)
        {
            release();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            other.ptr_ = nullptr;
            other.ref_count_ = nullptr;
        }
        return *this;
    }

    template<typename T>
    Ref<T> &Ref<T>::operator=(std::nullptr_t)
    {
        release();
        return *this;
    }

    template <typename T>
    bool Ref<T>::operator==(const Ref &rhs) const
    {
        return this->ptr_ == rhs.ptr_;
    }

    template <typename T>
    bool Ref<T>::operator==(Ref &&rhs) const
    {
        return this->ptr_ == rhs.ptr_;
    }

    template<typename T>
    bool Ref<T>::operator==(std::nullptr_t) const
    {
        return this->isNull();
    }

    template <typename T>
    bool Ref<T>::operator!=(const Ref &rhs) const
    {
        return !(*this == rhs);
    }

    template <typename T>
    bool Ref<T>::operator!=(Ref &&rhs) const
    {
        return !(*this == rhs);
    }

    template<typename T>
    bool Ref<T>::operator !=(std::nullptr_t) const
    {
        return !this->isNull();
    }

    template <typename T>
    bool Ref<T>::operator<(const Ref &rhs) const
    {
        return this->ptr_ < rhs.ptr_;
    }

    template <typename T>
    bool Ref<T>::operator<(Ref &&rhs) const
    {
        return this->ptr_ < rhs.ptr_;
    }

    template <typename T>
    bool Ref<T>::operator>(const Ref &rhs) const
    {
        return this->ptr_ > rhs.ptr_;
    }

    template <typename T>
    bool Ref<T>::operator>(Ref &&rhs) const
    {
        return this->ptr_ > rhs.ptr_;
    }

    template <typename T>
    T *Ref<T>::operator->() const
    {
        return ptr_;
    }

    template <typename T>
    T &Ref<T>::operator*() const
    {
        return *ptr_;
    }

    template <typename T>
    Ref<T>::operator bool() const
    {
        return !isNull();
    }

    template <typename T>
    void Ref<T>::release()
    {
        if(ref_count_ && !(--(*ref_count_)))
        {
            if constexpr (std::is_base_of_v<RoninPointer, T>)
            {
                RoninPointer::instance_end(ptr_);
            }
            delete ref_count_;
        }

        ptr_ = nullptr;
        ref_count_ = nullptr;
    }

    template <typename T>
    bool Ref<T>::isNull() const
    {
        return this->ptr_ == nullptr || this->ptr_->isNull();
    }

    template<typename T>
    Ref<T> RoninPointer::GetRef()
    {
        Ref<RoninPointer> ref = GetRefMain(static_cast<RoninPointer*>(this));
        return DynamicCast<T>(ref);
    }
} // namespace RoninEngine::Runtime
