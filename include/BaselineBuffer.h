#pragma once
#include "ITextBuffer.h"

// Contiguous std::string-backed implementation used as a reference oracle.
// Provides O(1) random access, while Insert and Delete take O(N) in the worest case, due to shifting elements in memory

class BaselineBuffer : public ITextBuffer {

    private:
    
    // Internal UTF-8 byte storage.
    // Invariant: Length() always equals buffer_.size().
    std::string buffer_;


    public:
    BaselineBuffer() = default;
    explicit BaselineBuffer(std::string_view initialText);
    
    void Insert(std::size_t index, std::string_view text) override;
    void Delete(std::size_t index, std::size_t length) override;
    void Replace(std::size_t index, std::size_t length, std::string_view text) override;
    
    std::string GetText() const override;
    std::string GetRange(std::size_t index, std::size_t length) const override;
    std::size_t Length() const noexcept override;

};