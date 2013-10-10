#ifndef DOUBLEBUFFERALLOCATOR
#define DOUBLEBUFFERALLOCATOR
#include "StackAllocator.h"

class DoubleBufferedAllocator
{
private:

	unsigned int m_currStack;
	StackAllocator m_stack[2];

public:

	void swapBuffers()
	{
		m_currStack = (unsigned int) !m_currStack;
	}

	void clearCurrentBuffer()
	{
		m_stack[m_currStack].clear();
	}

	void* alloc( unsigned int nBytes )
	{
		return m_stack[m_currStack].alloc(nBytes);
	}

	//... ect. Setup and Shutdown and other functions
}

#endif