#include "hash_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* map_malloc(size_t bytes) {
	void* result = malloc(bytes);
	if (result == NULL) {
		printf("Map malloc error\n");
		exit(1);
	}
	return result;
}

static void init_hash_table(kv_pair* table, int capacity) {
	for (int i = 0; i < capacity; i++)
		table[i].hash_val = EMPTY;
}

// Initialise the map with capacity LOAD_MULT * size
void init_map(hash_map* map, int size, hash_func_t hash, comp_t comparator) {
	if (map == NULL || hash == NULL || comparator == NULL || size < 0)
		return;

	if (size < 5)
		size = 5;

	map->hash = hash;
	map->comparator = comparator;

	map->capacity = (int)(size / LOAD_FACT);
	map->limit = (int)(LIMIT_FACT * map->capacity);
	
	map->table = map_malloc(sizeof(kv_pair) * map->capacity);
	init_hash_table(map->table, map->capacity);
	map->size = 0;	
}

void free_map(hash_map* map) {
	if (map == NULL || map->table == NULL)
		return;
	
	free(map->table);
	map->table = NULL;
}	


static void resize_put(hash_map* map, kv_pair entry) {
	int i = entry.hash_val % map->capacity;
	while (1) {
		if (map->table[i].hash_val == EMPTY) {
			map->table[i] = entry;
			return;
		}
		i = (i + 1) % map->capacity;
	}
}

static void resize(hash_map* map) {
	int old_capacity = map->capacity;
	kv_pair* old_table = map->table;

	map->capacity = old_capacity * RESIZE_FACT;
	map->limit = (int)(LIMIT_FACT * map->capacity);

	map->table = map_malloc(sizeof(kv_pair) * map->capacity);
	init_hash_table(map->table, map->capacity);

	for (int i = 0; i < old_capacity; i++)
		if (old_table[i].hash_val != EMPTY &&
			old_table[i].hash_val != DEFUNCT)
			resize_put(map, old_table[i]);
	
	free(old_table);
}

void* put(hash_map* map, void* key, void* value) { // returns original value if already in map
	if (map == NULL || map->table == NULL)
		return NULL;
	
	if (map->size == map->limit)
		resize(map);

	int hash_val = map->hash(key);
	if (hash_val < 0) {
		printf("Invalid Hash Function\n");
		return NULL;
	}
	
	kv_pair* table = map->table;

	for (int i = hash_val % map->capacity; ; i++) {
		if (i == map->capacity)
			i = 0;

		if (table[i].hash_val == EMPTY || table[i].hash_val == DEFUNCT) {
			table[i].hash_val = hash_val;
			table[i].key = key;
			table[i].value = value;
			map->size++;
			return NULL;
		}

		// update value
		if (table[i].hash_val == hash_val &&
			!map->comparator(table[i].key, key)) {
			void* old = table[i].value;
			table[i].value = value;
			return old;
		}
	}
}

void* get(hash_map* map, void* key) {	// returns value
	if (map == NULL || map->table == NULL)
		return NULL;

	int hash_val = map->hash(key);
	if (hash_val < 0) {
		printf("Invalid Hash Function\n");
		return NULL;
	}

	kv_pair* table = map->table;
	
	int index = hash_val % map->capacity;
	int i = index;
	do {
		if (table[i].hash_val == EMPTY)
			return NULL;

			else if (table[i].hash_val != DEFUNCT &&
				 table[i].hash_val == hash_val &&
				 !map->comparator(table[i].key, key))
				return table[i].value;

		i = (i + 1) % map->capacity;
	} while (i != index);
	
	return NULL;
}


void* del(hash_map* map, void* key) {	// returns value
	if (map == NULL || map->table == NULL)
		return NULL;

	int hash_val = map->hash(key);
	if (hash_val < 0) {
		printf("Invalid Hash Function\n");
		return NULL;
	}

	kv_pair* table = map->table;
	
	int index = hash_val % map->capacity;
	int i = index;
	do {
		if (table[i].hash_val == EMPTY)
			return NULL;

		else if (table[i].hash_val != DEFUNCT &&
				 table[i].hash_val == hash_val &&
				 !map->comparator(table[i].key, key)) {

			void* value = table[i].value;
			table[i].hash_val = DEFUNCT;
			map->size--;
			return value;
		}

		i = (i + 1) % map->capacity;
	} while (i != index);
	
	return NULL;
}
