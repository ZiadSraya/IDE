#pragma once
#include <string>
#include <string_view>
#include <cstddef>

class ITextBuffer {
public:
    virtual ~ITextBuffer() = default;

    // Insert text at a specific position
    virtual void Insert(std::size_t index, std::string_view text) = 0;

    // Delete a range of text starting at index with the given length
    virtual void Delete(std::size_t index, std::size_t length) = 0;

    // Replace a range of text with new text
    virtual void Replace(std::size_t index, std::size_t length, std::string_view text) = 0;

    // Retrieve the full text
    virtual std::string GetText() const = 0;

    // Retrieve a substring starting at index with the given length
    virtual std::string GetRange(std::size_t index, std::size_t length) const = 0;

    // Get the current length of the text
    virtual std::size_t Length() const noexcept = 0;
};
