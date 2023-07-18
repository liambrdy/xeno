#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <raylib.h>

#define Kilobytes(x) (x*1024)
#define Megabytes(x) (Kilobytes(x)*1024)
#define Gigabytes(x) (Megabytes(x)*1024)

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

typedef struct {

} Entity;

typedef struct {
    int x, y;
    Entity *entities;
} Chunk;

typedef struct WorldChunk {
    Chunk c;
    struct WorldChunk *next;
} WorldChunk;

typedef struct {
    u8 *memory;
    size_t capacity;
    size_t index;
} MemoryArena;

typedef struct {
    WorldChunk *worldChunks;
    size_t chunkCount;

    MemoryArena arena;
} World;

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

#define PushStruct(arena, type) (type *)ArenaPush(&arena, sizeof(type))
#define PushArray(arena, type, count) (type *)ArenaPush(&arena, count * sizeof(type))

Tile *GetTile(World *world, int x, int y, TileType type)
{
    int slotHash = 7 * x + 2 * y;
    int slotIndex = slotHash & 1024;
    assert(slotIndex < 1024);

    Tile *result = NULL;

    WorldTile worldTile = world->worldTiles[slotIndex];
    if (worldTile.t.x == x && worldTile.t.y == y && worldTile.t.type == type) {
        result = &worldTile.t;
    } else {
        while (worldTile.next) {
            if (worldTile.t.x == x && worldTile.t.y == y && worldTile.t.type == type) {
                result = &worldTile.t;
                break;
            }
            worldTile = *worldTile.next;
        }
    }

    if (!result) {
        Tile t = {};
        t.x = x;
        t.y = y;
        t.type = type;
        worldTile.next = PushStruct(world->arena, WorldTile);
        worldTile.next->next = NULL;
        worldTile.next->t = t;
        result = &worldTile.next->t;
        world->tileCount++;
    }

    return result;
}

int main()
{
    InitWindow(800, 600, "Xeno");

    SetTargetFPS(60);

    size_t memoryBlockSize = Megabytes(100);
    void *memoryBlock = MemAlloc(memoryBlockSize);
    
    MemoryArena arena = {};
    ArenaInit(&arena, memoryBlock, memoryBlockSize);

    World *world = PushStruct(arena, World); 
    world->worldTiles = PushArray(arena, WorldTile, 1024);
    world->tileCount = 0;

    while (!WindowShouldClose()) {
        PollInputEvents();

        BeginDrawing();
        EndDrawing();
    }

    MemFree(memoryBlock);
    CloseWindow();

    return 0;
}