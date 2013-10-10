#include <iostream>
#include <memory>

void* operator new(size_t size)
{
	return malloc(size);
}

void operator delete(void* mem)
{
	free(mem);
}

void* operator new[](size_t size)
{
    return malloc(size);
}

void operator delete[](void* mem)
{
    free(mem);
}

class Align
{
public:
    explicit Align(int value) : m_value(value)
    {
    }

    int GetValue() const
    {
       return m_value;
    }

private:
    int m_value;
};

void* operator new[] (size_t size, Align alignment);
void* operator new (size_t size, Align alignment);
void operator delete (void* mem, Align alignment);
void operator delete[] (void* mem, Align alignment);

//-----------------------------------------------------
//------------ ALIGNED MEMORY ALLOCATION --------------
//-----------------------------------------------------

void* allocateAligned(unsigned int size, Align alignment)
{
	//make sure our user has given us a reasonable request
	_ASSERT(alignment.GetValue() > 1 );

	unsigned int expandedSize_bytes = size + alignment.GetValue();

	unsigned int rawAddress = (unsigned int) new(size);
	unsigned int mask = (alignment.GetValue() - 1 );
	unsigned int misalignment = (rawAddress & mask);
	unsigned int adjustment = alignment.GetValue() - misalignment;

	unsigned int alignedAddress = rawAddress + adjustment;

	unsigned int* pAdjustment = (unsigned int*) (alignedAddress - 4);
	*pAdjustment = adjustment;

	return (void*)alignedAddress;
}

//-----------------------------------------------------------
//-------------- ALIGNED MEMORY FREE ------------------------
//-----------------------------------------------------------

void freeAligned(void* p)
{
	unsigned int alignedAddress = (unsigned int) p;
	unsigned char* pAdjustment = (unsigned char*) (alignedAddress - 4);
	unsigned int adjustment = (unsigned int) *pAdjustment;
	unsigned int rawAddress = alignedAddress - adjustment;
	free((void*)rawAddress);
}