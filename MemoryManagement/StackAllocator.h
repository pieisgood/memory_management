#ifndef STACKALLOCATOR
#define STACKALLOCATOR
#include<iostream>
#include<memory>
//does not need to be a template class, memory is type agnostic

class StackAllocator {
public:
	typedef unsigned int Marker; //current top of the stack

	//Constructor works!
	explicit StackAllocator(unsigned int stackSize_bytes)
	{
		//might be a bad idea?
		if( stackSize_bytes == 0 )
		{
			return;
		}

		m_marker = (Marker) malloc(stackSize_bytes);
		m_total_bytes = stackSize_bytes;
		m_curr_bytes = 0;
	}//construct a stack allocator with given total size

	//Alloc works! 
	void* alloc(unsigned int size_bytes){
		if(size_bytes == 0 || size_bytes > (m_total_bytes - m_curr_bytes))
		{
			return NULL;
		}

		unsigned int data_ptr = m_marker;
		m_marker = size_bytes + data_ptr;
		m_curr_bytes = m_curr_bytes + size_bytes;
		return (void*) data_ptr;
	}//allocate a new block of the given size

	Marker getMarker() { return m_marker; } //return current marker
	//Free to marker does not work... gotta figure out why.
	void freeToMarker(Marker marker)
	{
		free((void*)marker);
		m_curr_bytes = m_curr_bytes - (m_marker - marker);
		m_marker = marker;

	}//roll the stack back to a previous marker

	//clear works!
	void clear()
		{

			m_marker = m_marker - m_curr_bytes;
			free((void*)m_marker);
			m_marker = 0;
			m_total_bytes = 0;
			m_curr_bytes = 0;
	}//clear the entire stack
	
private:

	//prevent copying of our class
	//if we allowed copying, we would be able to overwrite mem addresses
	//which would be fucking horrible to debug and cause all sorts of problems.
	StackAllocator( StackAllocator &rhs){} 

	Marker m_marker;
	unsigned int m_total_bytes;
	unsigned int m_curr_bytes;
};

#endif