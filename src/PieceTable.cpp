#include "PieceTable.h"
#include <stdexcept>
#include <algorithm>

PieceTable::PieceTable(std::string_view initialText)
    : originalBuffer_(initialText)
{
    if (!originalBuffer_.empty())
    {
        pieces_.push_back({
            0,
            originalBuffer_.size(),
            BufferSource::Original
        });
    }
}

std::string PieceTable::GetText() const
{
    std::string result;

    for (const Piece& piece : pieces_)
    {
        const std::string& sourceBuffer = (piece.source == BufferSource::Original) ? originalBuffer_ : addBuffer_;

        result.append(sourceBuffer, piece.start, piece.length);
    }

    return result;
}

std::size_t PieceTable::Length() const noexcept
{
    std::size_t totalLength = 0;
    for (const Piece& piece : pieces_)
    {
        totalLength += piece.length;
    }
    return totalLength;
}

std::string PieceTable::GetRange(std::size_t index, std::size_t length) const
{
    const std::size_t documentLength = Length();

    if (index > documentLength || length > documentLength - index)
    {
        throw std::out_of_range("GetRange range is out of bounds");
    }

    std::string result;
    result.reserve(length);

    const std::size_t rangeEnd = index + length;

    // It represents the beginning of the current Piece within the final document (Logical Position).
    std::size_t currentPosition = 0;

    for (const Piece& piece : pieces_)
    {
        // pieceStart: The logical starting point of the piece in the document.
        const std::size_t pieceStart = currentPosition;
        // pieceEnd: Its logical conclusion.
        const std::size_t pieceEnd = pieceStart + piece.length;

        currentPosition = pieceEnd;

        // Discard unwanted pieces
        if (pieceEnd <= index)
            continue;

        if (pieceStart >= rangeEnd)
            break;
    
        // Calculate the required portion of each piece.

        const std::size_t overlapStart = std::max(index, pieceStart);
        const std::size_t overlapEnd = std::min(rangeEnd, pieceEnd);

        // offset: The distance from the beginning of the Piece to the beginning of the required part.
        // (This is a local offset within the piece).

        const std::size_t offset = overlapStart - pieceStart;

        // length is the total required range length,
        // while copyLength is the portion that the current Piece will contribute.

        const std::size_t copyLength = overlapEnd - overlapStart;

        const std::string& sourceBuffer =
            (piece.source == BufferSource::Original)
            ? originalBuffer_
            : addBuffer_;

        const std::size_t sourceStart = piece.start + offset;

        result.append(sourceBuffer, sourceStart, copyLength);
    }

    return result;
}

void PieceTable::Insert(std::size_t index, std::string_view text)
{
    if (index > Length())
    {
        throw std::out_of_range("Insert index is out of bounds");
    }

    if (text.empty())
        return;

    // Save the insertion position in Add Buffer
    std::size_t addStart = addBuffer_.size();

    addBuffer_.append(text);

    Piece newPiece{
        addStart,
        text.size(),
        BufferSource::Add
    };

    std::size_t currentPosition = 0;

    for (std::size_t i = 0; i < pieces_.size(); ++i)
    {
        Piece& piece = pieces_[i];

        std::size_t pieceEnd = currentPosition + piece.length;

        // Skip Pieces before the insertion point
        if (index > pieceEnd)
        {
            currentPosition = pieceEnd;
            continue;
        }

        std::size_t offset = index - currentPosition;

        // Insert at the beginning of the Piece
        if (offset == 0)
        {
            pieces_.insert(pieces_.begin() + i, newPiece);
        }
        // Insert at the end of the Piece
        else if (offset == piece.length)
        {
            pieces_.insert(pieces_.begin() + i + 1, newPiece);
        }
        // Insert inside the Piece: split it into two
        else
        {
            Piece rightPiece{
                piece.start + offset,
                piece.length - offset,
                piece.source
            };

            piece.length = offset;

            pieces_.insert(pieces_.begin() + i + 1, newPiece);
            pieces_.insert(pieces_.begin() + i + 2, rightPiece);
        }

        return;
    }

    // Insert at the end of the document
    pieces_.push_back(newPiece);
}


void PieceTable::Delete(std::size_t index, std::size_t length)
{
    const std::size_t documentLength = Length();

    if (index > documentLength || length > documentLength - index)
    {
        throw std::out_of_range("Delete range is out of bounds");
    }

    if (length == 0)
        return;

    const std::size_t deleteEnd = index + length;
    std::size_t currentPosition = 0;

    for (std::size_t i = 0; i < pieces_.size();)
    {
        Piece& piece = pieces_[i];

        const std::size_t pieceStart = currentPosition;
        const std::size_t pieceEnd = pieceStart + piece.length;

        if (pieceEnd <= index || pieceStart >= deleteEnd)
        {
            currentPosition = pieceEnd;
            ++i;
            continue;
        }

        const std::size_t overlapStart = std::max(pieceStart, index);
        const std::size_t overlapEnd = std::min(pieceEnd, deleteEnd);

        const std::size_t offset = overlapStart - pieceStart;
        const std::size_t deleteLength = overlapEnd - overlapStart;

        if (offset == 0 && deleteLength == piece.length)
        {
            pieces_.erase(pieces_.begin() + i);
            currentPosition = pieceEnd;
        }
        else if (offset == 0)
        {
            piece.start += deleteLength;
            piece.length -= deleteLength;

            currentPosition = pieceEnd;
            ++i;
        }
        else if (offset + deleteLength == piece.length)
        {
            piece.length -= deleteLength;

            currentPosition = pieceEnd;
            ++i;
        }
        else
        {
            Piece rightPiece{
                piece.start + offset + deleteLength,
                piece.length - offset - deleteLength,
                piece.source
            };

            piece.length = offset;

            pieces_.insert(pieces_.begin() + i + 1, rightPiece);

            currentPosition = pieceEnd;
            i += 2;
        }
    }
}

void PieceTable::Replace(std::size_t index, std::size_t length, std::string_view text)
{
    std::string replacement(text); // Independent copy: text may refer to addBuffer_

    Delete(index, length);
    Insert(index, replacement);
}

void PieceTable::Normalize()
{
    for (std::size_t i = 0; i < pieces_.size();)
    {
        if (pieces_[i].length == 0)
        {
            pieces_.erase(pieces_.begin() + i);
            continue;
        }

        if (i + 1 >= pieces_.size())
        {
            ++i;
            continue;
        }

        Piece& current = pieces_[i];
        Piece& next = pieces_[i + 1];

        if (current.source == next.source &&
            current.start + current.length == next.start)
        {
            current.length += next.length;
            pieces_.erase(pieces_.begin() + i + 1);
        }
        else
        {
            ++i;
        }
    }
}

std::size_t PieceTable::PieceCount() const noexcept
{
    return pieces_.size();
}