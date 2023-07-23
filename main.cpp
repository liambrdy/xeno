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

struct MemoryArena {
    u8 *memory;
    size_t capacity;
    size_t index;
};

struct GameState {
    MemoryArena arena;
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

int main()
{
    // InitWindow(800, 600, "Xeno");

    // SetTargetFPS(60);

    size_t memoryBlockSize = Megabytes(100);
    u8 *memoryBlock = (u8 *)MemAlloc(memoryBlockSize);
    
    GameState *gameState = (GameState *)memoryBlock;
    ArenaInit(&gameState->arena, memoryBlock + sizeof(GameState), memoryBlockSize - sizeof(GameState));

    // while (!WindowShouldClose()) {
    //     PollInputEvents();

    //     BeginDrawing();
    //     EndDrawing();
    // }

    MemFree(memoryBlock);
    // CloseWindow();

    return 0;
}