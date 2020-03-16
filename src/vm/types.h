//
// Curly
// types.h: Defines various types used by the virtual machine.
//
// jenra
// March 14 2020
//

#ifndef curly_types_h
#define curly_types_h

#include <inttypes.h>

typedef union
{
	int64_t	i64;
	double	f64;
	char*	str;
	void*	ptr;
} cvalue_t;

#endif /* curly_types_h */
