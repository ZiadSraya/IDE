#pragma once
#include "ITextBuffer.h"
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// Contiguous array-backed text buffer maintaining an empty gap at the cursor position.
// Provides O(K) localized Insert (for K inserted bytes) and O(1) Delete once positioned at the gap,
// while moving the gap takes O(|delta|).
class GapBuffer : public ITextBuffer {
private:
    std::vector<char> buffer_;

    // Invariant: 0 <= gapStart_ <= gapEnd_ <= buffer_.size()
    // Active text length is always buffer_.size() - (gapEnd_ - gapStart_)
    std::size_t gapStart_;
    std::size_t gapEnd_;

    std::size_t GapSize() const noexcept;
    void MoveGap(std::size_t targetIndex);
    void ExpandGap(std::size_t requiredSize);

public:
    // Initializes the buffer with a pre-allocated gap to reduce early reallocations.
    explicit GapBuffer(std::size_t initialCapacity = 16);

    void Insert(std::size_t index, std::string_view text) override;
    void Delete(std::size_t index, std::size_t length) override;
    void Replace(std::size_t index, std::size_t length, std::string_view text) override;

    std::string GetText() const override;
    std::string GetRange(std::size_t index, std::size_t length) const override;
    std::size_t Length() const noexcept override;
};
