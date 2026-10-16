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
            retval.ptr_->_ref_count_intrusive++;
        }
        return retval;
    }

    template<typename T,typename U>
    Ref<T> DynamicCast(Ref<U> input)
    {
        Ref<T> retval;
        if((retval.ptr_ = dynamic_cast<T*>(input.ptr_)))
        {
            retval.ptr_->_ref_count_intrusive++;
        }
        return retval;
    }

    template<typename T,typename U>
    Ref<T> StaticCast(Ref<U> input)
    {
        Ref<T> retval;
        if((retval.ptr_ = static_cast<T*>(input.ptr_)))
        {
            retval.ptr_->_ref_count_intrusive++;
        }
        return retval;
    }

    template <typename T>
    Ref<T>::Ref() noexcept : ptr_(nullptr)
    {
    }

    template <typename T>
    Ref<T>::Ref(std::nullptr_t) noexcept : ptr_(nullptr)
    {
    }

    template <typename T>
    Ref<T>::Ref(const Ref &other) noexcept : ptr_(other.ptr_)
    {
        if(ptr_)
            ptr_->_ref_count_intrusive++;
    }

    template <typename T>
    Ref<T>::Ref(Ref &&other) noexcept : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    template<typename T>
    Ref<T>::Ref(T* ptr) noexcept : ptr_(ptr)
    {
        if(ptr_)
        {
            ptr_->_ref_count_intrusive++;
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
            if(ptr_)
                ptr_->_ref_count_intrusive++;
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
            other.ptr_ = nullptr;
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
        if(ptr_ && !(--(ptr_->_ref_count_intrusive)))
        {
            RoninPointer::instance_end(ptr_);
        }

        ptr_ = nullptr;
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

    template<typename T, typename U>
    Ref<T> GetRefMain(U *ptr)
    {
        if(!ptr)
            return nullptr;
        return ptr->template GetRef<T>();
    }
} // namespace RoninEngine::Runtime
