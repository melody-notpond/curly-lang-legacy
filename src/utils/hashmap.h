//
//  hashmap.h
//
//  Created by jenra on 9/1/17.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stdlib.h>

// (void*, size_t) -> size_t
// Represents a hash function.
typedef size_t (*hash_func)(void*, size_t);

// Represents a bucket in a hashmap.
typedef struct hash_bucket
{
	// The key the bucket represents (used to resolve collisions).
	char* key;

	// The length of the key.
	size_t key_length;

	// The value the bucket holds.
	void* value;

	// The next bucket in the linked list of buckets with equivelent hash values.
	struct hash_bucket* next;

	// The last bucket in the linked list of buckets with equivelent hash values.
	struct hash_bucket* last;
} hash_bucket;

// Represents a hashmap.
typedef struct hashmap_t
{
	// The list of buckets.
	hash_bucket** buckets;

	// The number of items in the bucket list. (Includes collisions.)
	size_t item_count;

	// The number of buckets in the bucket list. (Does not include collisions.)
	size_t bucket_count;

	// The number of collisions in the bucket list.
	size_t collision_count;

	// The size of the bucket list.
	size_t buckets_size;

	// If the hashmap is currently being resized.
	bool resizing;

	// The hashing function used.
	hash_func function;
} hashmap_t;

// init_hashmap() -> hashmap_t*
// Initialises a hashmap.
hashmap_t* init_hashmap(void);

// map_resize(hashmap_t*, bool) -> void
// Resizes a hashmap and rehashes all the children.
void map_resize(hashmap_t* map, bool grow);

// map_add(hashmap_t*, char*, void*) -> void
// Adds an element to a hashmap.
void map_add(hashmap_t* map, char* key, void* value);

// map_addn(hashmap_t*, char*, size_t, void*) -> void
// Adds an element to a hashmap.
void map_addn(hashmap_t* map, char* key, size_t key_length, void* value);

// map_contains(hashmap_t*, char*) -> bool
// Returns true if the hashmap contains a given key.
bool map_contains(hashmap_t* map, char* key);

// map_containsn(hashmap_t*, char*, size_t) -> bool
// Returns true if the hashmap contains a given key.
bool map_containsn(hashmap_t* map, char* key, size_t key_length);

// map_keys(hashmap_t*, size_t*, size_t*) -> char**
// Returns a list of keys in the hashmap, updating the length and size parameters accordingly.
char** map_keys(hashmap_t* map, size_t* length, size_t* size);

// map_get(hashmap_t*, char*) -> void*
// Returns the value associated with the given key or NULL if it does not exist.
void* map_get(hashmap_t* map, char* key);

// map_getn(hashmap_t*, char*, size_t) -> void*
// Returns the value associated with the given key or NULL if it does not exist.
void* map_getn(hashmap_t* map, char* key, size_t key_length);

// map_remove(hashmap_t*, char*) -> void
// Removes a key from a hashmap.
void map_remove(hashmap_t* map, char* key);

// map_removen(hashmap_t*, char*, size_t) -> void
// Removes a key from a hashmap.
void map_removen(hashmap_t* map, char* key, size_t key_length);

// del_hashmap(hashmap_t*) -> void
// Deletes a hashmap.
void del_hashmap(hashmap_t* map);

#endif /* HASHMAP_H */
