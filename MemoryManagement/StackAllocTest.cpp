#include "StackAllocator.h"

int main(){
	StackAllocator test_stack(64);

	int* i_ptr = (int*) test_stack.alloc( sizeof(int));
	*i_ptr = 5;

	double* c_ptr = (double*) test_stack.alloc( sizeof(double) );
	*c_ptr = 3.0;

	double* d_ptr = (double*) test_stack.alloc( sizeof(double) );
	*d_ptr = 4.235;

	int x;

	std::cout << i_ptr << " : Address \n" << *i_ptr << " : value \n"
		<< c_ptr << " : Address \n" << *c_ptr << " : value \n"
		<< d_ptr << " : Address \n" << *d_ptr << " : value \n"
		<< test_stack.getMarker() << " : current Marker \n";

	unsigned int current = test_stack.getMarker();
	std::cout << std::hex <<current - 8 << " : Marker after shift \n";
	test_stack.freeToMarker( (unsigned int) d_ptr);
	std::cout << test_stack.getMarker() << " : Marker after release of memory \n";

	test_stack.clear();

	std::cout << test_stack.getMarker() << " : Current marker after clear";

	std::cin >> x;

	return 0;
}