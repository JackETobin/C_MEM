
#include <Windows.h>
#include <stdio.h>

#include "c_mem.h"

int WINAPI WinMain
(   HINSTANCE           currentInstance, 
    HINSTANCE           zero,
    PSTR                lpCmdLine,
    int                 nCmdShow
)
{
    OutputDebugStringA("C_MEM TEST SITE OPERATIONAL.....\n");
    
    pool_handle testPool = OpenPool(1);

    block_handle testBlock1 = BuildBlock(&testBlock1, testPool, 6, sizeof(uint32));
    uint32 numElements = NumElements(testBlock1);
    uint32 blockSize = BlockSize(testBlock1);
    uint32 elementSize = ElementSize(testBlock1);
    
    uint32 testData = 14;
    for (uint32 i = 0; i < NumElements(testBlock1); i++)
    {
        Push(testBlock1, &testData);
        testData++;
    }
    for (uint32 i = 0; i < NumElements(testBlock1); i++)
        RotateLast(testBlock1);

    for (uint32 i = 0; i < NumElements(testBlock1); i++)
        Pop(testBlock1);

    for (uint32 i = 0; i < NumElements(testBlock1) - 1; i++)
        Set(testBlock1, &testData, i);

    for (uint32 i = 0; i < NumElements(testBlock1); i++)
        RotateFirst(testBlock1);
    
    block_handle zeroBlock = BuildBlock(&zeroBlock, testPool, 0, sizeof(uint32));
    numElements = NumElements(zeroBlock);
    blockSize = BlockSize(zeroBlock);

    block_handle testBlock2 = BuildBlock(&testBlock2, testPool, 6, sizeof(uint32));
    block_handle testBlock3 = BuildBlock(&testBlock3, testPool, 6, sizeof(uint32));
    block_handle testBlock4 = BuildBlock(&testBlock4, testPool, 6, sizeof(uint32));
    block_handle testBlock5 = BuildBlock(&testBlock5, testPool, 6, sizeof(uint32));
    block_handle testBlock6 = BuildBlock(&testBlock6, testPool, 6, sizeof(uint32));
    block_handle testBlock7 = BuildBlock(&testBlock7, testPool, 6, sizeof(uint32));
    block_handle testBlock8 = BuildBlock(&testBlock8, testPool, 6, sizeof(uint32));

    void_info testPoolVoidInfo;
    FreeBlock(&testBlock1);
    testPoolVoidInfo = PoolVoidInfo(testPool);
    FreeBlock(&testBlock5);
    testPoolVoidInfo = PoolVoidInfo(testPool);
    FreeBlock(&testBlock8);
    testPoolVoidInfo = PoolVoidInfo(testPool);
    FreeBlock(&testBlock3);
    testPoolVoidInfo = PoolVoidInfo(testPool);

    testData = 14;
    for (uint32 i = 0; i < NumElements(testBlock2); i++)
    {
        Push(testBlock2, &testData);
        testData++;
    }
    testData = 14;
    for (uint32 i = 0; i < NumElements(testBlock4); i++)
    {
        Push(testBlock4, &testData);
        testData++;
    }
    testData = 14;
    for (uint32 i = 0; i < NumElements(testBlock6); i++)
    {
        Push(testBlock6, &testData);
        testData++;
    }
    testData = 14;
    for (uint32 i = 0; i < NumElements(testBlock7); i++)
    {
        Push(testBlock7, &testData);
        testData++;
    }
    
    ConsolidatePool(testPool);
    testPoolVoidInfo = PoolVoidInfo(testPool);

    uint32 dist1 = (handle)testBlock2 - (handle)zeroBlock - BlockSize(zeroBlock);
    uint32 dist2 = (handle)testBlock4 - (handle)testBlock2 - BlockSize(testBlock2);
    uint32 dist3 = (handle)testBlock6 - (handle)testBlock4 - BlockSize(testBlock4);
    uint32 dist4 = (handle)testBlock7 - (handle)testBlock6 - BlockSize(testBlock6);
   
    uint64 combinedSize = 0;
    char buffer[2 * sizeof(uint32)];
    OutputDebugStringA("Zero Block: \n");
    uint64 zeroBlockSize = BlockSize(zeroBlock);
    combinedSize += zeroBlockSize;
    for (uint32 i = 0; i < NumElements(zeroBlock); i++)
    {
        OutputDebugStringA(itoa(*(uint32*)GetElementAt(zeroBlock, i), buffer, 10));
        OutputDebugStringA("\n");
    }
    OutputDebugStringA("Block2: \n");
    uint64 testBlock2Size = BlockSize(testBlock2);
    combinedSize += testBlock2Size;
    for (uint32 i = 0; i < NumElements(testBlock2); i++)
    {
        OutputDebugStringA(itoa(*(uint32*)GetElementAt(testBlock2, i), buffer, 10));
        OutputDebugStringA("\n");
    }
    OutputDebugStringA("Block4: \n");
    uint64 testBlock4Size = BlockSize(testBlock4);
    combinedSize += testBlock4Size;
    for (uint32 i = 0; i < NumElements(testBlock4); i++)
    {
        OutputDebugStringA(itoa(*(uint32*)GetElementAt(testBlock4, i), buffer, 10));
        OutputDebugStringA("\n");
    }
    OutputDebugStringA("Block6: \n");
    uint64 testBlock6Size = BlockSize(testBlock6);
    combinedSize += testBlock6Size;
    for (uint32 i = 0; i < NumElements(testBlock6); i++)
    {
        OutputDebugStringA(itoa(*(uint32*)GetElementAt(testBlock6, i), buffer, 10));
        OutputDebugStringA("\n");
    }
    OutputDebugStringA("Block7: \n");
    uint64 testBlock7Size = BlockSize(testBlock7);
    combinedSize += testBlock7Size;
    for (uint32 i = 0; i < NumElements(testBlock7); i++)
    {
        OutputDebugStringA(itoa(*(uint32*)GetElementAt(testBlock7, i), buffer, 10));
        OutputDebugStringA("\n");
    }
    
    return 0;
}