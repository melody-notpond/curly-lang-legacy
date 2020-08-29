//
//  hashmap.c
//
//  Created by jenra on 9/1/17.
//

#include <string.h>

#include "hashes.h"
#include "hashmap.h"

// The initial size of a hashmap.
#define INITIAL_HASHMAP_SIZE 16

// The maximum number of collisions in a hashmap before a resize occurs.
#define HASHMAP_MAX_COLLISIONS 5 // TODO: Adjust.

// The infix function used to make the hashmap grow.
#define HASHMAP_GROWTH_FUNCTION << 1

// The infix function used to make the hashmap shrink.
#define HASHMAP_SHRINK_FUNCTION >> 1

// init_hash_bucket(char*, size_t, void*) -> hash_bucket*
// Initialises a hashmap bucket.
hash_bucket* init_hash_bucket(char* key, size_t key_length, void* value)
{
	hash_bucket* bucket = malloc(sizeof(hash_bucket));
	bucket->key = strndup(key, key_length);
	bucket->key_length = key_length;
	bucket->value = value;
	bucket->next = NULL;
	bucket->last = bucket;
	return bucket;
}

// del_hash_bucket(hash_bucket*) -> hash_bucket*
// Deletes a hashmap bucket, returning the next bucket with the same hash value. Note that values are not freed, since a hashmap does not know what type the value is and if it needs a special method to free it correctly, or even if the value is still needed!
hash_bucket* del_hash_bucket(hash_bucket* bucket)
{
	free(bucket->key);
	hash_bucket* next = bucket->next;
	free(bucket);
	return next;
}

// init_hashmap() -> hashmap_t*
// Initialises a hashmap.
hashmap_t* init_hashmap()
{
	hashmap_t* map = malloc(sizeof(hashmap_t));
	map->buckets = calloc(INITIAL_HASHMAP_SIZE, sizeof(hash_bucket*));
	map->item_count = 0;
	map->bucket_count = 0;
	map->collision_count = 0;
	map->buckets_size = INITIAL_HASHMAP_SIZE;
	map->function = hashing_function;
	map->resizing = false;
	return map;
}

// map_resize(hashmap_t*, bool) -> void
// Resizes a hashmap and rehashes all the children.
void map_resize(hashmap_t* map, bool grow)
{
	map->resizing = true;
	hash_bucket* all_buckets = NULL;
	for (size_t i = 0; i < map->buckets_size; i++)
	{
		hash_bucket* current = map->buckets[i];
		if (all_buckets == NULL)
			all_buckets = current;
		else
		{
			all_buckets->last->next = current;
			all_buckets->last = current->last;
		}
	}
	if (grow)
		map->buckets_size = map->buckets_size HASHMAP_GROWTH_FUNCTION;
	else
		map->buckets_size = map->buckets_size HASHMAP_SHRINK_FUNCTION;
	map->buckets = reallocf(map->buckets, map->buckets_size * sizeof(hash_bucket*));

	// I didn't want to deal with repeating such a large block of code, so here is a malloc-heavy and free-heavy loop.
	while (all_buckets != NULL)
	{
		map_addn(map, all_buckets->key, all_buckets->key_length, all_buckets->value);
		all_buckets = del_hash_bucket(all_buckets);
	}
	map->resizing = false;
}

// map_add(hashmap_t*, char*, void*) -> void
// Adds an element to a hashmap.
void map_add(hashmap_t* map, char* key, void* value)
{
	map_addn(map, key, strlen(key), value);
}

// map_addn(hashmap_t*, char*, size_t, void*) -> void
// Adds an element to a hashmap.
void map_addn(hashmap_t* map, char* key, size_t key_length, void* value)
{
	if (!map->resizing && map->collision_count > HASHMAP_MAX_COLLISIONS)
		map_resize(map, true);
	size_t index = map->function(key, key_length) % map->buckets_size;
	if (map->buckets[index] == NULL)
	{
		map->buckets[index] = init_hash_bucket(key, key_length, value);
		map->bucket_count++;
		map->item_count++;
	} else
	{
		hash_bucket* first = map->buckets[index];
		hash_bucket* current = first;
		while (true)
		{
			if (key_length == current->key_length && !strncmp(key, current->key, key_length))
			{
				current->value = value;
				break;
			} else if (current->next == NULL)
			{
				current->next = init_hash_bucket(key, key_length, value);
				first->last = current->next;
				map->item_count++;
				map->collision_count++;
				break;
			}
			current = current->next;
		}
	}
}

// map_contains(hashmap_t*, char*) -> bool
// Returns true if the hashmap contains a given key.
bool map_contains(hashmap_t* map, char* key)
{
	return map_containsn(map, key, strlen(key));
}

// map_containsn(hashmap_t*, char*, size_t) -> bool
// Returns true if the hashmap contains a given key.
bool map_containsn(hashmap_t* map, char* key, size_t key_length)
{
	size_t index = map->function(key, key_length) % map->buckets_size;
	hash_bucket* current = map->buckets[index];
	while (current != NULL)
	{
		if (key_length == current->key_length && !strncmp(key, current->key, key_length))
		{
			return true;
		}
		current = current->next;
	}
	return false;
}

// map_get(hashmap_t*, char*) -> void*
// Returns the value associated with the given key or NULL if it does not exist.
void* map_get(hashmap_t* map, char* key)
{
	return map_getn(map, key, strlen(key));
}

// map_getn(hashmap_t*, char*, size_t) -> void*
// Returns the value associated with the given key or NULL if it does not exist.
void* map_getn(hashmap_t* map, char* key, size_t key_length)
{
	size_t index = map->function(key, key_length) % map->buckets_size;
	hash_bucket* current = map->buckets[index];
	while (current != NULL)
	{
		if (key_length == current->key_length && !strncmp(key, current->key, key_length))
		{
			return current->value;
		}
		current = current->next;
	}
	return NULL;
}

// map_remove(hashmap_t*, char*) -> void
// Removes a key from a hashmap.
void map_remove(hashmap_t* map, char* key)
{
	map_removen(map, key, strlen(key));
}

// map_removen(hashmap_t*, char*, size_t) -> void
// Removes a key from a hashmap.
void map_removen(hashmap_t* map, char* key, size_t key_length)
{
	if (!map->resizing && map->item_count < map->buckets_size >> 5) // 16 seems like a good absolute minimum
		map_resize(map, false);
	size_t index = map->function(key, key_length) % map->buckets_size;
	hash_bucket* first = map->buckets[index];
	hash_bucket* current = first;
	hash_bucket* previous = NULL;
	while (current != NULL)
	{
		if (key_length == current->key_length && !strncmp(key, current->key, key_length))
		{
			if (current != first)
			{
				if (current->next == NULL)
					first->last = previous;
				previous->next = del_hash_bucket(current);
			}
			else
			{
				hash_bucket* last = first->last;
				first = del_hash_bucket(current);
				if (first != NULL)
					first->last = last;
				map->buckets[index] = first;
			}
			break;
		}
		previous = current;
		current = current->next;
	}
}

// del_hashmap(hashmap_t*) -> void
// Deletes a hashmap.
void del_hashmap(hashmap_t* map)
{
	for (size_t i = 0; i < map->buckets_size; i++)
	{
		hash_bucket* current = map->buckets[i];
		while (current != NULL)
			current = del_hash_bucket(current);
	}
	free(map->buckets);
	free(map);
}
