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

    bool collides;

    f32 width, height;
};

struct GameState {
    Entity entities[10000];
    u32 entityCount;

    u32 hotEntities[1024];
    u32 hotEntityCount;

    Camera2D cam;

    MemoryArena arena;

    f32 metersToPixels;
    f32 tileSideInMeters;
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

#define TILE_SIZE 20
#define ROOM_SIZE 20

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
    e.width = state->tileSideInMeters;
    e.height = e.width;
    e.collides = true;

    return AddEntity(state, e);
}

u32 AddPlayer(GameState *state, v2 p) {
    Entity e = {};
    e.pos = p;
    e.type = EntityTypePlayer;
    e.height = 1.0f;
    e.width = 0.5f;
    e.collides = true;

    return AddEntity(state, e);
}

void SetCamera(GameState *state, v2 center, f32 width, f32 height) {
    state->cam.offset = (Vector2){ width / 2.0f, height / 2.0f };
    state->cam.target = (Vector2){ center.x * state->metersToPixels, center.y * state->metersToPixels };
    state->cam.rotation = 0.0f;
    state->cam.zoom = 1.0f;

    rectangle2 cameraSpace;
    cameraSpace.min = V2(state->cam.target.x - state->cam.offset.x, state->cam.target.y - state->cam.offset.y) * (1/state->metersToPixels) - V2(state->tileSideInMeters, state->tileSideInMeters);
    cameraSpace.max = V2(state->cam.target.x + state->cam.offset.x, state->cam.target.y + state->cam.offset.y) * (1/state->metersToPixels) + V2(state->tileSideInMeters, state->tileSideInMeters);

    state->hotEntityCount = 0;
    for (u32 i = 0; i < state->entityCount; i++) {
        Entity e = state->entities[i];
        v2 pos = e.pos;
        pos.x += state->tileSideInMeters / 2.0f;
        pos.y += state->tileSideInMeters / 2.0f;

        if (IsInRectangle(cameraSpace, pos)) {
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

    float speed = 50.0f;
    ddP *= speed;

    ddP += -8.0f * e->dPos;

    v2 delta = ddP * dt * dt * 0.5f + e->dPos * dt;
    e->dPos = ddP * dt + e->dPos;
    
    v2 newPosition = e->pos + delta;

    for (u32 i = 0; i < state->hotEntityCount; i++) {
        if (entityIndex != state->hotEntities[i]) {
            Entity *comp = state->entities + state->hotEntities[i];
            if (comp->collides) {
                v2 oPos = comp->pos;

                if (oPos.x <= newPosition.x+e->width && oPos.x+comp->width >= newPosition.x && oPos.y+comp->height >= newPosition.y && oPos.y <= newPosition.y+e->height) {
                    newPosition = e->pos;
                }
            }
        }
    }

    e->pos = newPosition;
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

    i32 tileSideInPixels = 60;
    gameState->tileSideInMeters = 1.5f;
    gameState->metersToPixels = (f32)tileSideInPixels / gameState->tileSideInMeters;

    i32 screenX = 0;
    i32 screenY = 0;
    i32 tilesPerWidth = 17;
    i32 tilesPerHeight = 9;

    bool doorLeft = false, doorRight = false, doorTop = false, doorBottom = false;
    for (i32 screenIndex = 0; screenIndex < 10; screenIndex++) {
        i32 randomChoice = GetRandomValue(0, 1);
        if (randomChoice == 0) {
            doorRight = true;
        } else {
            doorTop = true;
        }
        
        for (i32 y = 0; y < tilesPerHeight; y++) {
            for (i32 x = 0; x < tilesPerWidth; x++) {
                i32 tileX = screenX*tilesPerWidth + x;
                i32 tileY = screenY*tilesPerHeight + y;

                i32 tileValue = 1;

                if (x == 0 && (!doorLeft || y != (tilesPerHeight/2))) {
                    tileValue = 2;
                }

                if (x == (tilesPerWidth - 1) && (!doorRight || y != (tilesPerHeight/2))) {
                    tileValue = 2;
                }

                if (y == 0 && (!doorBottom || x != (tilesPerWidth / 2))) {
                    tileValue = 2;
                }

                if (y == (tilesPerHeight - 1) && (!doorTop || x != (tilesPerWidth/2))) {
                    tileValue = 2;
                }

                if (tileValue == 2) {
                    AddWall(gameState, V2(tileX, tileY)*gameState->tileSideInMeters);
                }
            }
        }

        doorLeft = doorRight;
        doorBottom = doorTop;

        doorRight = false;
        doorTop = false;

        if (randomChoice == 0) screenX++;
        else screenY++;
    }

    u32 playerIndex = AddPlayer(gameState, V2(2, 2));

    v2 target = gameState->entities[playerIndex].pos;
    SetCamera(gameState, target, (f32) width, (f32) height);

    while (!WindowShouldClose()) {
        u32 renderCount = 0;

        // f32 camSpeed = 5.0f;
        // if (IsKeyDown(KEY_RIGHT)) { target.x += camSpeed; SetCamera(gameState, target, (f32) width, (f32) height); }
        // if (IsKeyDown(KEY_LEFT)) { target.x -= camSpeed; SetCamera(gameState, target, (f32) width, (f32) height); }
        // if (IsKeyDown(KEY_UP)) { target.y -= camSpeed; SetCamera(gameState, target, (f32) width, (f32) height); }
        // if (IsKeyDown(KEY_DOWN)) { target.y += camSpeed; SetCamera(gameState, target, (f32) width, (f32) height); }

        v2 ddP = {};
        if (IsKeyDown(KEY_A)) ddP.x = -1;
        if (IsKeyDown(KEY_D)) ddP.x = 1;
        if (IsKeyDown(KEY_S)) ddP.y = 1;
        if (IsKeyDown(KEY_W)) ddP.y = -1;
        if (ddP.x != 0 || ddP.y != 0) {
            MoveEntity(gameState, playerIndex, 1/60.0f, ddP);
            SetCamera(gameState, gameState->entities[playerIndex].pos, width, height);
        }

        float begin = GetTime();
        BeginDrawing();
        ClearBackground(GRAY);

        BeginMode2D(gameState->cam);
        for (u32 entityIndex = 0; entityIndex < gameState->hotEntityCount; entityIndex++) {
            Entity e = gameState->entities[gameState->hotEntities[entityIndex]];
            v2 pixelPosition = e.pos * gameState->metersToPixels;
            v2 dim = V2(e.width, e.height) * gameState->metersToPixels;
            switch (e.type) {
                case EntityTypeWall: {
                    DrawRectangle(pixelPosition.x, pixelPosition.y, dim.x, dim.y, WHITE);
                    renderCount++;
                } break;
                case EntityTypePlayer: {
                    DrawRectangle(pixelPosition.x, pixelPosition.y, dim.x, dim.y, ORANGE);
                    renderCount++;
                } break;
                default: break;
            }
            // DrawRectangleLines(pixelPosition.x, pixelPosition.y, e.width, e.height, BLACK);
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