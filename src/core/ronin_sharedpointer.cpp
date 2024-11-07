#include "ronin.h"
#include "SharedPointer.h"

using namespace RoninEngine::Runtime;

bool RoninPointer::isNull() const
{
    return true;
}

RoninPointer::operator bool() const
{
    return !isNull();
}

template<typename T>
SharedPointer<T>::SharedPointer() : ptr_(nullptr), ref_count_(nullptr)
{}

template<typename T>
SharedPointer<T>::SharedPointer(const T *ptr)
{
    if(ptr != nullptr)
    {
        ref_count_ = new size_t(1u);
        ptr_ = ptr;
    }
    else
    {
        ref_count_ = nullptr;
        ptr_ = nullptr;
    }
}

template<typename T>
SharedPointer<T>::SharedPointer(const SharedPointer &other) : ptr_(other.ptr_), ref_count_(other.ref_count_)
{
    (*ref_count_)++;
}

template<typename T>
SharedPointer<T>::SharedPointer(SharedPointer &&other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_)
{
    other.ptr_ = nullptr;
    other.ref_count_ = nullptr;
}

template<typename T>
SharedPointer<T>::~SharedPointer()
{
    release();
}

template<typename T>
T *SharedPointer<T>::get() const
{
    return ptr_;
}

template<typename T>
SharedPointer<T>& SharedPointer<T>::operator =(const SharedPointer<T>& other)
{
    if(this != &other)
    {
        release();

        ptr_ = other.ptr_;
        ref_count_ = other.ref_count_;

        ++(*ref_count_);
    }

    return *this;
}

template<typename T>
bool SharedPointer<T>::operator ==(const SharedPointer &rhs)
{
    return this->ptr_ == rhs.ptr_;
}

template<typename T>
bool SharedPointer<T>::operator !=(const SharedPointer &rhs)
{
    return !(*this == rhs);
}

template<typename T>
SharedPointer<T>& SharedPointer<T>::operator =(SharedPointer<T>&& other)
{
    if(this != &other)
    {
        release();

        ptr_ = other.ptr_;
        ref_count_ = other.ref_count_;
        other.ptr_ = nullptr;
        other.ref_count_ = nullptr;

        ++(*ref_count_);
    }

    return *this;
}

template<typename T>
T *SharedPointer<T>::operator->() const
{
    return ptr_;
}

template<typename T>
T &SharedPointer<T>::operator*() const
{
    return *ptr_;
}

template<typename T>
SharedPointer<T>::operator bool() const
{
    return ptr_ != nullptr;
}

template<typename T>
void SharedPointer<T>::release()
{
    if(ref_count_ && --ref_count_ == 0)
    {
        // TODO: Destroy

        delete ref_count_;
    }

    ptr_ = nullptr;
    ref_count_ = nullptr;
}

template<typename T>
bool SharedRoninObject<T>::isNull() const
{
    return this->ptr_ == nullptr || this->ptr_->isNull();
}
