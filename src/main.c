#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include "main.h"

GameState game;
Entity entities[NUM_ENTITY_TYPES];

void create_entities(Entity* e)
{
    e[EMPTY] = (Entity) {
        .pos = {-1,-1},
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

    Entity test_tree = entities[TREE];
    test_tree.pos[0] = 10;
    test_tree.pos[1] = 8;

    g->maps[0].tiles[10][8].entity = test_tree;
    g->selected_tile = NULL;

}

Vector2 screen_to_world(int x, int y)
{
    int world_x = x / SCALE;
    int world_y = y / SCALE;
    return (Vector2) {world_x,world_y};
}

void handle_input(Entity* p)
{
    int new_x = p->pos[0];
    int new_y = p->pos[1];

    //Keyboard-handling
    KeyboardKey key_pressed = GetKeyPressed();
    switch (key_pressed) {
        case KEY_RIGHT:
        case KEY_D:
            new_x += 1;
            break;
        case KEY_LEFT:
        case KEY_A:
            new_x -= 1;
            break;
        case KEY_UP:
        case KEY_W:
            new_y -= 1;
            break;
        case KEY_DOWN:
        case KEY_S:
            new_y += 1;
            break;
        default:
            break;
    }

    //Mouse-Handling
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        Vector2 mouse_world = screen_to_world(mouse_x, mouse_y);
        int tile_x = mouse_world.x / TILE_WIDTH;
        int tile_y = mouse_world.y / TILE_HEIGHT;
        Tile* selected = &game.maps[0].tiles[tile_x][tile_y];
        game.selected_tile = selected;
        
        printf("%d, %d\n", tile_x, tile_y);  
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (game.selected_tile != NULL) {
            game.selected_tile = NULL;
        }
    }

    if (new_x < 0 || new_x >= COLS || new_y < 0 || new_y >= ROWS) goto function_end;

    Entity target = game.maps[0].tiles[new_x][new_y].entity;
    if (target.walkable) {
        p->pos[0] = new_x;
        p->pos[1] = new_y;
    }

    function_end:
}

void update_game(GameState* g)
{
    handle_input(&g->player);

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

        update_game(&game);


        

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
                        case TREE:
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BROWN);
                            break;
                        default:
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
                    }
                }
            }
            Entity p = game.player;
            DrawRectangle(p.pos[0] * TILE_WIDTH, p.pos[1] * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, DARKGREEN);

            if (game.selected_tile != NULL) {
                Entity selected = game.selected_tile->entity;
                int select_x = selected.pos[0];
                int select_y = selected.pos[1];
                DrawCircle(select_x * TILE_WIDTH + HALF_TILE_WIDTH, select_y * TILE_HEIGHT + HALF_TILE_HEIGHT, 2, WHITE);
            }
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