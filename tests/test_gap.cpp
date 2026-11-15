#include "../include/BaselineBuffer.h"
#include "../include/GapBuffer.h"
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

// Part 1: Functional Testing
void RunFunctionalTests() {
    std::cout << "=== 1. Functional Testing ===\n";
    GapBuffer buffer(4);

    buffer.Insert(0, "Hello");
    buffer.Insert(5, " World");
    buffer.Insert(5, ",");
    buffer.Delete(5, 1);

    buffer.Insert(0, "Start ");
    buffer.Insert(buffer.Length(), " End");
    buffer.Replace(0, 5, "Begin");

    std::string finalText = buffer.GetText();
    std::size_t finalLength = buffer.Length();

    assert(finalText == "Begin Hello World End");
    assert(finalLength == 21);

    std::cout << "Expected Text : \"Begin Hello World End\"\n";
    std::cout << "Actual Text   : \"" << finalText << "\"\n";
    std::cout << "Length()      : " << finalLength << "\n";
    std::cout << "Exceptions    : None\n\n";
}

// Part 2: Edge Cases
void RunEdgeCaseTests() {
    std::cout << "=== 2. Edge Cases Testing ===\n";
    GapBuffer buffer(4);

    // 1. Empty Insert & Empty Delete (No-ops)
    buffer.Insert(0, "");
    assert(buffer.Length() == 0);
    buffer.Delete(0, 0);
    assert(buffer.Length() == 0);
    std::cout << "[PASS] Empty Insert & Empty Delete\n";

    // 2. Insert Beyond Bounds
    bool caughtInsert = false;
    try {
        buffer.Insert(100, "X");
    } catch (const std::out_of_range&) {
        caughtInsert = true;
    }
    assert(caughtInsert);
    std::cout << "[PASS] Insert Beyond Bounds threw std::out_of_range\n";

    // 3. Delete Beyond Bounds
    bool caughtDelete = false;
    try {
        buffer.Delete(0, 100);
    } catch (const std::out_of_range&) {
        caughtDelete = true;
    }
    assert(caughtDelete);
    std::cout << "[PASS] Delete Beyond Bounds threw std::out_of_range\n";

    // 4. Large Insert (larger than the initial gap size of 4)
    std::string largeText(50, 'A');
    buffer.Insert(0, largeText);
    assert(buffer.Length() == 50);
    assert(buffer.GetText() == largeText);
    std::cout << "[PASS] Large Insert triggered geometric expansion\n";

    // 5. Gap Movement (repeated insertions at the beginning and end)
    GapBuffer moveBuf(4);
    for (int i = 0; i < 10; ++i) {
        moveBuf.Insert(0, "L");                  // Insert at the beginning
        moveBuf.Insert(moveBuf.Length(), "R");   // Insert at the end
    }
    assert(moveBuf.Length() == 20);
    assert(moveBuf.GetText() == "LLLLLLLLLLRRRRRRRRRR");
    std::cout << "[PASS] Repeated Start/End Gap Movement\n\n";
}

// Part 3: Differential Testing (comparing 500 random operations between Baseline and GapBuffer)
void RunDifferentialTests() {
    std::cout << "=== 3. Differential Testing ===\n";
    BaselineBuffer baseline;
    GapBuffer gap(8);

    std::mt19937 rng(43); // Fixed seed to reproduce the test with identical results,                    [he Hitchhiker's Guide to the Galaxy]
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyz0123456789 ";

    for (int step = 0; step < 500; ++step) {
        std::size_t currentLen = baseline.Length();
        int op = rng() % 3; // 0: Insert, 1: Delete, 2: Replace

        if (op == 0 || currentLen == 0) {
            // Random Insert operation
            std::size_t pos = (currentLen == 0) ? 0 : (rng() % (currentLen + 1));
            std::size_t insertLen = (rng() % 5) + 1;
            std::string str;
            for (std::size_t i = 0; i < insertLen; ++i) {
                str += alphabet[rng() % alphabet.size()];
            }

            baseline.Insert(pos, str);
            gap.Insert(pos, str);
        } else if (op == 1) {
            // Random Delete operation
            std::size_t pos = rng() % currentLen;
            std::size_t maxDel = currentLen - pos;
            std::size_t delLen = (rng() % maxDel) + 1;

            baseline.Delete(pos, delLen);
            gap.Delete(pos, delLen);
        } else {
            // Random Replace operation
            std::size_t pos = rng() % currentLen;
            std::size_t maxRep = currentLen - pos;
            std::size_t repLen = (rng() % maxRep) + 1;
            std::size_t newLen = (rng() % 5) + 1;
            std::string str;
            for (std::size_t i = 0; i < newLen; ++i) {
                str += alphabet[rng() % alphabet.size()];
            }

            baseline.Replace(pos, repLen, str);
            gap.Replace(pos, repLen, str);
        }

        // Verify 100% consistency between both engines after every operation
        assert(baseline.GetText() == gap.GetText());
        assert(baseline.Length() == gap.Length());
    }

    std::cout << "[PASS] 500 randomized Insert/Delete/Replace operations matched 100%!\n\n";
}

int main() {
    RunFunctionalTests();
    RunEdgeCaseTests();
    RunDifferentialTests();

    std::cout << "ALL VERIFICATION TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}

