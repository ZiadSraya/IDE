#include "../include/BaselineBuffer.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

void TestInsert() {
    BaselineBuffer buf;
    buf.Insert(0, "World");
    assert(buf.GetText() == "World");

    buf.Insert(0, "Hello ");
    assert(buf.GetText() == "Hello World");

    buf.Insert(11, "!");
    assert(buf.GetText() == "Hello World!");

    buf.Insert(5, ",");
    assert(buf.GetText() == "Hello, World!");
    assert(buf.Length() == 13);
}

void TestDelete() {
    BaselineBuffer buf("Hello, World!");
    buf.Delete(5, 1);
    assert(buf.GetText() == "Hello World!");

    buf.Delete(5, 6);
    assert(buf.GetText() == "Hello!");

    buf.Delete(0, 6);
    assert(buf.GetText() == "");
    assert(buf.Length() == 0);
}

void TestEmptyOperations() {
    BaselineBuffer buf;
    buf.Insert(0, "");
    assert(buf.Length() == 0);

    buf.Delete(0, 0);
    assert(buf.GetText() == "");

    assert(buf.GetRange(0, 0) == "");
}

void TestBoundariesAndRange() {
    BaselineBuffer buf("ABC");
    assert(buf.GetRange(0, 1) == "A");
    assert(buf.GetRange(2, 1) == "C");
    assert(buf.GetRange(0, 3) == "ABC");
}

void TestSequentialOperations() {
    BaselineBuffer buf("I love Java");
    buf.Replace(7, 4, "C++");
    assert(buf.GetText() == "I love C++");

    buf.Insert(2, "really ");
    assert(buf.GetText() == "I really love C++");

    buf.Delete(2, 7);
    assert(buf.GetText() == "I love C++");
}

void TestInvalidInputs() {
    BaselineBuffer buf("Test");
    bool caught = false;

    try {
        buf.Insert(5, "X");
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try {
        buf.Delete(2, 5);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);
}

int main() {
    TestInsert();
    TestDelete();
    TestEmptyOperations();
    TestBoundariesAndRange();
    TestSequentialOperations();
    TestInvalidInputs();

    std::cout << "All tests passed successfully!\n";
    return 0;
}
