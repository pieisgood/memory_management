#include<iostream>

#define Malloc(SIZE, TYPE, DESC) _Malloc(SIZE, TYPE, DESC, __FILE__, __LINE__)
#define Realloc(PTR, SIZE) _Realloc(PTR, SIZE, __FILE__, __LINE__)
//#define New(CLASS, TYPE, DESC) new(TYPE, DESC, __FILE__, __LINE__) CLASS
#define Delete delete

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

struct AllocDesc
{
    void* Ptr;        // Allocation address
    tU32 Size;        // Allocation size
    TagDesc* Tag;     // Pointer to the allocation category
    AllocDesc* Next;  // Next allocation in the hashtable
};

struct TagDesc
{
    tU32 CRC;         // Category description string CRC
    tU32 Size;        // Total size of all allocations in this category
    tChar Tag[128];   // Category name
    TagDesc* Next;    // Next category in the hashtable
};

//each of these allocation and deallocation functions records data about where allocations took place, their type and their size.
void* _Malloc( unsigned int Size, unsigned int AllocType, const char* Desc, const char* File, unsigned int Line);
void* _Realloc(void* Ptr, unsigned int Size, const char* File, unsigned int Line);
void _Free(void* Ptr);

//we overload new and delete so that when we call these operators we can record everything about the dynamic memory being requested.
inline void* operator new(size_t Size, unsigned int AllocType, const char* Desc, const char* File, unsigned int Line) { return _Malloc(Size, AllocType, Desc, File, Line); }
inline void* operator new[](size_t Size, unsigned int AllocType, const char* Desc, const char* File, unsigned int Line) { return _Malloc(Size, AllocType, Desc, File, Line); }
inline void operator delete(void* Ptr) { _Free(Ptr); }
inline void operator delete[](void* Ptr) { _Free(Ptr); }

template<class T> class FreeList
{
    T* Free;
 
public:
    FreeList()
    : Free(NULL)
    {}
 
    inline T* New()
    {
        if(!Free)
        {
            const tU32 AllocSize = 65536;
            const tU32 NumAllocPerBatch = AllocSize / sizeof(T);
 
            T* AllocBatch = (T*)VirtualAlloc(NULL, AllocSize, MEM_COMMIT, PAGE_READWRITE);
 
            for(tU32 i=0; i<NumAllocPerBatch; i++)
            {
                Delete(AllocBatch++);
            }
        }
 
        T* Result = Free;
        Free = *((T**)Free);
        return Result;
    }
 
    inline void Delete(T* Ptr)
    {
        *(T**)Ptr = Free;
        Free = Ptr;
    }
};

FreeList<AllocDesc> FreeAllocs;

void RegisterAlloc(void* Ptr, tU32 Size, const tChar* Desc, const tChar* File, tU32 Line)
{
    // Allocate a new AllocDesc from the freelist
    AllocDesc* NewAllocDesc = FreeAllocs.New();
 
    // Set the allocation info
    NewAllocDesc->Ptr = Ptr;
    NewAllocDesc->Size = Size;
    NewAllocDesc->File = File;
    NewAllocDesc->Line = Line;
 
    // Register the allocation into its category (this updates the category total size)
    NewAllocDesc->Tag = TagDesc::Register(Desc, Size);
 
    // Register the allocation in the fixed-size hashtable
    tU32 PtrHash = GetPtrHash(Ptr) & (ALLOC_DESC_SIZE - 1);
    NewAllocDesc->Next = Hash[PtrHash];
    Hash[PtrHash] = NewAllocDesc;
}

static TagDesc* TagDesc::Register(const tChar* Tag, tU32 Size)
{
    tU32 TagCRC = Strhash(Tag);
    tU32 TagHash = TagCRC & (ALLOC_DESC_SIZE - 1);
    TagDesc* CurTag = CurTag = Hash[TagHash];
 
    while(CurTag)
    {
        if(CurTag->CRC == TagCRC)
        {
            // This category already exist, update its size and return it
            CurTag->Size += Size;
            return CurTag;
        }
 
        CurTag = CurTag->Next;
    }
 
    // Allocate a new TagDesc from the freelist
    CurTag = FreeTags.Allocate();
 
    // Set the category info
    CurTag->CRC = TagCRC;
    CurTag->Size = Size;
 
    tU32 TagLen = Min(strlen(Tag), 127);
    memcpy(CurTag->Tag, Tag, TagLen);
    CurTag->Tag[TagLen] = 0;
 
    // Register the category in the fixed-size hashtable
    CurTag->Next = Hash[TagHash];
    Hash[TagHash] = CurTag;
 
    return CurTag;
}

void DumpAllocs()
{
    printf("Address,Category,CategorySize,AllocSize\n");
    for(tU32 i=0; i<ALLOC_DESC_SIZE; i++)
    {
        AllocDesc* Alloc = Hash[i];
        while(Alloc)
        {
            printf("0x%08x,%s,%d,%d\n", Alloc->Ptr, Alloc->Tag->Tag, Alloc->Tag->Size, Alloc->Size);
            Alloc = Alloc->Next;
        }
    }
}

void* _Malloc(tU32 Size, tU32 AllocType, const tChar* Desc, const tChar* File, tU32 Line)
{
    void* Result = malloc(Size);
    RegisterAlloc(Result, Size, AllocType, Desc, File, Line);
    return Result;
}
 
void* _Realloc(void* Ptr, tU32 Size, const tChar* File, tU32 Line)
{
    void* Result = realloc(Ptr, Size);
    UpdateAlloc(Ptr, Result, Size, File, Line);
    return Result;
}
 
void _Free(void* Ptr)
{
    UnregisterAlloc(Ptr);
    return free(Ptr);
}