#include "MemoryPool.h"
#include <memory>

bool MemoryPool::GrowMemoryArray(void)
{
	size_t allocSize = sizeof(unsigned char*)*(m_memoryArraySize + 1);
	unsigned char** ppNewMemArray = (unsigned char**) malloc(allocSize);

	if(!ppNewMemArray)
	{
		return false;
	}

	for( unsigned int i = 0; i < m_memoryArraySize; ++i)
	{
		ppNewMemArray[i] = m_ppRawMemoryArray[i];
	}

	ppNewMemArray[m_memoryArraySize] = AllocateNewMemoryBlock();

	if(m_pHead)
	{
		unsigned char* pCurr = m_pHead;
		unsigned char* pNext = GetNext(m_pHead);

		while(pNext)
		{
			pCurr = pNext;
			pNext = GetNext(pNext);
		}

		SetNext(pCurr, ppNewMemArray[m_memoryArraySize]);
	}
	else
	{
		m_pHead = ppNewMemArray[m_memoryArraySize];
	}

	if(m_ppRawMemoryArray)
	{
		free(m_ppRawMemoryArray);
	}

	m_ppRawMemoryArray = ppNewMemArray;
	++m_memoryArraySize;

	return true;
}

unsigned char* MemoryPool::AllocateNewMemoryBlock(void)
{
	size_t blockSize = m_chunkSize + CHUNK_HEADER_SIZE; //CHUNK HEADER SIZE will be a defined type of pointer
	size_t trueSize = blockSize * m_numChunks;

	unsigned char* pNewMem = (unsigned char*) malloc(trueSize);

	if( !pNewMem)
	{
		return NULL;
	}

	unsigned char* pEnd = pNewMem + trueSize;
	unsigned char* pCurr = pNewMem;

	while( pCurr < pEnd )
	{
		unsigned char* pNext = pCurr + blockSize;

		unsigned char** ppChunkHeader = (unsigned char**) pCurr;
		ppChunkHeader[0] = (pNext < pEnd ? pNext : NULL);

		pCurr += blockSize;
	}

	return pNewMem;
}

void* MemoryPool::Alloc(void)
{
	if( !m_pHead )
	{
		if(!m_toAllowResize)
		{
			return NULL;
		}

		if( !GrowMemoryArray() )
		{
			return NULL;
		}
	}

	unsigned char* pRet = m_pHead;

	m_pHead = GetNext(m_pHead);
	return (pRet + CHUNK_HEADER_SIZE); //will add chunk header size down the road
}

void MemoryPool::Free(void* pMem)
{
	if( pMem != NULL)
	{
		unsigned char* pBlock = ((unsigned char*)pMem) - CHUNK_HEADER_SIZE;

		SetNext(pBlock, m_pHead);
		m_pHead = pBlock;
	}
}

unsigned int MemoryPool::GetChunkSize(void)
{
	return m_chunkSize;
}

unsigned char* MemoryPool::GetNext(unsigned char* pBlock)
{
	return (unsigned char*) 2;
}

void MemoryPool::SetNext(unsigned char* pBlockToChange, unsigned char* pNewNext){

}


