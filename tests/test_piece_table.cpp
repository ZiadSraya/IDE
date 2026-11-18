#include "PieceTable.h"

#include <iostream>
#include <cassert>
#include <stdexcept>

// Arrange – Act – Assert (AAA)
// A common pattern in Unit Testing.

void TestInsertAtBeginning()
{
    PieceTable buffer("World");

    buffer.Insert(0, "Hello ");

    assert(buffer.GetText() == "Hello World");
    assert(buffer.Length() == 11);

    std::cout << "TestInsertAtBeginning: PASSED\n";
}

void TestEmptyDocument()
{
    PieceTable buffer("");

    assert(buffer.GetText() == "");
    assert(buffer.Length() == 0);

    std::cout << "TestEmptyDocument: PASSED\n";
}

void TestInsertAtMiddle()
{
    PieceTable buffer("Hello World");

    std::size_t middle = 6;

    buffer.Insert(middle, "Beautiful ");

    assert(buffer.GetText() == "Hello Beautiful World");
    assert(buffer.Length() == 21);

    std::cout << "TestInsertAtMiddle: PASSED\n";
}

void TestInsertAtEnd()
{
    PieceTable buffer("Hello Beautiful");

    buffer.Insert(buffer.Length(), " World");

    assert(buffer.GetText() == "Hello Beautiful World");
    assert(buffer.Length() == 21);

    std::cout << "TestInsertAtEnd: PASSED\n";
}

void TestDeleteZero()
{
    PieceTable buffer("Hello World");

    buffer.Delete(1, 0);

    assert(buffer.GetText() == "Hello World");
    assert(buffer.Length() == 11);

    std::cout << "TestDeleteZero: PASSED\n";
}

void TestInsertEmptyString()
{
    PieceTable buffer("Hello World");

    buffer.Insert(0, "");

    assert(buffer.GetText() == "Hello World");
    assert(buffer.Length() == 11);

    std::cout << "InsertEmptyString: PASSED\n";

}

void TestDeleteEntireDocument()
{
    PieceTable buffer("Hello World");

    buffer.Delete(0, 11);

    assert(buffer.GetText() == "");
    assert(buffer.Length() == 0);

    std::cout << "DeleteEntireDocument: PASSED\n";
}

void TestDeleteAcrossPieces()
{
    PieceTable buffer("Hello World");

    buffer.Insert(5, " Beautiful");

    buffer.Delete(8, 10);

    std::cout << "Actual: [" << buffer.GetText() << "]\n";
    std::cout << "Length: " << buffer.Length() << '\n';

    assert(buffer.GetText() == "Hello Berld");
    assert(buffer.Length() == 11);

    std::cout << "DeleteAcrossPieces: PASSED\n";
    
}

void TestReplaceAcrossPieces()
{
    PieceTable buffer("Hello World");

    buffer.Insert(5, " Beautiful");

    buffer.Replace(8, 10, "XYZ");

    assert(buffer.GetText() == "Hello BeXYZrld");
    assert(buffer.Length() == 14);

    std::cout << "ReplaceAcrossPieces: PASSED\n";
    
}

void TestGetRangeAcrossPieceBoundaries()
{
    PieceTable buffer("Hello World");

    buffer.Insert(5, " Beautiful");

    std::string range = buffer.GetRange(3, 7);

    std::cout << "Actual range: [" << range << "]\n";
    std::cout << "Actual size: " << range.size() << '\n';
    
    assert(range == "lo Beau");
    assert(range.size() == 7);
    assert(buffer.Length() == 21);

    std::cout << "GetRangeAcrossPieceBoundaries: PASSED\n";
}

void TestGetRangeBeforePieceBoundaries()
{
    PieceTable buffer("Hello World");

    buffer.Insert(5, " Beautiful");

    std::string range = buffer.GetRange(0, 5);

    assert(range == "Hello");
    assert(range.size() == 5);
    assert(buffer.Length() == 21);

    std::cout << "TestGetRangeBeforePieceBoundaries: PASSED\n";
}

void TestInvalidInsertIndex()
{
    PieceTable buffer("Hello World");

    bool exceptionThrown = false;

    try
    {
        buffer.Insert(12, "Test");
    }
    catch(const std::out_of_range&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "TestInvalidInsertIndex: PASSED\n";

}


void TestInvalidDeleteIndex()
{
    PieceTable buffer("Hello World");

    bool exceptionThrown = false;

    try
    {
        buffer.Delete(12, 4);
    }
    catch(const std::out_of_range&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "TestInvalidDeleteIndex: PASSED\n";

}


void TestInvalidDeleteLength()
{
    PieceTable buffer("Hello World");

    bool exceptionThrown = false;

    try
    {
        buffer.Delete(5, 10);
    }
    catch(const std::out_of_range&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "TestInvalidDeleteLength: PASSED\n";

}


void TestInvalidGetRangeLength()
{
    PieceTable buffer("Hello World");

    bool exceptionThrown = false;

    try
    {
        buffer.GetRange(5, 10);
    }
    catch(const std::out_of_range&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "TestInvalidGetRangeLength: PASSED\n";

}

void TestOriginalBufferEmpty()
{
    PieceTable buffer("");

    buffer.Insert(0, "Hello World");
    
    assert(buffer.GetText() == "Hello World");
    assert(buffer.Length() == 11);

    std::cout << "OriginalBufferEmpty: PASSED\n";

}

void TestMultipleInsertsAtTheSamePosition()
{
    PieceTable buffer("Hello");

    buffer.Insert(5, " World");
    buffer.Insert(5, " Beautiful");
    buffer.Insert(5, " Very");

    assert(buffer.GetText() == "Hello Very Beautiful World");
    assert(buffer.Length() == 26);

    std::cout << "TestMultipleInsertsAtTheSamePosition: PASSED\n";
}


void TestDeleteAllThenInsert()
{
    PieceTable buffer("Deleted Hello");

    buffer.Delete(0, buffer.Length());

    assert(buffer.GetText() == "");
    assert(buffer.Length() == 0);

    buffer.Insert(0, "Inserted After Deletion Hello");

    assert(buffer.GetText() == "Inserted After Deletion Hello");
    assert(buffer.Length() == 29);

    std::cout << "TestDeleteAllThenInsert: PASSED\n";
}


void TestReplaceWithEmptyString()
{
    PieceTable buffer("Hello");

    buffer.Replace(0, 5, "");

    assert(buffer.GetText() == "");
    assert(buffer.Length() == 0);

    std::cout << "TestReplaceWithEmptyString: PASSED\n";
}


void TestInsertTextLongerThanDocument()
{
    PieceTable buffer("Hello");

    buffer.Insert(5, "This is a longer text");

    assert(buffer.GetText() == "HelloThis is a longer text");
    assert(buffer.Length() == 26);

    std::cout << "TestInsertTextLongerThanDocument: PASSED\n";
}


void TestAdjacentPiecesFromSameSource()
{
    PieceTable buffer("Hello");

    buffer.Insert(5, " ");
    buffer.Insert(6, "World");

    buffer.Normalize();

    assert(buffer.GetText() == "Hello World");
    assert(buffer.Length() == 11);

    std::cout << "TestAdjacentPiecesFromSameSource: PASSED\n";
}


void TestNewLinesAndCRLF()
{
    // LF Test
    PieceTable lfBuffer("Hello\nWorld");

    assert(lfBuffer.GetText() == "Hello\nWorld");
    assert(lfBuffer.Length() == 11);


    // CRLF Test
    PieceTable crlfBuffer("Hello\r\nWorld");

    assert(crlfBuffer.GetText() == "Hello\r\nWorld");
    assert(crlfBuffer.Length() == 12);


    // Insert CRLF Test
    PieceTable insertBuffer("HelloWorld");

    insertBuffer.Insert(5, "\r\n");

    assert(insertBuffer.GetText() == "Hello\r\nWorld");
    assert(insertBuffer.Length() == 12);

    std::cout << "TestNewLinesAndCRLF: PASSED\n";
}


void TestUTF8Arabic()
{
    // Basic Arabic Text
    PieceTable buffer("مرحبا بالعالم");

    assert(buffer.GetText() == "مرحبا بالعالم");
    assert(buffer.Length() == 25);


    // Insert Arabic Text
    buffer.Insert(25, " الجميل");

    assert(buffer.GetText() == "مرحبا بالعالم الجميل");
    assert(buffer.Length() == 38);


    // Delete Arabic Text
    buffer.Delete(0, buffer.Length());

    buffer.Insert(0, "مرحبا بالعالم");

    buffer.Delete(0, 11);

    assert(buffer.GetText() == "بالعالم");
    assert(buffer.Length() == 14);

    std::cout << "TestUTF8Arabic: PASSED\n";
}


int main()
{
    TestEmptyDocument();
    TestInsertAtBeginning();
    TestInsertAtMiddle();
    TestInsertAtEnd();
    TestDeleteZero();
    TestInsertEmptyString();
    TestDeleteEntireDocument();
    TestDeleteAcrossPieces();
    TestReplaceAcrossPieces();
    TestGetRangeAcrossPieceBoundaries();
    TestGetRangeBeforePieceBoundaries();
    TestInvalidInsertIndex();
    TestInvalidDeleteIndex();
    TestInvalidDeleteLength();
    TestInvalidGetRangeLength();
    TestOriginalBufferEmpty();
    TestMultipleInsertsAtTheSamePosition();
    TestDeleteAllThenInsert();
    TestReplaceWithEmptyString();
    TestInsertTextLongerThanDocument();
    TestAdjacentPiecesFromSameSource();
    TestNewLinesAndCRLF();
    TestUTF8Arabic();

    std::cout << "\nAll tests passed!\n";

    return 0;
}