#pragma once
/*** PIMPL (pointer to implementation) ***/

template <typename Last, typename First>
inline First *pimpl_first(Last *ptr)
{
    return reinterpret_cast<First *>(ptr);
}

template <typename First, typename Last>
inline Last *pimpl_last(First *ptr)
{
    return reinterpret_cast<Last *>(ptr);
}
