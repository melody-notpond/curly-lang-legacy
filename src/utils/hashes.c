//
//  hashes.c
//
//  Created by jenra on 9/1/17.
//

#include "hashes.h"

// one_at_a_time_hash(void*, size_t) -> size_t
// One at a time hash algorithm; developed by Bob Jenkins.
//
// This is the default hashing algorithm used.
size_t one_at_a_time_hash(void* key, size_t length)
{
	unsigned char* p = key;
	size_t hash = 0;
	for (size_t i = 0; i < length; i++)
	{
		hash += p[i];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}
