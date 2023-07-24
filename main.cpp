#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <raylib.h>

#include "types.h"
#include "maths.h"

struct MemoryArena {
    u8 *memory;
    size_t capacity;
    size_t index;
};

enum EntityType {
    EntityTypeWall,
    EntityTypePlayer,
};

struct Entity {
    EntityType type;
    v2 pos;
    v2 dPos;
};

struct GameState {
    Entity entities[10000];
    u32 entityCount;

    u32 hotEntities[1024];
    u32 hotEntityCount;

    Camera2D cam;

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

u32 AddEntity(GameState *state, Entity e) {
    assert(state->entityCount < ArrayCount(state->entities));

    u32 index = state->entityCount;
    state->entities[state->entityCount++] = e;

    return index;
}

u32 AddWall(GameState *state, v2 p) {
    Entity e = {};
    e.pos = p;
    e.type = EntityTypeWall;

    return AddEntity(state, e);
}

u32 AddPlayer(GameState *state, v2 p) {
    Entity e = {};
    e.pos = p;
    e.type = EntityTypePlayer;

    return AddEntity(state, e);
}

#define TILE_SIZE 20
#define ROOM_SIZE 20

void SetCamera(GameState *state, v2 center, f32 width, f32 height) {
    state->cam.offset = (Vector2){ width / 2.0f, height / 2.0f };
    state->cam.target = (Vector2){ center.x, center.y };
    state->cam.rotation = 0.0f;
    state->cam.zoom = 1.0f;

    rectangle2 cameraSpace;
    cameraSpace.min = V2(state->cam.target.x - state->cam.offset.x, state->cam.target.y - state->cam.offset.y) - V2(TILE_SIZE, TILE_SIZE);
    cameraSpace.max = V2(state->cam.target.x + state->cam.offset.x, state->cam.target.y + state->cam.offset.y) + V2(TILE_SIZE, TILE_SIZE);

    state->hotEntityCount = 0;
    for (u32 i = 0; i < state->entityCount; i++) {
        Entity e = state->entities[i];
        v2 pixelPos = e.pos;
        pixelPos.x += TILE_SIZE / 2.0f;
        pixelPos.y += TILE_SIZE / 2.0f;

        if (IsInRectangle(cameraSpace, pixelPos)) {
            state->hotEntities[state->hotEntityCount++] = i;
        }
    }
}

void MoveEntity(GameState *state, u32 entityIndex, float dt, v2 ddP) {
    Entity *e = state->entities + entityIndex;

    float length = Length(ddP);
    if (length > 1.0f) {
        ddP *= (1.0f / sqrtf(length));
    }

    float speed = 500.0f;
    ddP *= speed;

    ddP += -8.0f * e->dPos;

    v2 delta = ddP * dt * dt * 0.5f + e->dPos * dt;
    e->pos = e->pos + delta;
    e->dPos = ddP * dt + e->dPos;
}

int main()
{
    i32 width = 800, height = 600;
    InitWindow(width, height, "Xeno");

    SetTargetFPS(60);

    size_t memoryBlockSize = Megabytes(100);
    u8 *memoryBlock = (u8 *)MemAlloc(memoryBlockSize);
    
    GameState *gameState = (GameState *)memoryBlock;
    ArenaInit(&gameState->arena, memoryBlock + sizeof(GameState), memoryBlockSize - sizeof(GameState));

    for (i32 i = -100; i < 100; i++) {
        if (i == 0 || i % ROOM_SIZE == 0) {
            for (i32 j = 0; j <= ROOM_SIZE; j++) {
                if (j != ROOM_SIZE/2) {
                    AddWall(gameState, V2((f32)i * TILE_SIZE, (f32)j * TILE_SIZE));
                }
            }
        } else {
            AddWall(gameState, V2((f32)i * TILE_SIZE, 0));
            AddWall(gameState, V2((f32)i * TILE_SIZE, ROOM_SIZE * TILE_SIZE));
        }
    }

    u32 playerIndex = AddPlayer(gameState, V2(50, 50));

    v2 target = V2(0, 0);
    SetCamera(gameState, target, (f32) width, (f32) height);

    while (!WindowShouldClose()) {
        u32 renderCount = 0;

        if (IsKeyDown(KEY_RIGHT)) { target.x += 1.0f; SetCamera(gameState, target, (f32) width, (f32) height); }
        if (IsKeyDown(KEY_LEFT)) { target.x -= 1.0f; SetCamera(gameState, target, (f32) width, (f32) height); }
        if (IsKeyDown(KEY_UP)) { target.y -= 1.0f; SetCamera(gameState, target, (f32) width, (f32) height); }
        if (IsKeyDown(KEY_DOWN)) { target.y += 1.0f; SetCamera(gameState, target, (f32) width, (f32) height); }

        v2 ddP = {};
        if (IsKeyDown(KEY_A)) ddP.x = -1;
        if (IsKeyDown(KEY_D)) ddP.x = 1;
        if (IsKeyDown(KEY_S)) ddP.y = 1;
        if (IsKeyDown(KEY_W)) ddP.y = -1;
        if (ddP.x != 0 || ddP.y != 0) {MoveEntity(gameState, playerIndex, 1/60.0f, ddP); printf("%f, %f\n", gameState->entities[playerIndex].dPos.x, gameState->entities[playerIndex].dPos.y);}

        float begin = GetTime();
        BeginDrawing();
        ClearBackground(GRAY);

        BeginMode2D(gameState->cam);
        for (u32 entityIndex = 0; entityIndex < gameState->hotEntityCount; entityIndex++) {
            Entity e = gameState->entities[gameState->hotEntities[entityIndex]];
            switch (e.type) {
                case EntityTypeWall: {
                    DrawRectangle(e.pos.x, e.pos.y, TILE_SIZE, TILE_SIZE, WHITE);
                    renderCount++;
                } break;
                case EntityTypePlayer: {
                    DrawRectangle(e.pos.x, e.pos.y, TILE_SIZE, TILE_SIZE, ORANGE);
                    renderCount++;
                } break;
                default: break;
            }
        }
        EndMode2D();
        EndDrawing();

        float end = GetTime();
        float past = end - begin;
        printf("Drew %d objects in %f seconds\n", renderCount, past);
    }

    MemFree(memoryBlock);
    CloseWindow();

    return 0;
}