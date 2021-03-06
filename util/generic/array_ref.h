#pragma once

#include "vector_ops.h"

#include <util/generic/utility.h>

#include <algorithm>
#include <initializer_list>

/**
 * Non-owning array ref, inspired by TStringBuf
 */
template <class T>
class TArrayRef: public NVectorOps::TVectorOps<T, TArrayRef<T>> {
public:
    inline TArrayRef() noexcept
        : T_(nullptr)
        , S_(0)
    {
    }

    inline TArrayRef(T* data, size_t len) noexcept
        : T_(data)
        , S_(len)
    {
    }

    inline TArrayRef(T* begin, T* end) noexcept
        : T_(begin)
        , S_(end - begin)
    {
    }

    inline TArrayRef(std::initializer_list<T> list) noexcept
        : T_(list.begin())
        , S_(list.size())
    {
    }

    template <class Container>
    inline TArrayRef(Container&& container, decltype(std::declval<T*&>() = container.data(), nullptr) = nullptr) noexcept
        : T_(container.data())
        , S_(container.size())
    {
    }

    template <size_t N>
    inline TArrayRef(T (&array)[N]) noexcept
        : T_(array)
        , S_(N)
    {
    }

    template <class TT, typename = std::enable_if_t<std::is_same<typename std::remove_const<T>::type, typename std::remove_const<TT>::type>::value>>
    bool operator==(const TArrayRef<TT>& other) const noexcept {
        return Size() == other.Size() && std::equal(this->Begin(), this->End(), other.Begin());
    }

    inline ~TArrayRef() = default;

    inline T* Data() const noexcept {
        return T_;
    }

    inline size_t Size() const noexcept {
        return S_;
    }

    inline void Swap(TArrayRef& a) noexcept {
        ::DoSwap(T_, a.T_);
        ::DoSwap(S_, a.S_);
    }

    /* STL compatibility. */

    inline T* data() const noexcept {
        return Data();
    }

    inline size_t size() const noexcept {
        return Size();
    }

    inline void swap(TArrayRef& a) noexcept {
        Swap(a);
    }

    TArrayRef<T> Slice(size_t offset) const {
        Y_ASSERT(offset <= size());
        return TArrayRef<T>(data() + offset, size() - offset);
    }

    TArrayRef<T> Slice(size_t offset, size_t size) {
        Y_ASSERT(offset + size <= this->size());

        return TArrayRef<T>(data() + offset, data() + offset + size);
    }

private:
    T* T_;
    size_t S_;
};

template <class T>
using TConstArrayRef = TArrayRef<const T>;

template <class Range>
TArrayRef<const typename Range::value_type> MakeArrayRef(const Range& range) {
    return TArrayRef<const typename Range::value_type>(range);
}

template <class Range>
TArrayRef<typename Range::value_type> MakeArrayRef(Range& range) {
    return TArrayRef<typename Range::value_type>(range);
}

template <class T>
TArrayRef<T> MakeArrayRef(T* data, size_t size) {
    return TArrayRef<T>(data, size);
}

/* Compatibility layer. TODO: DROP! */
namespace NArrayRef {
    using ::TArrayRef;
    using ::TConstArrayRef;
}
