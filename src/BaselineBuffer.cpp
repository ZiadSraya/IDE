#include "BaselineBuffer.h"
#include <stdexcept>

BaselineBuffer::BaselineBuffer(std::string_view initial_text)
    : buffer_(initial_text) {}

void BaselineBuffer::Insert(std::size_t index, std::string_view text) {
    if (index > buffer_.size()) {
        throw std::out_of_range("Insert index is out of bounds.");
    }
    if (text.empty()) {
        return;
    }
    buffer_.insert(index, text);
}

void BaselineBuffer::Delete(std::size_t index, std::size_t length) {
    if (index > buffer_.size() || length > buffer_.size() - index) {
        throw std::out_of_range("Delete range is out of bounds.");
    }
    if (length == 0) {
        return;
    }
    buffer_.erase(index, length);
}

void BaselineBuffer::Replace(std::size_t index, std::size_t length, std::string_view text) {
    if (index > buffer_.size() || length > buffer_.size() - index) {
        throw std::out_of_range("Replace range is out of bounds.");
    }
    buffer_.replace(index, length, text);
}

std::string BaselineBuffer::GetText() const {
    return buffer_;
}

std::string BaselineBuffer::GetRange(std::size_t index, std::size_t length) const {
    if (index > buffer_.size() || length > buffer_.size() - index) {
        throw std::out_of_range("GetRange parameters are out of bounds.");
    }
    return buffer_.substr(index, length);
}

std::size_t BaselineBuffer::Length() const noexcept {
    return buffer_.size();
}

/*
    A subtle engineering note in the code (pay attention to it!)
    Look at this condition in the Delete function:
    if (index > buffer_.size() || length > buffer_.size() - index)

        Why didn't we write index + length > buffer_.size()?
        Because index and length are of type std::size_t (large positive numbers), if the user passes very large numbers,
    the sum of index + length might exceed the memory limit (Integer Overflow) and loop back to a small number,
    thus bypassing the condition! Subtracting index from buffer_.size() after ensuring that index <= buffer_.size() is a defensive programming technique that protects the code from this vulnerability.
  */