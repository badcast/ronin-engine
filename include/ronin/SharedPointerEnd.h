#pragma once

#include "SharedPointer.h"

namespace RoninEngine::Runtime
{

    template<typename T, typename U>
    Ref<T> GetRef(U*)
    {
        // TODO: Get Shared Pointer
        T * v = nullptr;
        return Ref<T>{v};
    }

    template<typename T,typename U>
    Ref<T> ReinterpretCast(Ref<U> input)
    {
        Ref<T> result {};
        if((result.ptr_ = reinterpret_cast<T*>(input.ptr_)))
        {
            result.ref_count_ = input.ref_count_;
        }
        return result;
    }

    template<typename T,typename U>
    Ref<T> DynamicCast(Ref<U> input)
    {
        Ref<T> result {};
        if((result.ptr_ = dynamic_cast<T*>(input.ptr_)))
        {
            result.ref_count_ = input.ref_count_;
        }
        return result;
    }

    template<typename T,typename U>
    Ref<T> StaticCast(Ref<U> input)
    {
        Ref<T> result {};
        if((result.ptr_ = static_cast<T*>(input.ptr_)))
        {
            result.ref_count_ = input.ref_count_;
        }
        return result;
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

    template<typename T>
    Ref<T> &Ref<T>::operator=(T* && ptr)
    {
        release();
        if(ptr)
        {
            ptr_ = ptr;
            ref_count_ = new std::size_t(1);
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
            if(ref_count_)
                ++(*ref_count_);
        }
        return *this;
    }

    template<typename T>
    Ref<T> &Ref<T>::operator=(T* ptr)
    {
        release();
        if (ptr)
        {
            ptr_ = ptr;
            ref_count_ = new std::size_t(1);
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
    bool Ref<T>::operator!=(const Ref &rhs) const
    {
        return !(*this == rhs);
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
        return ptr_ != nullptr;
    }

    template <typename T>
    void Ref<T>::release()
    {
        if(ref_count_ && --ref_count_ == 0)
        {
            // TODO: Destroy

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
        // TODO: Get Shared Pointer
        T * v = nullptr;
        return Ref<T>{v};
    }
} // namespace RoninEngine::Runtime
