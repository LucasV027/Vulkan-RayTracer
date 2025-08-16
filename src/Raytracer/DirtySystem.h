#pragma once

#include <bitset>

template <typename T, size_t N>
class DirtySystem {
public:
    bool IsDirty(T flag) const {
        return dirtyFlags.test(static_cast<size_t>(flag));
    }

    void SetDirty(T flag) const {
        dirtyFlags.set(static_cast<size_t>(flag));
    }

    void ClearDirty(T flag) const {
        dirtyFlags.reset(static_cast<size_t>(flag));
    }

    void ClearAllDirty() const {
        dirtyFlags.reset();
    }

    bool IsAnyDirty() const {
        return dirtyFlags.any();
    }

    void SetAllDirty() const {
        dirtyFlags.set();
    }

private:
    mutable std::bitset<N> dirtyFlags;
};

