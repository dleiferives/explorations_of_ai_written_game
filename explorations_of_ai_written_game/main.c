// ONE FILE GAME
// 2018-12-10
#define HASHMAP_ENTRY_TYPE_CHUNK_T 1
#define HASHMAP_ENTRY_TYPE_STRING 2
#define DH_PI 3.1415926535897932384626433832795
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

float interpolate(float a, float b, float blend);
int pow(int a, int b);
int factorial(int n);
double cos(double x);
float noise(float x, float y, float z, int seed);

// Random number generator
int randNum(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// blocks are a data structure
// blocks contain a block type, orientation
struct block_t_s
{
    unsigned int type : 12;
    int orientation : 4;
};
typedef union {
        struct block_t_s values;
        unsigned short data;
} block_t;



// Chunk_t data structure
// Chunk is 16x16x16
// Contains 16x16x16 blocks, and position
typedef struct {
    block_t blocks[16][16][16];
    int x;
    int y;
    int z;
} chunk_t;



// position_t data structure
typedef struct {
    int x;
    int y;
    int z;
} position_t;

// hashmap entry data structure
// contains a key, value
// value is a void pointer
typedef struct {
    int key;
    void *value;
} hashmap_entry_t;

// hash position function
// Hashes a position into a key
// uses exponentiation by squaring
int hash_position(position_t pos) {
    int key = 0;
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    int i = 0;
    for (i = 0; i < 32; i++) {
        if (x & 1) {
            key ^= 0x3b9aca07;
        }
        if (y & 1) {
            key ^= 0x61c88647;
        }
        if (z & 1) {
            key ^= 0x9e3779b9;
        }
        x >>= 1;
        y >>= 1;
        z >>= 1;
        key = (key << 1) | (key >> 31);
    }
    return key;
}


// hashmap data structure
// Contains a variable number of entries
// Contains function pointers to hashmap functions
// hash function, insert function, get function, remove function, free function, print function, resize function,rehash function,size function, contains function, clear function, empty function, get keys function, get entries function
typedef struct {
    hashmap_entry_t *entries;
    int entry_type;
    int entry_type_size;
    int size;
    int capacity;
    int (*hash)(hashmap_entry_t);
    void (*insert)(struct hashmap_t *, hashmap_entry_t);

    chunk_t (*get)(struct hashmap_t *, int);
    void (*remove)(struct hashmap_t *, int);
    void (*free)(struct hashmap_t *);
    void (*resize)(struct hashmap_t *, int);
    void (*rehash)(struct hashmap_t *);
    int (*contains)(struct hashmap_t *, int);
    void (*clear)(struct hashmap_t *);
    int (*empty)(struct hashmap_t *);
    position_t *(*get_keys)(struct hashmap_t *);
    hashmap_entry_t *(*get_entries)(struct hashmap_t *);
} hashmap_t;

// hashmap hash chunk function
// hashes a hashmap entry from its value
// enters computed hash into the entry's key
// assumes entry is a chunk
int hash_chunk(hashmap_entry_t entry) {
    chunk_t chunk = *(chunk_t *)entry.value;
    position_t pos = {chunk.x, chunk.y, chunk.z};
    return hash_position(pos);
}

// hashmap hash string function
// hashes a hashmap entry from its value
// enters computed hash into the entry's key
// assumes entry is a string
int hash_string(hashmap_entry_t entry) {
    char *str = (char *)entry.value;
    int hash = 0;
    int i = 0;
    for (i = 0; i < strlen(str); i++) {
        hash += str[i];
    }
    return hash;
}



// hashmap insert function
// inserts a value into the hashmap
// returns the key of the inserted value
int hashmap_insert(hashmap_t *map, hashmap_entry_t entry) {
    int index = map->hash(entry) % map->capacity;
    while (map->entries[index].key != 0) {
        index = (index + 1) % map->capacity;
    }
    entry.key = index;
    map->entries[index] = entry;
    map->size++;
    if (map->size >= map->capacity / 2) {
        map->resize(map, map->capacity * 2);
    }
    return index;
}

// hashmap get function
// gets a value from the hashmap
chunk_t hashmap_get(hashmap_t *map, int key) {
    int index = key % map->capacity;
    while (map->entries[index].key != 0) {
        if (map->entries[index].key == key) {
            return *(chunk_t *)map->entries[index].value;
        }
        index = (index + 1) % map->capacity;
    }
    return (chunk_t){0};
}

// hashmap remove function
// removes a key, value pair from the hashmap
void hashmap_remove(hashmap_t *map, int key) {
    int index = key % map->capacity;
    while (map->entries[index].key != 0) {
        if (map->entries[index].key == key) {
            map->entries[index].key = 0;
            map->entries[index].value = NULL;
            map->size--;
            return;
        }
        index = (index + 1) % map->capacity;
    }
}

// hashmap free function
// frees the hashmap
void hashmap_free(hashmap_t *map) {
    free(map->entries);
    free(map);
}

// hashmap resize function
// resizes the hashmap
void hashmap_resize(hashmap_t *map, int capacity) {
    hashmap_entry_t *entries = malloc(sizeof(hashmap_entry_t) * capacity);
    int i = 0;
    for (i = 0; i < capacity; i++) {
        entries[i].key = 0;
        entries[i].value = NULL;
    }
    int old_capacity = map->capacity;
    map->capacity = capacity;
    hashmap_entry_t *old_entries = map->entries;
    map->entries = entries;
    map->size = 0;
    for (i = 0; i < old_capacity; i++) {
        if (old_entries[i].key != 0) {
            map->insert(map, old_entries[i]);
        }
    }
    free(old_entries);
}

// hashmap rehash function
// rehashes the hashmap
void hashmap_rehash(hashmap_t *map) {
    hashmap_entry_t *entries = malloc(sizeof(hashmap_entry_t) * map->capacity);
    int i = 0;
    for (i = 0; i < map->capacity; i++) {
        entries[i].key = 0;
        entries[i].value = NULL;
    }
    hashmap_entry_t *old_entries = map->entries;
    map->entries = entries;
    map->size = 0;
    for (i = 0; i < map->capacity; i++) {
        if (old_entries[i].key != 0) {
            map->insert(map, old_entries[i]);
        }
    }
    free(old_entries);
}

// hashmap contains function
// checks if the hashmap contains a key
int hashmap_contains(hashmap_t *map, int key) {
    int index = key % map->capacity;
    while (map->entries[index].key != 0) {
        if (map->entries[index].key == key) {
            return 1;
        }
        index = (index + 1) % map->capacity;
    }
    return 0;
}

// hashmap clear function
// clears the hashmap
void hashmap_clear(hashmap_t *map) {
    int i = 0;
    for (i = 0; i < map->capacity; i++) {
        map->entries[i].key = 0;
        map->entries[i].value = NULL;
    }
    map->size = 0;
}

// hashmap empty function
// checks if the hashmap is empty
int hashmap_empty(hashmap_t *map) {
    return map->size == 0;
}

// hashmap get keys function
// gets all the keys in the hashmap
int *hashmap_get_keys(hashmap_t *map) {
    int *keys = malloc(map->entry_type_size * map->size);
    int i = 0;
    int j = 0;
    for (i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != 0) {
            keys[j] = map->entries[i].key;
            j++;
        }
    }
    return keys;
}

// hashmap get entries function
// gets all the entries in the hashmap
hashmap_entry_t *hashmap_get_entries(hashmap_t *map) {
    hashmap_entry_t *entries = malloc(sizeof(hashmap_entry_t) * map->size);
    int i = 0;
    int j = 0;
    for (i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != 0) {
            entries[j] = map->entries[i];
            j++;
        }
    }
    return entries;
}

// hashmap constructor
// creates a new hashmap
hashmap_t *hashmap_new(int type, int type_size) {
    hashmap_t *map = malloc(sizeof(hashmap_t));
    map->capacity = 16;
    map->size = 0;
    map->entries = malloc(sizeof(hashmap_entry_t) * map->capacity);
    int i = 0;
    for (i = 0; i < map->capacity; i++) {
        map->entries[i].key = 0;
        map->entries[i].value = NULL;
    }
    map->insert = hashmap_insert;
    map->get = hashmap_get;
    map->remove = hashmap_remove;
    map->free = hashmap_free;
    map->resize = hashmap_resize;
    map->rehash = hashmap_rehash;
    map->contains = hashmap_contains;
    map->clear = hashmap_clear;
    map->empty = hashmap_empty;
    map->get_keys = hashmap_get_keys;
    map->get_entries = hashmap_get_entries;
    map->entry_type = type;
    map->entry_type_size = type_size;
    return map;
}

// hashmap entry constructor for chunk_t
// creates a new hashmap entry for chunk_t
hashmap_entry_t hashmap_entry_new_chunk_t(chunk_t value) {
    hashmap_entry_t entry;
    entry.key = -1;
    entry.value = malloc(sizeof(chunk_t));
    *(chunk_t *)entry.value = value;
    return entry;
}

// hashmap entry constructor for strings
// creates a new hashmap entry for strings
hashmap_entry_t hashmap_entry_new_string(char *value) {
    hashmap_entry_t entry;
    entry.key = -1;
    entry.value = malloc(sizeof(char) * (strlen(value) + 1));
    strcpy(entry.value, value);
    return entry;
}

// hashmap generator for chunk_t
// generates a hashmap for chunk_t
hashmap_t *hashmap_new_chunk_t() {
    hashmap_t *map = hashmap_new(HASHMAP_ENTRY_TYPE_CHUNK_T, sizeof(chunk_t));
    map->hash = hash_chunk;
    return map;
}

// hashmap generator for strings
// generates a hashmap for strings
hashmap_t *hashmap_new_string() {
    hashmap_t *map = hashmap_new(HASHMAP_ENTRY_TYPE_STRING, sizeof(char));
    map->hash = hash_string;
    return map;
}

// convert a hashmap entry to a chunk_t
// converts a hashmap entry to a chunk_t
chunk_t hashmap_entry_to_chunk_t(hashmap_entry_t entry) {
    return *(chunk_t *)entry.value;
}

// converts a hashmap entry to a string
// converts a hashmap entry to a string
char *hashmap_entry_to_string(hashmap_entry_t entry) {
    return entry.value;
}



// world_t data structure
// Contains information about the world
// Contains infinite number of chunks stored in hashmap
typedef struct
{
    // map of chunk positions to chunks
    hashmap_t chunks;
    int seed;
    int size;

    // map of chunk positions to world_data files
    hashmap_t world_data;


    int (*get)(struct world_t *world, int x, int y, int z);

    void (*set)(struct world_t *world, int x, int y, int z, int block);

    void (*generate)(struct world_t *world, int x, int y, int z);

    void (*free)(struct world_t *world);

    void (*save)(struct world_t *world);

    void (*load)(struct world_t *world);

    void (*generate_chunk)(struct world_t *world, int x, int y, int z);

    void (*generate_chunk_threaded)(struct world_t *world, int x, int y, int z);

    void (*generate_chunk_at_threaded)(struct world_t *world, int x, int y, int z);
    // loads chunk from a world_data file if it exists and returns it to the world hashmap
    void (*load_chunk)(struct world_t *world, int x, int y, int z);

    void (*unload_chunk)(struct world_t *world, int x, int y, int z);
    // saves chunk to a world_data file
    void (*save_chunk)(struct world_t *world, int x, int y, int z);
    // saves all chunks to world_data files
    void (*save_all_chunks)(struct world_t *world);


} world_t;

// chunk file storage format
// stores a chunk in a file
// stores 64x64x64 chunk into a file
// each chunk is stored in a stream of bytes
// the bytes are stored in the following order:
// the first 4 bytes are the x position of the chunk
// the next 4 bytes are the y position of the chunk
// the next 4 bytes are the z position of the chunk
// the next 4 bytes are set to 0
// each of the following 2 bytes are the blocks read into the file


unsigned char *compress_chunk_t(chunk_t chunk) {
    unsigned char * result = malloc(sizeof(char) * (4 + 4 + 4 + (16*16*16*2)));
    int it = 0;
    // write in x y z
    result[it++] = (chunk.x & 0xFF000000 ) >> 24;
    result[it++] = (chunk.x & 0x00FF0000) >> 16;
    result[it++] = (chunk.x & 0x0000FF00) >> 8;
    result[it++] = (chunk.x & 0x000000FF) >> 0;
    result[it++] = (chunk.y & 0xFF000000 ) >> 24;
    result[it++] = (chunk.y & 0x00FF0000) >> 16;
    result[it++] = (chunk.y & 0x0000FF00) >> 8;
    result[it++] = (chunk.y & 0x000000FF) >> 0;
    result[it++] = (chunk.z & 0xFF000000 ) >> 24;
    result[it++] = (chunk.z & 0x00FF0000) >> 16;
    result[it++] = (chunk.z & 0x0000FF00) >> 8;
    result[it++] = (chunk.z & 0x000000FF) >> 0;

    // write in block data
    // if next n blocks is of same type write a 1 then the next 15 bits represent how many blocks are the same following it
    // if next n bloack is not of same type (raw writing) write a 0 then the next 15 bits describe how many blocks are not the same following it.
    // takes two bytes to setup a chain. a block takes up 2 bytes,
    // for three blocks of the same in a row, -> 6 bytes if raw written. or 4 bytes if chained. therefore, any grouping of 3 or more in a row, denotes a chain should be used.
    // assuming most blocks should be of same type

    // read until block changes
    int i = 0;
    unsigned short start;// = !((unsigned short *)chunk.blocks)[i++];
    int start_index = 0;
    // XOOXOOXOOXOOXOOX
    int diff_start = 0;
    int dif_loops = 0;
    while(i<16*16*16)
    {
        // start index starts on new block type.
        start_index = i;
        start = !((unsigned short *)chunk.blocks)[i++];

        //find # of following blocks that are same as start
        int same_block_counter = 0;
        for(int k =i; k<16*16*16; k++)
        {
            if ((start & ((unsigned short *) chunk.blocks)[k]) != 0)
            {
                same_block_counter++;
            }
        }
        // if less than three, not worth making string, continue on past then.
        if (same_block_counter < 2)
        {
            if(dif_loops == 0) diff_start = start_index;
            dif_loops++;
        }
        else
        {
            if ((start_index - diff_start) >  0)
            {
                for (int w_it = 0; w_it < start_index -1 - diff_start; w_it++)
                {
                    result[it++] = ((unsigned short *) chunk.blocks)[w_it + diff_start] & 0xFF00;
                    result[it++] = ((unsigned short *) chunk.blocks)[w_it + diff_start] & 0x00FF;
                }

                //write from diff_start for [start_index -1 -diff_start]
            }
            for (int w_it = 0; w_it < same_block_counter; w_it++)
            {
                result[it++] = ((unsigned short *) chunk.blocks)[w_it + start_index] & 0xFF00;
                result[it++] = ((unsigned short *) chunk.blocks)[w_it + start_index] & 0x00FF;
            }
            //write from start_index for same_block_counter
            dif_loops = 0;
        }

        i+=1 + same_block_counter;
    }
    if (dif_loops > 0)
    {
        for (int w_it = diff_start; w_it < 16*16*16; w_it++)
        {
            result[it++] = ((unsigned short *) chunk.blocks)[w_it] & 0xFF00;
            result[it++] = ((unsigned short *) chunk.blocks)[w_it] & 0x00FF;
        }
    }
    unsigned char * ret_val = malloc(sizeof(char) * (it));
    for(int i = 0; i < it;i++)
    {
        ret_val[i] = result[i];
    }

    return ret_val;
}

// decompression algorithm for chunk_t
// decompresses a char array into a chunk_t
chunk_t decompress_chunk_t(char *compressed) {
    chunk_t chunk;
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    int m = 0;
    int n = 0;
    for (i = 0; i < 8; i++) {
        chunk.x |= compressed[i] << (i * 8);
    }
    for (i = 0; i < 8; i++) {
        chunk.y |= compressed[i + 8] << (i * 8);
    }
    for (i = 0; i < 8; i++) {
        chunk.z |= compressed[i + 16] << (i * 8);
    }
}






// random number generator
// generates a random number between min and max
int rand_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// smooth noise function
// generates smooth noise for a given x, y, z u
float smooth_noise(float x, float y, float z, int seed) {
    float corners = (noise(x - 1, y - 1, z - 1, seed) + noise(x + 1, y - 1, z - 1, seed) + noise(x - 1, y + 1, z - 1, seed) + noise(x + 1, y + 1, z - 1, seed) + noise(x - 1, y - 1, z + 1, seed) + noise(x + 1, y - 1, z + 1, seed) + noise(x - 1, y + 1, z + 1, seed) + noise(x + 1, y + 1, z + 1, seed)) / 16;
    float sides = (noise(x - 1, y, z - 1, seed) + noise(x + 1, y, z - 1, seed) + noise(x, y - 1, z - 1, seed) + noise(x, y + 1, z - 1, seed) + noise(x - 1, y, z + 1, seed) + noise(x + 1, y, z + 1, seed) + noise(x, y - 1, z + 1, seed) + noise(x, y + 1, z + 1, seed) + noise(x - 1, y - 1, z, seed) + noise(x + 1, y - 1, z, seed) + noise(x - 1, y + 1, z, seed) + noise(x + 1, y + 1, z, seed)) / 8;
    float center = noise(x, y, z, seed) / 4;
    return corners + sides + center;
}

// interpolated noise function
// generates interpolated noise
float interpolated_noise(float x, float y, float z, int seed) {
    int int_x = (int)x;
    float frac_x = x - int_x;
    int int_y = (int)y;
    float frac_y = y - int_y;
    int int_z = (int)z;
    float frac_z = z - int_z;
    float v1 = smooth_noise(int_x, int_y, int_z, seed);
    float v2 = smooth_noise(int_x + 1, int_y, int_z, seed);
    float v3 = smooth_noise(int_x, int_y + 1, int_z, seed);
    float v4 = smooth_noise(int_x + 1, int_y + 1, int_z, seed);
    float v5 = smooth_noise(int_x, int_y, int_z + 1, seed);
    float v6 = smooth_noise(int_x + 1, int_y, int_z + 1, seed);
    float v7 = smooth_noise(int_x, int_y + 1, int_z + 1, seed);
    float v8 = smooth_noise(int_x + 1, int_y + 1, int_z + 1, seed);
    float i1 = interpolate(v1, v2, frac_x);
    float i2 = interpolate(v3, v4, frac_x);
    float i3 = interpolate(v5, v6, frac_x);
    float i4 = interpolate(v7, v8, frac_x);
    float i5 = interpolate(i1, i2, frac_y);
    float i6 = interpolate(i3, i4, frac_y);
    return interpolate(i5, i6, frac_z);
}

// perlin noise function
// generates perlin noise
float perlin_noise(float x, float y, float z, int seed) {
    float total = 0;
    float p = 0.5;
    int n = 4;
    for (int i = 0; i < n; i++) {
        float frequency = pow(2, i);
        float amplitude = pow(p, i);
        total = total + interpolated_noise(x * frequency, y * frequency, z * frequency, seed) * amplitude;
    }
    return total;
}

// interpolate function
// interpolates between two values


float interpolate(float a, float b, float blend)
{
    double theta = blend * DH_PI;
    float f = (1 - cos(theta)) * 0.5;
    return a * (1 - f) + b * f;
}


// fast power for ints
// returns a^b
int pow(int a, int b)
{
    int result = 1;
    while (b > 0) {
        if (b & 1) {
            result *= a;
        }
        b >>= 1;
        a *= a;
    }
    return result;
}

// cosine funciton for doubles
// returns cos(x)
// uses taylor series
double cos(double x)
{
    double result = 0;
    for (int i = 0; i < 10; i++) {
        result += pow(-1, i) * pow(x, 2 * i) / factorial(2 * i);
    }
    return result;
}

// factorial function for ints
// returns n!
int factorial(int n)
{
    int result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

// noise function
// generates noise for a given x, y, z and seed
float noise(float x, float y, float z, int seed) {
    int n = x + y * 57 + z * 57 * 57 + seed;
    n = (n << 13) ^ n;
    return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

// layered noise function
// layers noise together
float layered_noise(float x, float y, float z, int seed) {
    float total = 0;
    float d = (float)pow(2, 4 - 1);
    for (int i = 0; i < 4; i++) {
        float freq = (float)(pow(2, i) / d);
        float amp = (float)pow(2, i - 1);
        total = total + noise(x * freq, y * freq, z * freq, seed) * amp;
    }
    return total;
}

// plains height noise function
// generates height noise for plains biome
// uses constant values for noise generation
float plains_height_noise(float x, float y, float z, int seed) {
    float total = 0;
    float d = (float)pow(2, 4 - 1);
    for (int i = 0; i < 4; i++) {
        float freq = (float)(pow(2, i) / d);
        float amp = (float)pow(2, i - 1);
        total = total + noise(x * freq, y * freq, z * freq, seed) * amp;
    }
    return total;
}


// generate height function
// generates height for a given x, y, z and seed
float generate_height(float x, float y, float z, int seed) {
    float height = 0;
    height = plains_height_noise(x, y, z, seed);
    return height;
}

// generate chunk function
// generates a surface of height for all values of x and y in a chunk, the result is normalized to a value between 0 and 16, and the result is stored in the chunk at the given x and y, at the z value of the height of the surface at the given x and y in the chunk
// all block_t above the surface are set to 1,0 , all blocks below the surface are set to 0,0
chunk_t generate_chunk(int seed) {
    chunk_t chunk;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            float height = generate_height(x + chunk.x * 16, y + chunk.y * 16, 0, seed);
            int height_int = (int)height;
            for (int z = 0; z < 16; z++) {
                if (z < height_int) {
                    chunk.blocks[x][y][z].type = 0;
                    chunk.blocks[x][y][z].orientation = 0;
                }
//                else if (z == height_int) {
//                    chunk->blocks[x][y][z].id = 1;
//                    chunk->blocks[x][y][z].data = 0;
//                }
                else {
                    chunk.blocks[x][y][z].type = 1;
                    chunk.blocks[x][y][z].orientation = 0;
                }
            }
        }
    }
    return chunk;
}

// world generate chunk function
// Parameters: world_t* world, int x, int y, int z
// Returns: void
// Functionality: generates a chunk. Stores the chunk in the world's hashmap at the given x, y, z coordinates cast to a position_t
void world_generate_chunk(world_t* world, int x, int y, int z) {
    position_t position;
    position.x = x;
    position.y = y;
    position.z = z;
    chunk_t chunk = generate_chunk(world->seed);
    chunk.x = x;
    chunk.y = y;
    chunk.z = z;
    // stores the chunk in the world's hashmap using the position as the key
    world->chunks.insert(&(world->chunks), hashmap_entry_new_chunk_t(chunk));
}

// load chunk function
// loads a chunk from a file



// random chunk generator
// generates a random chunk
chunk_t random_chunk() {
    chunk_t chunk;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                chunk.blocks[x][y][z].type = rand() % 2;
                chunk.blocks[x][y][z].orientation = rand() % 2;
            }
        }
    }
    return chunk;
}




















//


