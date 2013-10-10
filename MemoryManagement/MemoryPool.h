#ifndef MEMORYPOOL
#define MEMORYPOOL

unsigned int CHUNK_HEADER_SIZE = 1;

class MemoryPool
{
private:
	unsigned char** m_ppRawMemoryArray; //an array of memory blocks, eacg split up into chunks
	unsigned char* m_pHead; //the front of the memory chunk linked list
	unsigned int m_chunkSize, m_numChunks; //the size of each chunk and number of chunks per array
	unsigned int m_memoryArraySize; //the number of elements in the memory array
	bool m_toAllowResize; //true if we resize the memory pool when it fills

public:
	MemoryPool(void);
	~MemoryPool(void);
	bool Init(unsigned int chenkSize, unsigned int numChunks);
	void Destroy(void);

	//allocation funcs
	void* Alloc(void); //done
	void Free(void* pMem); //done
	unsigned int GetChunkSize(void); //done
	void SetAllowResize(bool toAllowResize)
	{
		m_toAllowResize = toAllowResize;
	} //done

private:
	void Reset(void);

	//internal memory allocation helper
	bool GrowMemoryArray(void); //done
	unsigned char* AllocateNewMemoryBlock(void); //done

	//internal linked list management
	unsigned char* GetNext(unsigned char* pBlock);
	void SetNext(unsigned char* pBlockToChange, unsigned char* pNewNext);

	//dont allow copy constructor
	MemoryPool(const MemoryPool & memPool) {} //advice from Effective C++ :)
};


#endif