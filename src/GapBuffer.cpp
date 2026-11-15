#include "GapBuffer.h"
#include <stdexcept>
#include <algorithm>

GapBuffer::GapBuffer(std::size_t initialCapacity)
    : buffer_(initialCapacity), gapStart_(0), gapEnd_(initialCapacity) {}

void GapBuffer::MoveGap(std::size_t targetIndex)
{
    const std::size_t currentLength = buffer_.size() - GapSize();

    if (targetIndex > currentLength)
    {
        throw std::out_of_range("Target index is out of range");
    }

    if (targetIndex < gapStart_)
    {
        const std::size_t distance = gapStart_ - targetIndex;

        std::move_backward(
            buffer_.begin() + targetIndex,
            buffer_.begin() + gapStart_,
            buffer_.begin() + gapEnd_
        );

        gapStart_ -= distance;
        gapEnd_ -= distance;
    }
    else if (targetIndex > gapStart_)
    {
        const std::size_t distance = targetIndex - gapStart_;

        std::move(
            buffer_.begin() + gapEnd_,
            buffer_.begin() + gapEnd_ + distance,
            buffer_.begin() + gapStart_
        );

        gapStart_ += distance;
        gapEnd_ += distance;
    }
}

void GapBuffer::ExpandGap(std::size_t requiredSize)
{
    const std::size_t currentGapSize = GapSize();

    if (currentGapSize >= requiredSize)
    {
        return;
    }

    const std::size_t requiredAdditionalSize =
        requiredSize - currentGapSize;

    const std::size_t availableAdditionalSize =
        buffer_.max_size() - buffer_.size();

    if (requiredAdditionalSize > availableAdditionalSize)
    {
        throw std::length_error("GapBuffer capacity exceeded");
    }

// We guarantee that the expansion is at least the required size,
// at least the current buffer size (double the capacity), and a minimum of 16 bytes.
    const std::size_t minGrowth = std::max<std::size_t>(16, buffer_.size());
    const std::size_t additionalSize = std::max(requiredAdditionalSize, minGrowth);

    const std::size_t textAfterGap =
        buffer_.size() - gapEnd_;

    buffer_.resize(buffer_.size() + additionalSize);

    std::move_backward(
        buffer_.begin() + gapEnd_,
        buffer_.begin() + gapEnd_ + textAfterGap,
        buffer_.end()
    );

    gapEnd_ += additionalSize;
}

std::size_t GapBuffer::GapSize() const noexcept
{
    return gapEnd_ - gapStart_;
}

void GapBuffer::Delete(std::size_t index, std::size_t length)
{
    const std::size_t currentLength = buffer_.size() - GapSize();

    if (index > currentLength || length > currentLength - index)
    {
        throw std::out_of_range("Delete range is out of bounds");
    }

    if (length == 0)
    {
        return;
    }

    MoveGap(index);

    gapEnd_ += length;
}

void GapBuffer::Replace(std::size_t index, std::size_t length, std::string_view text)
{
    const std::size_t currentLength = buffer_.size() - GapSize();

    if (index > currentLength || length > currentLength - index)
    {
        throw std::out_of_range("Replace range is out of bounds");
    }

    if (length == 0 && text.empty())
    {
        return;
    }

    MoveGap(index);

    // Delete the specified range
    gapEnd_ += length;

    // Insert the new text
    ExpandGap(text.size());

    for (char ch : text)
    {
        buffer_[gapStart_] = ch;
        ++gapStart_;
    }
}

std::size_t GapBuffer::Length() const noexcept
{
    return buffer_.size() - GapSize();
}

std::string GapBuffer::GetText() const
{
    std::string result;
    result.reserve(Length());

    result.append(buffer_.begin(), buffer_.begin() + gapStart_);
    result.append(buffer_.begin() + gapEnd_, buffer_.end());

    return result;
}

std::string GapBuffer::GetRange(std::size_t index, std::size_t length) const
{
    const std::size_t currentLength = buffer_.size() - GapSize();

    if (index > currentLength || length > currentLength - index)
    {
        throw std::out_of_range("GetRange range is out of bounds");
    }

    if (length == 0)
    {
        return {};
    }

    std::string result;
    result.reserve(length);

    if (index < gapStart_ && length <= gapStart_ - index)
    {
        // Entire range is before the gap
        result.append(buffer_.begin() + index, buffer_.begin() + index + length);
    }
    else if (index >= gapStart_)
    {
        // Entire range is after the gap
        result.append(
            buffer_.begin() + index + GapSize(),
            buffer_.begin() + index + GapSize() + length
        );
    }
    else
    {
        // Range spans the gap
        const std::size_t lengthBeforeGap = gapStart_ - index;
        const std::size_t lengthAfterGap = length - lengthBeforeGap;

        result.append(buffer_.begin() + index, buffer_.begin() + gapStart_);
        result.append(buffer_.begin() + gapEnd_, buffer_.begin() + gapEnd_ + lengthAfterGap);
    }

    return result;
}

void GapBuffer::Insert(std::size_t index, std::string_view text)
{
    const std::size_t currentLength = buffer_.size() - GapSize();

    if (index > currentLength)
    {
        throw std::out_of_range("Insert index is out of bounds");
    }

    if (text.empty())
    {
        return;
    }

    MoveGap(index);

    ExpandGap(text.size());

    for (char ch : text)
    {
        buffer_[gapStart_] = ch;
        ++gapStart_;
    }
}