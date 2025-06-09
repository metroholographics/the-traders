#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include "main.h"

GameState game;
Entity entities[NUM_ENTITY_TYPES];

void create_entities(Entity* e)
{
    e[EMPTY] = (Entity) {0};
    e[EMPTY] = (Entity) {
        .pos = {-1,-1},
        .type = EMPTY,
        .walkable = true,
        .interactable = false,
        .action = NO_ACTION,
    };
    e[PLAYER] = (Entity){0};
    e[PLAYER] = (Entity) {
        .type = PLAYER,
        .walkable = false,
        .interactable = false,
        .action = NO_ACTION,
    };
    e[TREE] = (Entity){0};
    e[TREE] = (Entity) {
        .type = TREE,
        .walkable = false,
        .interactable = true,
        .action = CUT,
        .health = 100
    };
}

Tile create_tile(Entity current, Entity previous, int x, int y)
{
    Entity c = current;
    Entity p = previous;
    c.pos[0] = p.pos[0] =  x;
    c.pos[1] = p.pos[1] = y;
    return (Tile) {.entity = c, .previous = p};
}

Map empty_map(void)
{
    Map result = {0};

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            result.tiles[x][y] = create_tile(entities[EMPTY], entities[EMPTY], x, y);
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
    g->maps[0].tiles[10][8] = create_tile(test_tree, test_tree, 10, 8);
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
    //Mouse-Handling
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Tile* selected = get_selected_tile(&game.maps[0]);
        if (game.selected_tile == selected) {
            register_action(&game.player, game.selected_tile);
            printf("already selected\n");
        } else {
            game.selected_tile = selected;
        }
        printf("%d, %d\n", selected->entity.pos[0], selected->entity.pos[1]);  
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (game.selected_tile != NULL) {
            game.selected_tile = NULL;
        }
    }
        //Keyboard-handling
    int new_x = p->pos[0];
    int new_y = p->pos[1];
    
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

    if (tile_in_bounds(new_x, new_y)) {
        Entity target = game.maps[0].tiles[new_x][new_y].entity;
        if (target.walkable) {
            p->pos[0] = new_x;
            p->pos[1] = new_y;
            //p->action = NO_ACTION;
        } else {
            printf("not walkable\n");
        }
    }
}

Tile* get_selected_tile(Map* m)
{
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    Vector2 mouse_world = screen_to_world(mouse_x, mouse_y);
    int tile_x = mouse_world.x / TILE_WIDTH;
    int tile_y = mouse_world.y / TILE_HEIGHT;

    return &m->tiles[tile_x][tile_y];
}

bool tile_in_bounds(int x, int y)
{
    return (x >= 0 && x < COLS && y >= 0 && y < ROWS);
}

void register_action(Entity* p, Tile* target)
{
    if (!target->entity.interactable) return;

    switch(target->entity.action) {
        case NO_ACTION:
            break;
        case CUT:
            p->action = CUT;
            p->target = &target->entity;
            printf("For Cut: %p\n", &target->entity);
            break;
        default:
            break;
    }
}

void handle_action(Entity* p)
{
    Action action = p->action;
    if (action == NO_ACTION) {
        printf("no action\n");
        return;
    }
    switch (action) {
        case CUT:
            printf("To Cut: %p ", p->target); 
            cut_target(p, p->target);
            //printf("To Cut: %p\n", p->target);
            break;
        default:
            break;
    }
}

void cut_target(Entity* p, Entity* t)
{
    printf("Cutting %p: \n", t);
    float current_time = p->timer.time;
    if (current_time > 0.5f) p->timer.time = current_time = 0.0f;
    if (current_time == 0.0f) {
        t->health -= 30;
    }
    p->timer.time += GetFrameTime();

    if (t->health <= 0) {
        t->health = 100;
        p->action = NO_ACTION;
    }
}


void update_game(GameState* g)
{
    handle_action(&g->player);
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
                            printf("%d\n", m_entity.entity.health);
                            float height_factor = m_entity.entity.health / 100.0f; 
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, height_factor * TILE_HEIGHT, BROWN);
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