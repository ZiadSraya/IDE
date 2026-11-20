#pragma once
#include "ITextBuffer.h"
#include <string>
#include <string_view>
#include <vector>
#include <cstddef>

class PieceTable : public ITextBuffer {
private:
    std::string originalBuffer_;
    std::string addBuffer_;

    enum class BufferSource
    {
        Original,
        Add
    };

    struct Piece
    {
        std::size_t start;
        std::size_t length;
        BufferSource source;
    };

    std::vector<Piece> pieces_;

public:
    explicit PieceTable(std::string_view initialText);
    std::size_t PieceCount() const noexcept;

    void Insert(std::size_t index, std::string_view text) override;
    void Delete(std::size_t index, std::size_t length) override;
    void Replace(std::size_t index, std::size_t length, std::string_view text) override;
    void Normalize();

    std::string GetText() const override;
    std::string GetRange(std::size_t index, std::size_t length) const override;
    std::size_t Length() const noexcept override;


};