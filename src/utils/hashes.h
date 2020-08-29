//
//  hashes.h
//
//  Created by jenra on 9/1/17.
//

#ifndef hashes_h
#define hashes_h

#include <stdlib.h>

// The default hashing algorithm used.
#define hashing_function one_at_a_time_hash

// one_at_a_time_hash(void*, size_t) -> size_t
// One at a time hash algorithm, developed by Bob Jenkins.
//
// This is the default hashing algorithm used.
size_t one_at_a_time_hash(void* key, size_t length);

#endif /* hashes_h */
