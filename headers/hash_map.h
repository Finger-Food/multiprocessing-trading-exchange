#ifndef HASH_MAP_H
#define HASH_MAP_H

#define LOAD_FACT (0.5)
#define LIMIT_FACT (0.6)
#define RESIZE_FACT (2)	// factor to increase the array by when at size limit
#define DEFUNCT (-1)
#define EMPTY (-2)

typedef int (*hash_func_t)(void* data);	// should not return a negative number

typedef int (*comp_t)(void* a, void* b); // returns >0, 0, <0 respectively when a > b, a == b, a < b

typedef struct key_value_pair {
	int hash_val;
	void* key;
	void* value;
} kv_pair;

typedef struct linear_probing_hash_map {
	int capacity;
	int limit;
	int size;
	hash_func_t hash;
	comp_t comparator;
	kv_pair* table;
} hash_map;

extern void init_map(hash_map* map, int size, hash_func_t hash, comp_t comparator);	// init function

extern void free_map(hash_map* map); // frees map and its dynamic memory

extern void* put(hash_map* map, void* key, void* item);	// returns original value if already in map

extern void* get(hash_map*, void* key);	// returns value

extern void* del(hash_map* map, void* key);	// returns value

#endif
