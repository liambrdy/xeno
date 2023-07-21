#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <raylib.h>

#define Kilobytes(x) (x*1024)
#define Megabytes(x) (Kilobytes(x)*1024)
#define Gigabytes(x) (Megabytes(x)*1024)

#define ArrayCount(arr) (sizeof(arr)/sizeof(arr[0]))

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define CHUNK_UNINITIALIZED INT32_MAX

struct Chunk {
    i32 x, y;

    Chunk *next;
};

struct MemoryArena {
    u8 *memory;
    size_t capacity;
    size_t index;
};

struct World {
    Chunk chunks[4096];
};

void ArenaInit(MemoryArena *arena, void *mem, size_t capacity)
{
    arena->memory = (u8 *)mem;
    arena->capacity = capacity;
    arena->index = 0;
}

u8 *ArenaPush(MemoryArena *arena, size_t size)
{
    assert(arena->index + size < arena->capacity);
    u8 *ptr = arena->memory + arena->index;
    arena->index += size;
    return ptr;
}

void ArenaClear(MemoryArena *arena)
{
    arena->index = 0;
}

#define PushStruct(arena, type) (type *)ArenaPush(arena, sizeof(type))
#define PushArray(arena, type, count) (type *)ArenaPush(arena, count * sizeof(type))

Chunk *GetChunk(World *world, int x, int y, MemoryArena *arena)
{
    int slotHash = 7 * x + 2 * y;
    int slotIndex = slotHash & (ArrayCount(world->chunks)-1);
    assert(slotIndex < ArrayCount(world->chunks));

    Chunk *chunk = &world->chunks[slotIndex];

    do
    {
        if (chunk->x == x && chunk->y == y) {
            break;
        }

        if (chunk->x != CHUNK_UNINITIALIZED && !chunk->next) {
            chunk->next = PushStruct(arena, Chunk);
            chunk = chunk->next;
            chunk->x = CHUNK_UNINITIALIZED;
        }

        if (chunk->x == CHUNK_UNINITIALIZED) {
            chunk->x = x;
            chunk->y = y;

            chunk->next = 0;
            
            break;
        }

        chunk = chunk->next;
    } while (chunk);
    
    return chunk;
}

void InitializeWorld(World *world)
{
    for (u32 chunkIndex = 0; chunkIndex < ArrayCount(world->chunks); chunkIndex++) {
        world->chunks[chunkIndex].x = CHUNK_UNINITIALIZED;
        world->chunks[chunkIndex].next = 0;
    }
}

int main()
{
    InitWindow(800, 600, "Xeno");

    SetTargetFPS(60);

    size_t memoryBlockSize = Megabytes(100);
    void *memoryBlock = MemAlloc(memoryBlockSize);
    
    MemoryArena arena = {};
    ArenaInit(&arena, memoryBlock, memoryBlockSize);

    World *world = PushStruct(&arena, World);
    InitializeWorld(world);

    while (!WindowShouldClose()) {
        PollInputEvents();

        BeginDrawing();
        EndDrawing();
    }

    MemFree(memoryBlock);
    CloseWindow();

    return 0;
}