#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include "main.h"

GameState game;
Entity entities[NUM_ENTITY_TYPES];

Tile* get_selected_tile(Map* m)
{
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    Vector2 mouse_world = screen_to_world(mouse_x, mouse_y);
    int tile_x = mouse_world.x / TILE_WIDTH;
    int tile_y = mouse_world.y / TILE_HEIGHT;

    return &m->tiles[tile_x][tile_y];
    return &m->tiles[tile_x][tile_y];
}

bool tile_in_bounds(int x, int y)
{
    return (x >= 0 && x < COLS && y >= 0 && y < ROWS);
}

Vector2 screen_to_world(int x, int y)
{
    int world_x = x / SCALE;
    int world_y = y / SCALE;
    return (Vector2) {world_x,world_y};
}

Map empty_map(void)
{
    Map result = {0};
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            result.tiles[x][y] = create_tile(EMPTY, EMPTY, x, y);
            result.biome = 0;
            result.entity_queue.queue[0] = NULL;
            result.entity_queue.count = 0;
        }
    }
    return result;
}

void create_entities(Entity* e)
{
    e[EMPTY] = (Entity) {0};
    e[EMPTY] = (Entity) {
        .pos = {-1,-1},
        .type = EMPTY,
        .walkable = true,
        .action = NO_ACTION,
    };
    e[PLAYER] = (Entity){0};
    e[PLAYER] = (Entity) {
        .type = PLAYER,
        .walkable = false,
        .action = NO_ACTION,
    };
    e[TREE] = (Entity){0};
    e[TREE] = (Entity) {
        .type = TREE,
        .walkable = false,
        .action = CUT,
        .health = 100
    };
    e[STUMP] = (Entity){0};
    e[STUMP] = (Entity){
        .type = STUMP,
        .walkable = false,
        .action = GROW,
        .health = 0
    }; 
}

Tile create_tile(Entity_Type current, Entity_Type previous, int x, int y)
{
    Entity c = entities[current];
    Entity p = entities[previous];
    c.pos[0] = p.pos[0] =  x;
    c.pos[1] = p.pos[1] = y;
    return (Tile) {.entity = c, .previous = p};
}

void reset_game(GameState* g)
{
    g->player = entities[PLAYER];
    g->player.pos[0] = 5;
    g->player.pos[1] = 5;

    g->maps[0] = empty_map();

    g->maps[0].tiles[10][8] = create_tile(TREE, STUMP, 10, 8);
    g->maps[0].tiles[11][8] = create_tile(TREE, STUMP, 11, 8);
    g->selected_tile = NULL;
    g->debug_mode = false;

}

void handle_input(Entity* p)
{
    //Mouse-Handling
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Tile* selected = get_selected_tile(&game.maps[0]);
        if (game.selected_tile == selected) {
            register_action(p, game.selected_tile);
        } else {
            game.selected_tile = selected;
        }
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
            p->action = NO_ACTION;
            new_x += 1;
            break;
        case KEY_LEFT:
        case KEY_A:
            p->action = NO_ACTION;
            new_x -= 1;
            break;
        case KEY_UP:
        case KEY_W:
            p->action = NO_ACTION;
            new_y -= 1;
            break;
        case KEY_DOWN:
        case KEY_S:
            p->action = NO_ACTION;
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
        }
    }
}

void register_action(Entity* p, Tile* target)
{
    switch(target->entity.action) {
        case NO_ACTION:
            p->action = NO_ACTION;
            break;
        case CUT:
            p->action = CUT;
            p->target = &target->entity;
            break;
        default:
            break;
    }
}

void handle_action(Entity* p)
{
    Action action = p->action;
    if (action == NO_ACTION) {
        return;
    }
    switch (action) {
        case CUT:
            cut_target(p, p->target);
            break;
        default:
            break;
    }
}

void cut_target(Entity* p, Entity* t)
{
    float current_time = p->timer.time;
    if (current_time > 0.5f) p->timer.time = current_time = 0.0f;
    if (current_time == 0.0f) {
        t->health -= 30;
    }
    p->timer.time += GetFrameTime();

    if (t->health <= 0) {
        int tile_x = t->pos[0];
        int tile_y = t->pos[1];
        Tile* t = &game.maps[0].tiles[tile_x][tile_y];
        *t = create_tile(
            t->previous.type,
            t->entity.type,
            tile_x, tile_y
        );
        add_to_map_queue(&game.maps[0], tile_x, tile_y);
        p->action = NO_ACTION; 
        p->target = NULL; 
    }
}

void add_to_map_queue(Map* m, int x, int y)
{
    Entity* to_add = &m->tiles[x][y].entity;
    for (int i = 0; i < MAP_ENTITY_NUM; i++) {
        if (m->entity_queue.queue[i] == NULL) {
            m->entity_queue.queue[i] = to_add;
            m->entity_queue.count++;
            break;
        }
    }
}

void handle_map_queue(Map* m, Entity_Queue* eq)
{
    for (int i = 0; i < MAP_ENTITY_NUM; i++) {
        Entity* e = eq->queue[i];
        if (e == NULL) continue;
        switch (e->action) {
            case GROW:
                grow_stump(m, e, i);
                break;
            default:
                break;
        }
    }
}

void grow_stump(Map* m, Entity* e, int index)
{
    e->timer.time += GetFrameTime();
    if (e->timer.time < 1.0f) return;

    e->timer.time = 0.0f; // Reset timer
    if (e->health < 100) {
        e->health += 30;
    } else {
        int x = e->pos[0];
        int y = e->pos[1];
        Tile* t = &m->tiles[x][y];
        m->tiles[x][y] = create_tile(t->previous.type, t->entity.type, x, y);
        m->entity_queue.queue[index] = NULL; // Remove from queue
        m->entity_queue.count--;
    }
}


void update_game(GameState* g)
{
    if (g->debug_mode) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Tile* selected = get_selected_tile(&g->maps[0]);
            Entity selected_entity = selected->entity;
            printf("Type: %d ", selected_entity.type);
            printf("Pos: %d, %d\n",selected_entity.pos[0], selected_entity.pos[1]);
            printf("Health: %d\n", selected_entity.health);
            printf("Action: %d\n", selected_entity.action);
        }
    }
    handle_action(&g->player);
    handle_input(&g->player);
    handle_map_queue(&g->maps[0], &g->maps[0].entity_queue);

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
        if (IsKeyPressed(KEY_LEFT_CONTROL)) {
            game.debug_mode = !game.debug_mode;
        }
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
                        case TREE: {
                            float height_factor = m_entity.entity.health / 100.0f;
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, height_factor * TILE_HEIGHT, BROWN);
                        }
                            break;
                        case STUMP: {
                            float height_factor = m_entity.entity.health / 100.0f; 
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, height_factor * TILE_HEIGHT, DARKGRAY);
                        }
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

            if (game.debug_mode) {
                DrawText("Debug", 10, 10, 8, RED);
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