#include<iostream>
#include<Windows.h>

typedef unsigned char       tU8;
typedef signed char         tS8;
typedef unsigned short      tU16;
typedef signed short        tS16;
typedef unsigned long       tU32;
typedef signed long         tS32;
typedef int                 tInt;
typedef unsigned __int64    tU64;
typedef signed __int64      tS64;
typedef bool                tBool;
typedef float               tFloat;
typedef double              tDouble;
typedef char                tChar;
typedef unsigned short      tWChar;

typedef StrictSegHeap<
    10, 
    StrictSegHeapTraits,
    ThreadSafeHeap<PagedFreeList>,
    ThreadSafeHeap<CRTMallocHeap>
> GlobalHeap;

class CRTMallocHeap
{
public:
    inline void* Malloc(tU32 Size)
    {
        return malloc(Size);
    }
 
    inline void* Realloc(void* Ptr, tU32 Size)
    {
        return realloc(Ptr, Size);
    }
 
    inline void Free(void* Ptr)
    {
        return free(Ptr);
    }
};

typedef CRTMallocHeap GlobalHeap;
 
GlobalHeap GHeap;
 
void* _Malloc(tU32 Size, tU32 AllocType, const tChar* Desc, const tChar* File, tU32 Line)
{
    void* Result = GHeap.Malloc(Size);
    RegisterAlloc(Result, Size, AllocType, Desc, File, Line);
    return Result;
};
 
void* _Realloc(void* Ptr, tU32 Size, const tChar* File, tU32 Line)
{
    void* Result = GHeap.Realloc(Ptr, Size);
    UpdateAlloc(Ptr, Result, Size, File, Line);
    return Result;
};
 
void _Free(void* Ptr)
{
    UnregisterAlloc(Ptr);
    return GHeap.Free(Ptr);
};

class PagedFreeList
{
    tU8* FirstFree;
 
public:
    PagedFreeList()
    : FirstFree(NULL)
    {}
 
    inline void* Malloc(tU32 Size)
    {
        if(!FirstFree)
        {
            const tU32 PageSize = 65536;
            const tU32 NumAllocPerBatch = PageSize / Size;
 
            // Allocate a 64K page from the OS
            tU8* AllocBatch = (tU8*)VirtualAlloc(NULL, PageSize, MEM_COMMIT, PAGE_READWRITE);
 
            for(tU32 i=0; i<NumAllocPerBatch; i++)
            {
                Free(AllocBatch);
                AllocBatch += Size;
            }
        }
 
        tU8* Result = FirstFree;
        FirstFree = *((tU8**)FirstFree);
        return Result;
    }
 
    inline void Free(void* Ptr)
    {
        *(tU8**)Ptr = FirstFree;
        FirstFree = (tU8*)Ptr;
    }
};

template<class SuperHeap> class SizeHeap: public SuperHeap
{
    struct FreeObject
    {
        tU32 Size;
    };
 
public:
    inline void* Malloc(tU32 Size)
    {
        FreeObject* Ptr = (FreeObject*)SuperHeap::Malloc(Size + sizeof(FreeObject));
        if(Ptr)
        {
            Ptr->Size = Size;
        }
        return (void*)(Ptr + 1);
    }
 
    inline void* Realloc(void* Ptr, tU32 Size)
    {
        FreeObject* NewPtr = (FreeObject*)SuperHeap::Realloc(Ptr ? (FreeObject*)Ptr - 1 : NULL, Size + sizeof(FreeObject));
        if(NewPtr)
        {
            NewPtr->Size = Size;
        }
        return (void*)(NewPtr + 1);
    }
 
    inline void Free(void* Ptr)
    {
        SuperHeap::Free((FreeObject*)Ptr - 1);
    }
 
    inline tU32 GetSize(void* Ptr)
    {
        return ((FreeObject*)Ptr - 1)->Size;
    }
};