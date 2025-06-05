#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>

#define WIDTH 320
#define HEIGHT 240
#define SCALE 4
#define G_WIDTH WIDTH * SCALE
#define G_HEIGHT HEIGHT * SCALE

#define ROWS 12
#define COLS 16
#define TILE_WIDTH WIDTH / COLS
#define TILE_HEIGHT HEIGHT / ROWS

#define NUM_MAPS 1

typedef enum {
    EMPTY,
    PLAYER,
    NUM_ENTITY_TYPES
} Entity_Type; 

typedef struct entity {
    int pos[2];
    Entity_Type type;
    bool walkable;
} Entity;

typedef struct tile {
    Entity entity;
} Tile;

typedef struct map {
    Tile tiles[COLS][ROWS];
    int biome;
} Map;

typedef struct game_state {
    Entity player;
    Map maps[NUM_MAPS];
    Texture2D images[NUM_ENTITY_TYPES];
} GameState;

GameState game;

Map empty_map(void)
{
    Map result = {0};

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            result.tiles[x][y] = (Tile) {
                .entity = (Entity) {
                    .pos = {x, y},
                    .type = EMPTY,
                    .walkable = true
                }
            };
            result.biome = 0;
        }
    }
    return result;
}

void reset_game(GameState* g)
{
    g->player = (Entity) {
        .pos = {5, 5},
        .type = PLAYER,
        .walkable = false
    };
    g->maps[0] = empty_map();

}

int main (int argc, char *argv[])
{
    (void)argc; (void)argv;

    InitWindow(G_WIDTH, G_HEIGHT, "the traders");
    SetTargetFPS(60);

    RenderTexture2D screen = LoadRenderTexture(WIDTH, HEIGHT);
    SetTextureFilter(screen.texture, TEXTURE_FILTER_POINT);

    reset_game(&game);

    bool screen_change = true;

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_RIGHT)) {
            game.player.pos[0] += 1;

        }

    //Drawing to 320x240 render texture
        BeginTextureMode(screen);
            ClearBackground(BLACK);
            for (int i = 0; i < ROWS; i++) {
                for (int j = 0; j < COLS; j++) {
                    Map m = game.maps[0];
                    Tile m_entity = m.tiles[j][i];
                    switch (m_entity.entity.type) {
                        case EMPTY:
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
                            break;
                        default:
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
                    }
                }
            }
            Entity p = game.player;
            DrawRectangle(p.pos[0] * TILE_WIDTH, p.pos[1] * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, DARKGREEN);
        EndTextureMode();

        //Drawing to frame buffer
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(
                screen.texture,
                (Rectangle){0, 0, WIDTH, -HEIGHT},
                (Rectangle){0, 0, G_WIDTH, G_HEIGHT},
                (Vector2){0,0},
                0.0f,
                WHITE
            );
            
        EndDrawing();
    }

    UnloadRenderTexture(screen);
    CloseWindow();
    
}