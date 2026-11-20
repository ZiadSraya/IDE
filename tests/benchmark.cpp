#include "BaselineBuffer.h"
#include "GapBuffer.h"
#include "PieceTable.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <random>
#include <iomanip>

using Clock = std::chrono::steady_clock;

enum class FileSize : size_t {
    KB_100 = 100 * 1024,
    KB_512 = 512 * 1024,
    MB_1   = 1024 * 1024,
    MB_5   = 5 * 1024 * 1024,
    MB_10  = 10 * 1024 * 1024
};

enum class OperationType
{
    Insert,
    Delete,
    Replace
};

struct EditOperation
{
    OperationType type;
    std::size_t index;
    std::size_t length;
    std::string text;
};

constexpr std::size_t TRIAL_COUNT = 7;
constexpr std::size_t TYPING_CHUNK_SIZE = 4096;
constexpr std::size_t RANDOM_OPERATION_COUNT = 2000;
constexpr std::size_t RANGE_READ_COUNT = 256;
constexpr std::size_t RANGE_READ_LENGTH = 128;
constexpr std::size_t FULL_READ_COUNT = 5;

volatile std::uint64_t g_checksumSink = 0;

template <typename Function>
double Measure(Function&& function)
{
    const auto start = Clock::now();

    function();

    const auto end = Clock::now();

    return std::chrono::duration<double, std::milli>(
        end - start
    ).count();
}

std::vector<EditOperation> GenerateRandomOperations(
    std::size_t initialLength,
    unsigned int seed = 12345)
{
    std::vector<EditOperation> operations;
    operations.reserve(RANDOM_OPERATION_COUNT);

    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> operationDistribution(0, 99);

    std::size_t simulatedLength = initialLength;

    for (std::size_t i = 0; i < RANDOM_OPERATION_COUNT; ++i)
    {
        int operation = operationDistribution(generator);

        // Insert: 40%
        if (operation < 40 || simulatedLength == 0)
        {
            std::uniform_int_distribution<std::size_t>
                indexDistribution(0, simulatedLength);

            std::size_t index = indexDistribution(generator);

            operations.push_back({
                OperationType::Insert,
                index,
                0,
                "random text"
            });

            simulatedLength += 11;
        }
        // Delete: 30%
        else if (operation < 70)
        {
            std::uniform_int_distribution<std::size_t>
                indexDistribution(0, simulatedLength - 1);

            std::size_t index = indexDistribution(generator);

            std::size_t maxLen = std::min<std::size_t>(
                100,
                simulatedLength - index
            );

            std::uniform_int_distribution<std::size_t>
                lengthDistribution(1, maxLen);

            std::size_t length = lengthDistribution(generator);

            operations.push_back({
                OperationType::Delete,
                index,
                length,
                ""
            });

            simulatedLength -= length;
        }
        // Replace: 30%
        else
        {
            std::uniform_int_distribution<std::size_t>
                indexDistribution(0, simulatedLength - 1);

            std::size_t index = indexDistribution(generator);

            std::size_t maxLen = std::min<std::size_t>(
                100,
                simulatedLength - index
            );

            std::uniform_int_distribution<std::size_t>
                lengthDistribution(1, maxLen);

            std::size_t length = lengthDistribution(generator);

            operations.push_back({
                OperationType::Replace,
                index,
                length,
                "replacement"
            });

            if (length <= 11)
                simulatedLength += 11 - length;
            else
                simulatedLength -= length - 11;
        }
    }

    return operations;
}
std::vector<std::pair<std::size_t, std::size_t>>
GenerateReadRanges(std::size_t textLength, unsigned int seed = 54321)
{
    if (textLength < RANGE_READ_LENGTH)
        throw std::invalid_argument("Text is too small for range reading.");

    std::mt19937 generator(seed);

    std::uniform_int_distribution<std::size_t>
        indexDistribution(0, textLength - RANGE_READ_LENGTH);

    std::vector<std::pair<std::size_t, std::size_t>> ranges;
    ranges.reserve(RANGE_READ_COUNT);

    for (std::size_t i = 0; i < RANGE_READ_COUNT; ++i)
    {
        ranges.emplace_back(
            indexDistribution(generator),
            RANGE_READ_LENGTH
        );
    }

    return ranges;
}

std::unique_ptr<ITextBuffer> CreateBuffer(
    int type,
    std::string_view initialText)
{
    switch (type)
    {
    case 0:
        return std::make_unique<BaselineBuffer>(initialText);

    case 1:
    {
        auto buffer = std::make_unique<GapBuffer>(initialText.size());
        buffer->Insert(0, initialText);
        return buffer;
    }

    case 2:
        return std::make_unique<PieceTable>(initialText);

    default:
        throw std::invalid_argument("Unknown buffer type.");
    }
}

std::string ToString(FileSize size)
{
    const std::size_t bytes = static_cast<std::size_t>(size);

    if (bytes % (1024 * 1024) == 0)
        return std::to_string(bytes / (1024 * 1024)) + " MiB";

    return std::to_string(bytes / 1024) + " KiB";
}

std::string GenerateText(
    FileSize size = FileSize::MB_1,
    const std::string& pattern = "Abo galmbo elly nayem 3ala ganbo")
{
    const std::size_t targetSize = static_cast<std::size_t>(size);

    if (targetSize > 0 && pattern.empty())
        throw std::invalid_argument("Pattern cannot be empty.");

    std::string result;
    result.reserve(targetSize);

    while (result.size() < targetSize)
        result += pattern;

    result.resize(targetSize);
    return result;
}

double CalculateMedian(std::vector<double> times)
{
    if (times.empty())
        throw std::invalid_argument("Cannot calculate median of empty data.");

    std::sort(times.begin(), times.end());

    const std::size_t n = times.size();

    if (n % 2 == 0)
        return (times[n / 2 - 1] + times[n / 2]) / 2.0;

    return times[n / 2];
}

void ApplyOperations(
    ITextBuffer& buffer,
    const std::vector<EditOperation>& operations)
{
    for (const auto& op : operations)
    {
        switch (op.type)
        {
        case OperationType::Insert:
            buffer.Insert(op.index, op.text);
            break;

        case OperationType::Delete:
            buffer.Delete(op.index, op.length);
            break;

        case OperationType::Replace:
            buffer.Replace(op.index, op.length, op.text);
            break;
        }
    }
}

template <typename Factory, typename Workload>
void RunTrials(
    const std::string& bufferName,
    const std::string& workloadName,
    const std::string& expectedText,
    Factory factory,
    Workload workload)
{
    std::vector<double> times;
    times.reserve(TRIAL_COUNT);

    auto runTrial = [&]()
    {
        auto buffer = factory();

        double elapsed = Measure([&]()
        {
            workload(*buffer);
        });

        if (buffer->GetText() != expectedText)
            throw std::runtime_error(
                bufferName + " failed correctness validation."
            );

        return elapsed;
    };

    // Warm-up
    runTrial();

    // Actual trials
    for (std::size_t i = 0; i < TRIAL_COUNT; ++i)
        times.push_back(runTrial());

    std::cout
        << std::left << std::setw(16) << bufferName
        << std::setw(28) << workloadName
        << std::right << std::fixed << std::setprecision(3)
        << CalculateMedian(times) << " ms\n";
}

void RunBenchmark()
{
    const std::string bufferNames[] =
    {
        "BaselineBuffer",
        "GapBuffer",
        "PieceTable"
    };

    const FileSize sizes[] =
    {
        FileSize::KB_100,
        FileSize::KB_512,
        FileSize::MB_1,
        FileSize::MB_5,
        FileSize::MB_10
    };

    for (FileSize size : sizes)
    {
        std::string initialText = GenerateText(size);

        auto operations =
            GenerateRandomOperations(initialText.size());

        auto ranges =
            GenerateReadRanges(initialText.size());

        // Calculate expected result for Random Edits.
        std::string expectedRandomText = initialText;

        for (const auto& op : operations)
        {
            switch (op.type)
            {
            case OperationType::Insert:
                expectedRandomText.insert(op.index, op.text);
                break;

            case OperationType::Delete:
                expectedRandomText.erase(op.index, op.length);
                break;

            case OperationType::Replace:
                expectedRandomText.replace(
                    op.index, op.length, op.text
                );
                break;
            }
        }

        std::cout << "\n========================================\n";
        std::cout << "File Size: " << ToString(size) << '\n';
        std::cout << "========================================\n";

        std::cout << std::left
                  << std::setw(16) << "Buffer"
                  << std::setw(28) << "Workload"
                  << "Median\n";

        for (int type = 0; type < 3; ++type)
        {
            const std::string& bufferName = bufferNames[type];

            // --------------------------------
            // 1. Sequential Typing
            // --------------------------------

            auto typingFactory = [&]()
            {
                return CreateBuffer(type, "");
            };

            auto typingWorkload = [&](ITextBuffer& buffer)
            {
                for (std::size_t i = 0;
                     i < initialText.size();
                     i += TYPING_CHUNK_SIZE)
                {
                    std::string_view chunk(initialText);

                    chunk = chunk.substr(
                        i,
                        TYPING_CHUNK_SIZE
                    );

                    buffer.Insert(buffer.Length(), chunk);
                }
            };

            RunTrials(
                bufferName,
                "Sequential Typing",
                initialText,
                typingFactory,
                typingWorkload
            );

            // --------------------------------
            // 2. Random Edits
            // --------------------------------

            auto randomFactory = [&]()
            {
                return CreateBuffer(type, initialText);
            };

            auto randomWorkload = [&](ITextBuffer& buffer)
            {
                ApplyOperations(buffer, operations);
            };

            RunTrials(
                bufferName,
                "Random Edits",
                expectedRandomText,
                randomFactory,
                randomWorkload
            );

            PieceTable diagnosticBuffer(initialText);

            ApplyOperations(diagnosticBuffer, operations);
            const std::size_t piecesBefore = diagnosticBuffer.PieceCount();
            const std::string textBefore = diagnosticBuffer.GetText();

            diagnosticBuffer.Normalize();

            const std::size_t piecesAfter = diagnosticBuffer.PieceCount();
            const std::string textAfter = diagnosticBuffer.GetText();

            std::cout << "Piece Count Before Normalize: " << piecesBefore << '\n';
            std::cout << "Piece Count After Normalize:  " << piecesAfter << '\n';

            if (textBefore != textAfter)
            {
                throw std::runtime_error("Normalize changed document content!");
            }

            // --------------------------------
            // 3. Full Read
            // --------------------------------

            auto fullReadWorkload = [&](ITextBuffer& buffer)
            {
                std::uint64_t checksum = 0;

                for (std::size_t i = 0; i < FULL_READ_COUNT; ++i)
                {
                    std::string result = buffer.GetText();

                    for (unsigned char ch : result)
                        checksum = checksum * 31 + ch;
                }

                g_checksumSink = g_checksumSink ^ checksum;
            };

            RunTrials(
                bufferName,
                "Full Read",
                initialText,
                randomFactory,
                fullReadWorkload
            );

            // --------------------------------
            // 4. Range Read
            // --------------------------------

            auto rangeReadWorkload = [&](ITextBuffer& buffer)
            {
                std::uint64_t checksum = 0;

                for (const auto& [index, length] : ranges)
                {
                    std::string result =
                        buffer.GetRange(index, length);

                    for (unsigned char ch : result)
                        checksum = checksum * 31 + ch;
                }

                g_checksumSink = g_checksumSink ^ checksum;
            };

            RunTrials(
                bufferName,
                "Range Read",
                initialText,
                randomFactory,
                rangeReadWorkload
            );
        }
    }
}

int main()
{
    try
    {
        RunBenchmark();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Benchmark Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}