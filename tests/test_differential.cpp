#include "BaselineBuffer.h"
#include "PieceTable.h"
#include <random>
#include <iostream>
#include <string>
#include <cassert>

enum OperationType
{
    Insert,
    Delete
};

OperationType GetOperationType(std::mt19937& rng)
{
    std::uniform_int_distribution<int> operationDist(0, 1);
    return operationDist(rng) == 0 ? Insert : Delete;
}

bool VerifyBuffers(
    const BaselineBuffer& baselineBuffer,
    const PieceTable& pieceTableBuffer,
    int operationNumber,
    OperationType operationType,
    std::size_t index
)
{
    std::string expected = baselineBuffer.GetText();
    std::string actual = pieceTableBuffer.GetText();

    if (expected != actual)
    {
        std::cerr << "Mismatch detected!\n";
        std::cerr << "Operation number: " << operationNumber << '\n';
        std::cerr << "Operation type: "
                  << (operationType == Insert ? "Insert" : "Delete") << '\n';
        std::cerr << "Index: " << index << '\n';
        std::cerr << "Expected: " << expected << '\n';
        std::cerr << "Actual: " << actual << '\n';

        return false;
    }

    if (baselineBuffer.Length() != pieceTableBuffer.Length())
    {
        std::cerr << "Length mismatch at operation: "
                  << operationNumber << '\n';
        std::cerr << "Expected length: "
                  << baselineBuffer.Length() << '\n';
        std::cerr << "Actual length: "
                  << pieceTableBuffer.Length() << '\n';

        return false;
    }

    return true;
}

bool VerifyRandomRange(
    const BaselineBuffer& baselineBuffer,
    const PieceTable& pieceTableBuffer,
    std::mt19937& rng,
    int operationNumber
)
{
    std::size_t currentLength = baselineBuffer.Length();

    std::uniform_int_distribution<std::size_t> rangeIndexDist(
        0, currentLength
    );

    std::size_t rangeIndex = rangeIndexDist(rng);
    std::size_t remainingLength = currentLength - rangeIndex;

    std::uniform_int_distribution<std::size_t> rangeLengthDist(
        0, remainingLength
    );

    std::size_t rangeLength = rangeLengthDist(rng);

    std::string expectedRange =
        baselineBuffer.GetRange(rangeIndex, rangeLength);

    std::string actualRange =
        pieceTableBuffer.GetRange(rangeIndex, rangeLength);

    if (expectedRange != actualRange)
    {
        std::cerr << "GetRange mismatch!\n";
        std::cerr << "Operation number: " << operationNumber << '\n';
        std::cerr << "Range index: " << rangeIndex << '\n';
        std::cerr << "Range length: " << rangeLength << '\n';
        std::cerr << "Expected: " << expectedRange << '\n';
        std::cerr << "Actual: " << actualRange << '\n';

        return false;
    }

    return true;
}

void RunDifferentialTest()
{
    std::mt19937 rng(12345);

    BaselineBuffer baselineBuffer("Hello");
    PieceTable pieceTableBuffer("Hello");

    for (int i = 0; i < 10000; ++i)
    {
        OperationType operationType = GetOperationType(rng);

        std::size_t documentLength = baselineBuffer.Length();

        std::uniform_int_distribution<std::size_t> indexDist(
            0, documentLength
        );

        std::size_t index = indexDist(rng);

        if (operationType == Insert)
        {
            std::string text = "abc";

            baselineBuffer.Insert(index, text);
            pieceTableBuffer.Insert(index, text);
        }
        else if (documentLength > 0 && index < documentLength)
        {
            std::size_t remainingLength = documentLength - index;

            std::uniform_int_distribution<std::size_t> lengthDist(
                1, remainingLength
            );

            std::size_t length = lengthDist(rng);

            baselineBuffer.Delete(index, length);
            pieceTableBuffer.Delete(index, length);
        }

        // False >> true
        if (!VerifyBuffers(baselineBuffer, pieceTableBuffer, i, operationType, index))
        {
            return;
        }

        if (!VerifyRandomRange(baselineBuffer, pieceTableBuffer, rng, i))
        {
            return;
        }
    }

    std::cout << "Random Differential Test Passed!\n";
}

int main()
{
    RunDifferentialTest();
    return 0;
}