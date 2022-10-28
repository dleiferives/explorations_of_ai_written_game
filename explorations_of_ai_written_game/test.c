#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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

chunk_t random_chunk(void);
unsigned char *compress_chunk_t(chunk_t chunk, int *pInt);
chunk_t decompress_chunk_t(unsigned char *b, int size);

int main()
{
    // init rand
    srand(time(NULL));
    chunk_t a = random_chunk();
    for(int i =0; i < 16*16*16*2; i++)
    {
        printf("%i%i", ((block_t *)a.blocks)[i].data & 0xFF00, ((block_t *)a.blocks)[i].data & 0x00FF);
    }
    putchar(10);
    putchar(10);
    putchar(10);
    putchar(10);
    int it = 0;
    unsigned char *b = compress_chunk_t(a, &it);
    for(int i =0; i < 16*16*16*2; i++)
    {
        printf("%i", b[i]);
    }
    putchar(10);
    putchar(10);
    chunk_t r = decompress_chunk_t(b,it);
    for(int i =0; i < 16*16*16*2; i++)
    {
        printf("%i%i", ((block_t *)r.blocks)[i].data & 0xFF00, ((block_t *)a.blocks)[i].data & 0x00FF);
    }
    putchar(10);
    putchar(10);
    putchar(10);


}



unsigned char *compress_chunk_t(chunk_t chunk, int * passback_size) {
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
    unsigned char * ret_val = malloc(sizeof(char) * (it+1));
    for(int i = 0; i < it+1;i++)
    {
        ret_val[i] = result[i];
    }
    ret_val[it] = '\0';
    *passback_size = it;
    return ret_val;
}


chunk_t random_chunk() {
    chunk_t chunk;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                chunk.blocks[x][y][z].values.type = rand() % 512;
                chunk.blocks[x][y][z].values.orientation = rand() % 2;
            }
        }
    }
    return chunk;
}

// a functino to decompress a chunk_t from a compressed chunk_t stored in a string.
// returns a chunk_t
// writes out data according to the following format:
// 4 bytes for x
// 4 bytes for y
// 4 bytes for z
// 4 bytes for empty space
// 2 bytes LSB ==1 for repeating block type the rest is the number of blocks of that type
// 2 bytes LSB ==0 for raw block type the rest is the number of blocks of that type
chunk_t decompress_chunk_t(unsigned char *b, int size)
{
    chunk_t chunk;
    int it = 0;
    // write in x y z
    chunk.x = (((int)b[it++]) << 24) | (((int)b[it++]) << 16) | (((int)b[it++]) << 8) | (((int)b[it++]));
    chunk.y = (((int)b[it++]) << 24) | (((int)b[it++]) << 16) | (((int)b[it++]) << 8) | (((int)b[it++]));
    chunk.z = (((int)b[it++]) << 24) | (((int)b[it++]) << 16) | (((int)b[it++]) << 8) | (((int)b[it++]));
    it += 4; // skip 4 bytes

    int i = it;
    int j = 0;
    while(i < size)
    {
        if (b[i] == 0)
        {
            int num_blocks = (b[i+1] << 8) | (b[i+2]);
            for (int k = 0; k < num_blocks; k++)
            {
                ((block_t *)chunk.blocks[j])->values.type = b[i+3] << 8;
                ((block_t *)chunk.blocks[j])->values.orientation = b[i+4];
                j++;
            }
            i+=5;
        }
        else
        {
            int num_blocks = (b[i+1] << 8) | (b[i+2]);
            for (int k = 0; k < num_blocks; k++)
            {
                ((block_t *)chunk.blocks[j])->values.type = b[i+3] << 8;
                ((block_t *)chunk.blocks[j])->values.orientation = b[i+4];
                j++;
            }
            i+=5;
        }
    }
    return chunk;
}


