#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
}

bool tile_in_bounds(int x, int y)
{
    return (x >= 0 && x < COLS && y >= 0 && y < ROWS);
}

Vector2 screen_to_world(int x, int y)
{
    int world_x = x / SCALE;
    int world_y = y / SCALE;
    return (Vector2) {world_x, world_y};
}

Map empty_map(void)
{
    Map result = {0};
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            result.tiles[x][y] = create_tile(EMPTY, EMPTY, x, y);
        }
    }
    result.biome = 0;
    result.entity_queue.queue[0] = NULL;
    result.entity_queue.count = 0;
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
        .max_health = 100,
        .health = 100,
        .action_rate = 0.65f,
        .drop = LOG
    };
    e[STUMP] = (Entity){0};
    e[STUMP] = (Entity) {
        .type = STUMP,
        .walkable = false,
        .action = GROW,
        .max_health = 100,
        .health = 0,
        .action_rate = 1.0f,
        .growth_per_tick = 30,
    };
    e[RUIN] = (Entity){0};
    e[RUIN] = (Entity) {
        .type = RUIN,
        .walkable = false,
        .action = HARVEST,
        .health = 50,
        .action_rate = 0.5,
        .drop = IVY,
    };
    e[CLEAN_RUIN] = (Entity) {0};
    e[CLEAN_RUIN] = (Entity) {
        .type = CLEAN_RUIN,
        .walkable = false,
        .action = GROW,
        .max_health = 50,
        .health = 0,
        .action_rate = 1.0f,
        .growth_per_tick = 15
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

    g->stats = (Player_Stats) {
        .woodcut_dmg = 30
    };

    g->jobs = (Job_Manager) {0};

    g->maps[0] = empty_map();
    g->current_map = &g->maps[0];
    g->current_map->tiles[5][3] = create_tile(TREE, STUMP, 5, 3);
    g->current_map->tiles[7][4] = create_tile(RUIN, CLEAN_RUIN, 7, 4);
    g->current_map->tiles[2][10] = create_tile(RUIN, CLEAN_RUIN, 2, 10);
    g->current_map->tiles[10][8] = create_tile(TREE, STUMP, 10, 8);
    g->current_map->tiles[11][8] = create_tile(TREE, STUMP, 11, 8);
    g->current_map->tiles[10][5] = create_tile(TREE, STUMP, 10, 5);
    g->current_map->tiles[11][7] = create_tile(TREE, STUMP, 11, 7);
    g->selected_tile = NULL;
    g->debug_mode = false;
    g->hover_text = (Hover_Text) {0};

    g->inventory = (Inventory) {0};
    g->inventory.space = (Rectangle) {
        .x = 0,
        .y = (INV_Y_POS_FACTOR * ROWS) * GAME_TILE_HEIGHT,
        .width = INV_WIDTH_FACTOR * G_WIDTH,
        .height = (1.0f - INV_Y_POS_FACTOR) * G_HEIGHT,
    };
    g->inventory.slot_size = (Rectangle) {
        .x = g->inventory.space.x + INV_INITIAL_OFFSET,
        .y = g->inventory.space.y + INV_INITIAL_OFFSET,
        .width = (g->inventory.space.width / 4) , //GAME_TILE_WIDTH - INV_INITIAL_OFFSET,
        .height = (g->inventory.space.height / 4)//GAME_TILE_HEIGHT - INV_INITIAL_OFFSET,
    };

}

void set_hover_text(Hover_Text* t, Tile target, char* msg)
{
    char* text = msg;
    if (text == NULL) {
        switch (target.entity.action) {
            case CUT: {
                text = "cut?";
            }
                break;
            case HARVEST: {
                text = "harvest?";
            }
                break;
            case EMPTY: {
                text = "the land teems with overgrowth\n";
            } 
            break;
            default:
                break;
        }
    }
   
    if (text != NULL) {
        t->active = true;
        snprintf(t->string, MAX_HOVER_TEXT_LEN, text);
        t->time.time = 0.0f;
    } else {
        t->active = false;
    }
}

void handle_input(Entity* p)
{
    //Mouse-Handling
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Tile* selected = get_selected_tile(game.current_map);
        if (game.selected_tile == selected) {
            register_action(p, game.selected_tile);
            
        } else {
            game.selected_tile = selected;
            set_hover_text(&game.hover_text, *game.selected_tile, NULL);
        }
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (game.selected_tile != NULL) {
            game.selected_tile = NULL;
        }
        game.hover_text.active = false;
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
        Entity target = game.current_map->tiles[new_x][new_y].entity;
        if (target.walkable) {
            p->pos[0] = new_x;
            p->pos[1] = new_y;
        }
    }
}

void register_action(Entity* p, Tile* target)
{
    game.hover_text.active = false;
    switch(target->entity.action) {
        case NO_ACTION:
            p->action = NO_ACTION;
            break;
        case CUT: {
            if (entities_adjacent(*p, target->entity)) {
                p->action = CUT;
                p->target = &target->entity;
            } else {
                game.hover_text.active = true;
                set_hover_text(&game.hover_text, *target, "I'm too far...");
            }
        }
            break;
        case HARVEST: {
            if (entities_adjacent(*p, target->entity)) {
                p->action = HARVEST;
                p->target = &target->entity;
            } else {
                game.hover_text.active = true;
                set_hover_text(&game.hover_text, *target, "I'm too far...");
            }
        }
            break;
        default:
            break;
    }
}

void handle_action(Entity* p)
{
    Action action = p->action;
    if (action == NO_ACTION) return;

    switch (action) {
        case CUT: {
                cut_target(p, p->target);
            } 
            break;
        case HARVEST: {
            harvest_target(p, p->target);
        }
            break;
        default:
            break;
    }
}

bool entities_adjacent(Entity e, Entity target)
{
    int e_x = e.pos[0];
    int e_y = e.pos[1];
    int target_x = target.pos[0];
    int target_y = target.pos[1];
    bool x_adjacent, y_adjacent;
    x_adjacent = y_adjacent = false;

    if (e_x + 1 == target_x || e_x - 1 == target_x || e_x == target_x) {
        x_adjacent = true;
    }

    if (e_y + 1 == target_y || e_y - 1 == target_y || e_y == target_y) {
        y_adjacent = true;
    }

    return (x_adjacent && y_adjacent);
}

void cut_target(Entity* p, Entity* t)
{
    float current_time = p->timer.time;
    if (current_time > t->action_rate) p->timer.time = current_time = 0.0f;
    if (current_time == 0.0f) {
        t->health -= game.stats.woodcut_dmg;
    }
    p->timer.time += GetFrameTime();

    if (t->health <= 15) {
        int tile_x = t->pos[0];
        int tile_y = t->pos[1];
        Tile* t = &game.current_map->tiles[tile_x][tile_y];
        *t = create_tile(
            t->previous.type,
            t->entity.type,
            tile_x, tile_y
        );
        add_to_map_queue(game.current_map, tile_x, tile_y);
        p->action = NO_ACTION; 
        p->target = NULL;
        add_to_inventory(LOG, &game); 
    }
}

void harvest_target(Entity* p, Entity* t)
{
    float current_time = p->timer.time;
    if (current_time > t->action_rate) p->timer.time = current_time = 0.0f;
    if (current_time == 0.0f) {
        t->health -= 20;
    }
    p->timer.time += GetFrameTime();
    if (t->health <= 0) {
        int tile_x = t->pos[0];
        int tile_y = t->pos[1];
        Tile* t = &game.current_map->tiles[tile_x][tile_y];
        *t = create_tile(
            t->previous.type,
            t->entity.type,
            tile_x, tile_y
        );
        add_to_map_queue(game.current_map, tile_x, tile_y);
        p->action = NO_ACTION; 
        p->target = NULL;
        add_to_inventory(IVY, &game); 
    }

    return;
}

void add_to_inventory(Drop drop, GameState* g)
{
    g->inventory_count[drop] += 1;
    Drop* slots = g->inventory.slots;
    int i = 0;
    for (i = 0; i < INVENTORY_SLOTS; i++) {
        if (slots[i] == NONE) {
            slots[i] = drop;
            break;
        }
    }

    if (i == INVENTORY_SLOTS) {
        printf("inventory full - handle this\n");
        g->inventory_count[drop] -= 1;
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
    //::todo: check if queue is full, increase capacity and re-call function to add entity if so
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
    if (e->timer.time < e->action_rate) return;

    e->timer.time = 0.0f; // Reset timer
    if (e->health < e->max_health) {
        e->health += e->growth_per_tick;
    } else {
        int x = e->pos[0];
        int y = e->pos[1];
        Tile* t = &m->tiles[x][y];
        *t = create_tile(t->previous.type, t->entity.type, x, y);
        m->entity_queue.queue[index] = NULL; // Remove from queue
        m->entity_queue.count--;
    }
}

void update_hover_text(Hover_Text* t)
{
    if (!t->active) return;

    Vector2 text_w = MeasureTextEx(game.game_font, t->string, FONT_SIZE, FONT_SPACING);

    //note: hove text position is in worldspace
    int player_x = game.player.pos[0];
    int player_y = game.player.pos[1];

    int centre_x = (player_x * GAME_TILE_WIDTH) + (0.5f * GAME_TILE_WIDTH); 

    int text_x = centre_x - (0.5f * text_w.x);
    int text_y = player_y * GAME_TILE_HEIGHT;
    float y_offset = 0.125f;
    if (player_y == 0) {
        text_y += GAME_TILE_HEIGHT;
        y_offset *= -1;
    }

    t->pos[0] = text_x;
    t->pos[1] = text_y - (y_offset * GAME_TILE_HEIGHT);

}

void update_game(GameState* g)
{
    if (g->debug_mode) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Tile* selected = get_selected_tile(g->current_map);
            Entity selected_entity = selected->entity;
            printf("Type: %d ", selected_entity.type);
            printf("Pos: %d, %d\n",selected_entity.pos[0], selected_entity.pos[1]);
            printf("Health: %d\n", selected_entity.health);
            printf("Action: %d\n", selected_entity.action);
        }
    }

    tick_job_queue(&g->jobs);

    if (g->jobs.create_job) {
        create_job(&g->jobs);
    }

    handle_action(&g->player);
    handle_input(&g->player);
    update_hover_text(&g->hover_text);
    handle_map_queue(g->current_map, &g->current_map->entity_queue);
}

void create_job(Job_Manager* j)
{
    j->current_job.status = OFFERED;
    //::todo: continue here - first add more drop types

    j->create_job = false;
}

void tick_job_queue(Job_Manager* j) 
{
    if (j->current_job.status != INACTIVE) return;

    j->timer.time += GetFrameTime();

    if (j->timer.time >= NEW_JOB_TIME) {
        j->create_job = true;
        j->timer.time = 0.0f;
    } 
    return;
}

void load_sprite_sources(GameState* g)
{
    Rectangle* src_array = g->sprites.source;
    src_array[EMPTY]  = (Rectangle) {0,0,0,0};
    src_array[PLAYER] = (Rectangle) {.x =  0.0f,  .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[TREE]   = (Rectangle) {.x = 32.0f,  .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[STUMP]  = (Rectangle) {.x = 64.0f,  .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[GRASS]  = (Rectangle) {.x = 96.0f,  .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[RUIN]   = (Rectangle) {.x = 128.0f, .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[CLEAN_RUIN] = (Rectangle) {.x = 128.0f, .y = 32.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};

}

void load_drop_images(GameState* g)
{
    Rectangle* drop_array = g->sprites.drop_source;
    drop_array[EMPTY] = (Rectangle) {0,0,0,0};
    drop_array[LOG]   = (Rectangle) {.x = 32.0f, .y = 32.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    drop_array[IVY]   = (Rectangle) {.x = 128.0f, .y = 64.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
}

void draw_display_ui(GameState* g)
{
    DrawRectangleRec(g->inventory.space, (Color) {10, 10, 10, 125});

    Texture2D sheet = g->sprites.spritesheet;
    Rectangle* drop_images = g->sprites.drop_source; 
    Inventory inventory = g->inventory;
    Rectangle inv_shape = inventory.slot_size;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        Drop d = inventory.slots[i];
        DrawTexturePro(sheet, drop_images[d], inv_shape, (Vector2){0,0}, 0.0f, WHITE);
        inv_shape.x += inv_shape.width;
        if ((i + 1) % 4 == 0) {
            inv_shape.y += inv_shape.height;
            inv_shape.x = inventory.slot_size.x;
        }
    }
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
    game.sprites.spritesheet = LoadTexture("assets/spritesheet.png");
    game.game_font = LoadFont("assets/fonts/Kobata-Bold.otf");
    SetTextureFilter(game.sprites.spritesheet, TEXTURE_FILTER_POINT);
    load_sprite_sources(&game);
    load_drop_images(&game);

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
                    Rectangle dest = (Rectangle) {.x = (float)j * TILE_WIDTH, .y = (float)i * TILE_HEIGHT, .width = TILE_WIDTH, .height = TILE_HEIGHT};
                    Vector2 dest_vec = (Vector2) {0,0};
                    //DrawRectangleLines(dest.x, dest.y, dest.width, dest.height, RED);
                    switch (m_entity.entity.type) {
                        case EMPTY:
                            DrawTexturePro(game.sprites.spritesheet, game.sprites.source[GRASS], dest, dest_vec, 0.0f, WHITE);
                            break;
                        case TREE: {
                            float height_factor = m_entity.entity.health / 100.0f;
                            float chop = 1.0f - height_factor;
                            Rectangle source = game.sprites.source[TREE];
                            source.y = source.y + (chop * SPRITE_SIZE);
                            source.height =  height_factor * SPRITE_SIZE;
                            dest.y = ((float) i * TILE_HEIGHT) + (chop * TILE_WIDTH);
                            dest.height = height_factor * TILE_HEIGHT;
                            DrawTexturePro(game.sprites.spritesheet, source, dest, dest_vec, 0.0f, WHITE);
                        }
                            break;
                        case RUIN: {
                            DrawTexturePro(game.sprites.spritesheet, game.sprites.source[RUIN], dest, dest_vec, 0.0f, P_WHITE);
                        }
                            break;
                        case CLEAN_RUIN: {
                            DrawTexturePro(game.sprites.spritesheet, game.sprites.source[CLEAN_RUIN], dest, dest_vec, 0.0f, P_WHITE);
                        }
                            break;
                        case STUMP: {
                            DrawTexturePro(game.sprites.spritesheet, game.sprites.source[STUMP], dest, dest_vec, 0.0f, P_WHITE);
                        }
                            break;
                        default:
                            DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
                    }
                }
            }
            
            Entity p = game.player;
            Rectangle dest_rect = (Rectangle) {.x = p.pos[0] * TILE_WIDTH, .y = p.pos[1] * TILE_HEIGHT, .width = TILE_WIDTH, .height = TILE_HEIGHT};
            DrawTexturePro(game.sprites.spritesheet, game.sprites.source[PLAYER], dest_rect, (Vector2) {0,0}, 0.0f, WHITE);

            if (game.selected_tile != NULL) {
                Entity selected = game.selected_tile->entity;
                int select_x = selected.pos[0];
                int select_y = selected.pos[1];
                DrawCircle(select_x * TILE_WIDTH + HALF_TILE_WIDTH, select_y * TILE_HEIGHT + HALF_TILE_HEIGHT, FONT_SPACING, WHITE);
            }

            if (game.debug_mode) {
                DrawTextEx(game.game_font, "Debug",(Vector2) {10, 10}, 22, FONT_SPACING, P_RED);
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
            if (game.hover_text.active) {
                Hover_Text t = game.hover_text;
                DrawTextEx(game.game_font, t.string,(Vector2){t.pos[0], t.pos[1]}, FONT_SIZE, FONT_SPACING, P_WHITE);
            }
            draw_display_ui(&game);
        EndDrawing();
    }

    UnloadRenderTexture(screen);
    CloseWindow();
    
}