#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include "main.h"

GameState game;
Entity entities[NUM_ENTITY_TYPES];

void create_entities(Entity* e)
{
    e[EMPTY] = (Entity) {
        .pos = {0,0},
        .type = EMPTY,
        .walkable = true
    };

    e[PLAYER] = (Entity) {
        .pos = {0,0},
        .type = PLAYER,
        .walkable = false
    };

    e[TREE] = (Entity) {
        .pos = {0,0},
        .type = TREE,
        .walkable = false
    };
}

Map empty_map(void)
{
    Map result = {0};

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            result.tiles[x][y].entity = entities[EMPTY]; 
            result.tiles[x][y].entity.pos[0] = x;
            result.tiles[x][y].entity.pos[1] = y;
            result.biome = 0;
        }
    }
    return result;
}

void reset_game(GameState* g)
{
    g->player = entities[PLAYER];
    g->player.pos[0] = 5;
    g->player.pos[1] = 5;

    g->maps[0] = empty_map();
}

int main (int argc, char *argv[])
{
    (void)argc; (void)argv;

    InitWindow(G_WIDTH, G_HEIGHT, "the traders");
    SetTargetFPS(60);

    RenderTexture2D screen = LoadRenderTexture(WIDTH, HEIGHT);
    SetTextureFilter(screen.texture, TEXTURE_FILTER_POINT);

    create_entities(entities);

    reset_game(&game);

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_RIGHT)) {
            game.player.pos[0] += 1;
        } else if (IsKeyPressed(KEY_LEFT)) {
            game.player.pos[0] -= 1;
        } else if (IsKeyPressed(KEY_UP)) {
            game.player.pos[1] -= 1;
        } else if (IsKeyPressed(KEY_DOWN)) {
            game.player.pos[1] += 1;
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