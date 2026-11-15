#pragma once
#include "ITextBuffer.h"


class BaselineBuffer : public ITextBuffer {

    private:
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