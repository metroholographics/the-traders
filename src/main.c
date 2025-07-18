#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
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
    float world_x = x / (G_WIDTH / WIDTH);
    float world_y = y / (G_HEIGHT / HEIGHT);
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
    result.walkable = true;
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
        .growth_per_tick = 20
    };
    e[SHOP] = (Entity) {0};
    e[SHOP] = (Entity) {
        .type = SHOP,
        .walkable = false,
        .action = SELL,
    };
    e[ROCK] = (Entity){0};
    e[ROCK] = (Entity) {
        .type = ROCK,
        .walkable = false,
        .action = MINE,
        .health = 100,
        .action_rate = 1.0f,
        .drop = ORE,
    };
    e[EMPTY_ROCK] = (Entity){0};
    e[EMPTY_ROCK] = (Entity) {
        .type = EMPTY_ROCK,
        .walkable = false,
        .action = GROW,
        .max_health = 100,
        .health = 0,
        .action_rate = 1.0f,
        .growth_per_tick = 30
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

Map generate_biome_map(Biome_Type b)
{
    Map map = {0};
    Biome j = game.biomes[b];
    int count = j.count;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (x == 0 || y == 0 || x == COLS - 1 || y == ROWS - 1) {
                map.tiles[x][y] = create_tile(j.default_tile, j.default_tile, x, y);
                continue;
            }
            int rand = GetRandomValue(1, 100);
            for (int i = 0; i < count; i++) {
                if (rand <= j.chance[i]) {
                    map.tiles[x][y] = j.tile_types[i];
                    map.tiles[x][y].entity.pos[0] = x;
                    map.tiles[x][y].entity.pos[1] = y;
                    break;
                }
            }
        }
    }
    map.walkable = true;
    map.biome = JUNGLE;
    map.entity_queue.queue[0] = NULL;
    map.entity_queue.count = 0;
    return map;
}

void generate_world(Map world[][WORLD_COLS])
{
    for (int y = 0; y < WORLD_ROWS; y++) {
        for (int x = 0; x < WORLD_COLS; x++) {
            world[y][x] = generate_biome_map(JUNGLE); 
            //world[y][x] = empty_map();
        }
    }

    /*
        # # # # * . . . .
        # # # # * . . . .
        * # # # # * * . .
        . * # # # # # # *
        . . * # # # # # #
        . . . * # # # # *
    */
    world[0][5].walkable = false;
    world[0][6].walkable = false;
    world[0][7].walkable = false;
    world[0][8].walkable = false;
    world[1][5].walkable = false;
    world[1][6].walkable = false;
    world[1][7].walkable = false;
    world[1][8].walkable = false;
    world[2][7].walkable = false;
    world[2][8].walkable = false;
    world[3][0].walkable = false;
    world[4][0].walkable = false;
    world[4][1].walkable = false;
    world[5][0].walkable = false;
    world[5][1].walkable = false;
    world[5][2].walkable = false;
}

Biome create_jungle_biome()
{
    Biome b = {0};
    CREATE_BIOME_TILE(b, 0, RUIN, CLEAN_RUIN, 5);
    CREATE_BIOME_TILE(b, 1, ROCK, EMPTY_ROCK, 10);
    CREATE_BIOME_TILE(b, 2, TREE, STUMP, 25);
    CREATE_BIOME_TILE(b, 3, EMPTY, EMPTY, 100);

    b.default_tile = EMPTY;
    return b;
}

void set_ui_elements(GameState* g)
{
    g->ui.showing = true;
    g->ui.inventory = (Inventory) {0};
    g->ui.inventory.space = (Rectangle) {
        .x = 0,
        .y = (INV_Y_POS_FACTOR * ROWS) * GAME_TILE_HEIGHT,
        .width = INV_WIDTH_FACTOR * G_WIDTH,
        .height = (1.0f - INV_Y_POS_FACTOR) * G_HEIGHT,
    };
    g->ui.inventory.slot_size = (Rectangle) {
        .x = g->ui.inventory.space.x + INV_INITIAL_OFFSET,
        .y = g->ui.inventory.space.y + INV_INITIAL_OFFSET,
        .width = (g->ui.inventory.space.width / 4) + INV_INITIAL_OFFSET, 
        .height = (g->ui.inventory.space.height / 4) + INV_INITIAL_OFFSET
    };
    g->ui.shapes[INVENTORY] = g->ui.inventory.space;

    g->ui.offer = (Job_Offer) {0};
    g->ui.offer.space = (Rectangle) {
        .x = 0,
        .y = (OFFER_Y_FACTOR * ROWS) * GAME_TILE_HEIGHT,
        .width = INV_WIDTH_FACTOR * G_WIDTH,
        .height = 0.25f * G_HEIGHT,
    };
    g->ui.offer.slot_size = (Rectangle) {
        .x = g->ui.offer.space.x + INV_INITIAL_OFFSET,
        .y = g->ui.offer.space.y + (0.5f * g->ui.offer.space.height),
    };
    g->ui.offer.slot_size.width = (g->ui.offer.space.width / 3) + INV_INITIAL_OFFSET;
    g->ui.offer.slot_size.height = (g->ui.offer.space.height - g->ui.offer.slot_size.y);
    Vector2 t_pos = get_centered_text_rec(g->ui_font, "$0000.00", g->ui.offer.space, 48, 2);
    t_pos.y *= 0.8f;
    g->ui.offer.reward_pos = t_pos;
    g->ui.shapes[JOB_OFFER] = g->ui.offer.space;

    Button *b = &g->ui.offer.accept_button;
    *b = (Button) {0};
    b->clickable = true;
    b->shape = (Rectangle) {
        .x = g->ui.offer.space.x,
        .y = g->ui.offer.space.y + g->ui.offer.space.height,
        .width = g->ui.offer.space.width,
        .height = (0.5f * GAME_TILE_HEIGHT)
    };
    b->accept_text_pos = get_centered_text_rec(g->ui_font, "ACCEPT?", b->shape, 32, 2);
    g->ui.shapes[JOB_ACCEPT_BUTTON] = g->ui.offer.accept_button.shape;

    g->ui.active_job = (Accepted_Job){0};
    g->ui.active_job.space = (Rectangle) {
        .x = 0,
        .y = (0.45f * ROWS) * GAME_TILE_HEIGHT,
        .width = INV_WIDTH_FACTOR * G_WIDTH,
        .height = 0.125f * G_HEIGHT 
    };
    g->ui.active_job.slot_size = (Rectangle) {
        .x = g->ui.active_job.space.x + INV_INITIAL_OFFSET,
        .y = g->ui.active_job.space.y + (0.2f * g->ui.active_job.space.height),
    };
    g->ui.active_job.slot_size.width  = (g->ui.active_job.space.width / 3);
    g->ui.active_job.slot_size.height = 0.75f * GAME_TILE_HEIGHT;//(g->ui.active_job.space.height - (0.33f * g->ui.active_job.space.height));
    g->ui.active_job.reward_pos = (Vector2) {
        .x = g->ui.active_job.space.x + INV_INITIAL_OFFSET,
        .y = g->ui.active_job.space.y + INV_INITIAL_OFFSET
    };
    g->ui.shapes[JOB_ACTIVE] = g->ui.active_job.space;
}

void reset_game(GameState* g)
{
    SetRandomSeed(time(NULL));

    g->biomes[JUNGLE] = create_jungle_biome();

    g->player = entities[PLAYER];
    g->player.pos[0] = 5;
    g->player.pos[1] = 5;
    g->stats = (Player_Stats) {
        .woodcut_dmg = 30,
        .mine_dmg = 30
    };

    g->physics_queue = (Physics_Queue) {0};

    g->jobs = (Job_Manager) {0};
    g->jobs.sell_timer.max_time = SELL_TIMER_MAX;
    g->jobs.sell_timer.min_time = SELL_TIMER_MIN;



    generate_world(g->world);
    g->world_pos = (Int_Vec2) {0,0};
    g->current_map = &g->world[g->world_pos.y][g->world_pos.x];

    g->world[0][0].tiles[13][1] = create_tile(SHOP, EMPTY, 13, 1);
    g->world[0][3].tiles[7][4]  = create_tile(SHOP, EMPTY,  7, 4);
    g->world[2][0].tiles[12][2] = create_tile(SHOP, EMPTY, 12, 2);
    g->world[3][3].tiles[3][8]  = create_tile(SHOP, EMPTY,  3, 8);
    g->world[5][5].tiles[6][3]  = create_tile(SHOP, EMPTY,  6, 3);
    g->world[4][8].tiles[13][7] = create_tile(SHOP, EMPTY, 13, 7);


    g->current_map->tiles[13][1] = create_tile(SHOP, EMPTY, 13, 1);

    g->selected_tile = NULL;
    g->debug_mode = false;
    g->hover_text = (Hover_Text) {0};

    set_ui_elements(g);
}

void set_entity_action_text(char* b, Entity e)
{
    char* text = "\0";
    switch (e.action) {
        case CUT: {
            text = "cut?";
            break;
        }
        case HARVEST: {
            text = "harvest?";
            break;
        }
        case SELL: {
            text = "sell?";
            break;
        }
        case MINE: {
            text = "mine?";
            break;
        }
        case NO_ACTION: {
            text = "the land teems with overgrowth";
            break;
        } 
        default:
            break;
    }
    snprintf(b, MAX_HOVER_TEXT_LEN, text);
}

Vector2 get_centered_text_rec(Font font, const char* t, Rectangle rec, int font_size, int font_spacing)
{
    Vector2 text_size = MeasureTextEx(font, t, font_size, font_spacing);
    float rec_center_x = rec.x  + (0.5f * rec.width);
    float rec_center_y = rec.y  + (0.5f * rec.height);
    float text_x = rec_center_x - (0.5f * text_size.x);
    float text_y = rec_center_y - (0.5f * text_size.y);

    return (Vector2){text_x, text_y};
}

void set_hover_text(Hover_Text* t, char* msg, float time_limit)
{
    if (msg == NULL || msg[0] == '\0') {
        t->active = false;
        return;
    }
    t->active = true;
    snprintf(t->string, MAX_HOVER_TEXT_LEN, msg);
    t->time.time = 0.0f;
    t->max_time = time_limit;
}

void update_hover_text(Hover_Text* t)
{
    if (!t->active) return;

    if (t->max_time != 0.0f) {
        t->time.time += GetFrameTime();
        if (t->time.time >= t->max_time) {
            t->active = false;
            t->max_time = 0.0f;
        }
    }
    //note: hover text is in screen space
    Rectangle player_tile_rec = (Rectangle) {
        .x = game.player.pos[0] * GAME_TILE_WIDTH,
        .y = game.player.pos[1] * GAME_TILE_HEIGHT,
        .width = GAME_TILE_WIDTH,
        .height = GAME_TILE_HEIGHT,
    };
    Vector2 text_pos = get_centered_text_rec(game.game_font, t->string, player_tile_rec, FONT_SIZE, FONT_SPACING);
    float y_offset = -0.5f;
    if (game.player.pos[1] == 0) {
         text_pos.y += (0.5f * TILE_HEIGHT);
         y_offset *= -1;
     }

    t->pos[0] = text_pos.x;
    t->pos[1] = text_pos.y + (y_offset * GAME_TILE_HEIGHT);
}

bool mouse_in_rec(Vector2 mouse_pos, Rectangle rec) {
    Vector2 mp = mouse_pos;
    return (mp.x >= rec.x && mp.x <= rec.x + rec.width && mp.y >= rec.y && mp.y <= rec.y + rec.height);
}

bool handle_ui_clicks(Vector2 mouse_pos, GameState* g) {
    
    bool showing = game.ui.showing;

    int i = 0;
    Job_State job_status = g->jobs.current_job.status;
    for (i = 0; i < NUM_UI_ELEMENTS; i++) {
        if (mouse_in_rec(mouse_pos, g->ui.shapes[i])) {
            break;
        }
    }
    switch (i) {
        case INVENTORY:
            if (showing) return true;
            break;
        case JOB_OFFER:
            if (job_status == OFFERED) return true;
            break;
        case JOB_ACCEPT_BUTTON:
            if (showing && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && job_status == OFFERED) {
                accept_job(&g->jobs);
                set_hover_text(&g->hover_text,"Got a new payload...", 2.0f);
                return true;
            }
            break;
        case JOB_ACTIVE:
            if (job_status == ACCEPTED && showing) return true;
            break;
        case NUM_UI_ELEMENTS:
            break;
        default: break;
    }
    return false;
}

void handle_input(Entity* p)
{
    //Mouse-Handling
    Vector2 mouse_pos = GetMousePosition();
    //check if hovering over UI element, and then handle accordingly
    bool ui_clicked = handle_ui_clicks(mouse_pos, &game);
    if (!ui_clicked) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Tile* selected = get_selected_tile(game.current_map);
            if (game.selected_tile == selected) {
                register_action(p, game.selected_tile);
            } else {
                game.selected_tile = selected;
                char buffer[MAX_HOVER_TEXT_LEN];
                set_entity_action_text(buffer, game.selected_tile->entity);
                float msg_time = (selected->entity.type == EMPTY) ? 1.0f : 0.0f;
                set_hover_text(&game.hover_text, buffer, msg_time);
            }
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (game.selected_tile != NULL) {
                game.selected_tile = NULL;
            }
            game.hover_text.active = false;
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
        case KEY_I:
            game.ui.showing = !game.ui.showing;
            break;
        default:
            break;
    }

    int world_x = game.world_pos.x;
    int world_y = game.world_pos.y;
    { //::open for map movement ~~~
        //::todo -- copy and modify the copy of the world pos, then update
        if (new_x >= COLS) {
            world_x++;
            if (world_x < WORLD_COLS && game.world[world_y][world_x].walkable) {
                new_x = 0;
            } else {
                new_x = COLS - 1;
                world_x--;
            }
        } else if (new_x < 0) {
            world_x--;
            if (world_x >= 0 && game.world[world_y][world_x].walkable) {
                new_x = COLS - 1;
            } else {
                new_x = 0;
                world_x++;
            }
        }

        if (new_y >= ROWS) {
            world_y++;
            if (world_y < WORLD_ROWS && game.world[world_y][world_x].walkable) {
                new_y = 0;
            } else {
                new_y = ROWS - 1;
                world_y--;
            }
        } else if (new_y < 0) {
            world_y--;
            if (world_y >= 0 && game.world[world_y][world_x].walkable) {
                new_y = ROWS - 1;
            } else {
                new_y = 0;
                world_y++;
            }
        }
    }

    if (game.world[world_y][world_x].walkable) {
        game.world_pos.x = world_x;
        game.world_pos.y = world_y;
        game.current_map = &game.world[game.world_pos.y][game.world_pos.x];
    }

    bool target_walkable   = game.current_map->tiles[new_x][new_y].entity.walkable;
    if (target_walkable) {
        p->pos[0] = new_x;
        p->pos[1] = new_y;
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
                set_hover_text(&game.hover_text, "I'm too far...", 1.0f);
            }
            break;
        }
        case HARVEST: {
            if (entities_adjacent(*p, target->entity)) {
                p->action = HARVEST;
                p->target = &target->entity;
            } else {
                game.hover_text.active = true;
                set_hover_text(&game.hover_text, "I'm too far...", 1.0f);
            }
            break;
        }
        case MINE: {
            if (entities_adjacent(*p, target->entity)) {
                p->action = MINE;
                p->target = &target->entity;
            } else {
                game.hover_text.active = true;
                set_hover_text(&game.hover_text, "I'm too far...", 1.0f);
            }
            break;
        }
        case SELL: {
            if (entities_adjacent(*p, target->entity)) {
                p->action = SELL;
                p->target = &target->entity;
            } else {
                game.hover_text.active = true;
                set_hover_text(&game.hover_text, "I'm too far...", 1.0f);
            }
            break;
        }
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
            break;
        } 
        case HARVEST: {
            harvest_target(p, p->target);
            break;
        }
        case MINE: {
            mine_target(p, p->target);
            break;
        }
        case SELL: {
            sell_job_items(p, p->target);
            break;
        }
        default:
            break;
    }
}

void end_action(Entity* p)
{
    p->action = NO_ACTION;
    p->target = NULL;
}

void sell_job_items(Entity* p, Entity* shop) {
    Job_Manager* jm = &game.jobs;
    Job* job = &game.jobs.current_job;
    if (job->status != ACCEPTED) {
        end_action(p);
        set_hover_text(&game.hover_text, "sell what? to who?", 1.0f);
        return;
    }

    if (!check_job_complete(job) && !(game.debug_mode)) {
        end_action(p);
        set_hover_text(&game.hover_text, "the job isn't done...", 1.0f);
        return;
    }
    //job accepted and complete at this stage
    jm->sell_timer.time += GetFrameTime();
    if (jm->sell_timer.time >= jm->sell_timer.trigger_time) {
        for (int i = 0; i < 3; i++) {
            Drop d = job->requirements[i]; //this is the drop in the requirement
            if (d == NONE) continue;
            int amount = job->amount[i]; // this is the amount we need to finish the job
            if (amount > 0) {
                job->amount[i]--;
                job->in_inventory[i]--;
                job->true_amount--;
                remove_from_inventory(d, game.inventory_count, &game.ui.inventory);
                Physics_Object obj = (Physics_Object) {
                    .start_pos = (Vector2) {p->pos[0] * TILE_WIDTH + (0.5f * HALF_TILE_WIDTH), p->pos[1] * TILE_HEIGHT + (0.5f * HALF_TILE_HEIGHT)},
                    .end_pos   = (Vector2) {shop->pos[0] * TILE_WIDTH + (0.5f * HALF_TILE_WIDTH), shop->pos[1] * TILE_HEIGHT + (0.5f * HALF_TILE_HEIGHT)},
                    .sprite_array = game.sprites.drop_source,
                    .sprite_index = d,
                    .time_to_travel = jm->sell_timer.trigger_time * SELL_TIME_FACTOR
                };
                Vector2 diff = Vector2Subtract(obj.end_pos, obj.start_pos);
                obj.velocity = Vector2Scale(diff, 1.0f / obj.time_to_travel);
                Rectangle s = (Rectangle) {.x = obj.start_pos.x, .y = obj.start_pos.y, .width = HALF_TILE_WIDTH, .height = HALF_TILE_HEIGHT};
                add_to_physics_queue(obj, s);
                break;
            }
        }
        jm->sell_timer.trigger_time *= SELL_TIME_FACTOR;
        if (jm->sell_timer.trigger_time < jm->sell_timer.min_time) {
            jm->sell_timer.trigger_time = jm->sell_timer.min_time;
        }
        jm->sell_timer.time = 0.0f;
    }

    if (job->true_amount == 0) {
        jm->timer.time = 0.0f;
        jm->sell_timer.time = 0.0f;
        jm->sell_timer.trigger_time = jm->sell_timer.max_time;
        job->status = INACTIVE;
        end_action(p);
    }
}

void remove_from_inventory(Drop d, int* state_inventory, Inventory* ui_inventory)
{
    state_inventory[d]--;
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (ui_inventory->slots[i] == d) {
            ui_inventory->slots[i] = NONE;
            break;
        }
    }
}

bool check_job_complete(Job* j)
{
    if (j->status != ACCEPTED) return false;

    return (j->complete[0] && j->complete[1] && j->complete[2]);
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
        add_to_inventory(t->drop, &game); 
        int tile_x = t->pos[0];
        int tile_y = t->pos[1];
        Tile* tile = &game.current_map->tiles[tile_x][tile_y];
        *tile = create_tile(
            tile->previous.type,
            tile->entity.type,
            tile_x, tile_y
        );
        add_to_map_queue(game.current_map, tile_x, tile_y);
        end_action(p);
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
        add_to_inventory(t->drop, &game); 
        int tile_x = t->pos[0];
        int tile_y = t->pos[1];
        Tile* tile = &game.current_map->tiles[tile_x][tile_y];
        *tile = create_tile(
            tile->previous.type,
            tile->entity.type,
            tile_x, tile_y
        );
        add_to_map_queue(game.current_map, tile_x, tile_y);
        end_action(p);
    }

    return;
}

void mine_target(Entity* p, Entity* t)
{
    float current_time = p->timer.time;
    if (current_time > t->action_rate) p->timer.time = current_time = 0.0f;

    if (fmod(current_time, 0.3) == 0) {
        int chance = GetRandomValue(0,2);
        if (chance == 2) {
            add_to_inventory(t->drop, &game);
        }
    }
    if (current_time == 0.0f) {
        t->health -= game.stats.mine_dmg;
    }
    p->timer.time += GetFrameTime();

    if (t->health <= 0) {
        add_to_inventory(t->drop, &game); 
        int tile_x = t->pos[0];
        int tile_y = t->pos[1];
        Tile* tile = &game.current_map->tiles[tile_x][tile_y];
        *tile = create_tile(
            tile->previous.type,
            tile->entity.type,
            tile_x, tile_y
        );
        add_to_map_queue(game.current_map, tile_x, tile_y);
        end_action(p);
    }

    return;
}

void add_to_inventory(Drop drop, GameState* g)
{
    g->inventory_count[drop] += 1;
    Drop* slots = g->ui.inventory.slots;
    int i = 0;
    for (i = 0; i < INVENTORY_SLOTS; i++) {
        if (slots[i] == NONE) {
            slots[i] = drop;
            break;
        }
    }

    if (i == INVENTORY_SLOTS) {
        set_hover_text(&g->hover_text, "My bag is full!", 1.0f);
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
                grow_entity(m, e, i);
                break;
            default:
                break;
        }
    }
}

void grow_entity(Map* m, Entity* e, int index)
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

void update_game(GameState* g)
{
    //::debug behaviour
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
        g->jobs.create_job = false;
    }
    if (g->jobs.current_job.status == ACCEPTED) {
        update_job_requirements(g, &g->jobs.current_job);
    }

    handle_action(&g->player);
    handle_input(&g->player);
    update_hover_text(&g->hover_text);
    if (g->physics_queue.count > 0) {
        handle_physics_queue(&g->physics_queue);
    }
    
    handle_map_queue(g->current_map, &g->current_map->entity_queue);
    
    update_ui_elements(g);
}

void add_to_physics_queue(Physics_Object obj, Rectangle shape)
{
    Physics_Queue* queue = &game.physics_queue;
    obj.shape = shape;
    if (queue->count > MAX_PHYSICS_OBJECTS) {
        printf("ERROR: exceeded physics queue\n");
        return;
    }
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (!queue->queue[i].active) {
            queue->queue[i] = obj;
            queue->queue[i].active = true;
            queue->count++;
            break;
        }
    }
}

void handle_physics_queue(Physics_Queue* queue)
{
    int counter = 0;
    int limit = queue->count;
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (counter == limit) break;
        Physics_Object* obj = &queue->queue[i];
        if (!obj->active) continue;

        counter++;
        float dt = GetFrameTime();
        Vector2 curr_pos = (Vector2) {obj->shape.x, obj->shape.y};
        Vector2 velocity_step = Vector2Scale(obj->velocity, dt);
        Vector2 new_pos = Vector2Add(curr_pos, velocity_step);
        Vector2 to_end_before = Vector2Subtract(obj->end_pos, curr_pos);
        Vector2 to_end_after = Vector2Subtract(obj->end_pos, new_pos);

        if (Vector2DotProduct(to_end_before, to_end_after) <= 0) {
            // passed or exactly reached target
            new_pos = obj->end_pos;
            obj->active = false;
            queue->count--;
        }
        obj->shape.x = roundf(new_pos.x);
        obj->shape.y = roundf(new_pos.y);
    }
}

void update_job_requirements(GameState* g, Job* j)
{
    for (int i = 0; i < 3; i++) {
        Drop d = j->requirements[i];
        j->in_inventory[i] = g->inventory_count[d];
        if (!j->complete[i]) {
            if (j->in_inventory[i] >= j->amount[i]) j->complete[i] = true;
        }
    }
}

void tick_job_queue(Job_Manager* j) 
{
    j->timer.time += GetFrameTime();

    if (j->current_job.status == OFFERED) {
        float accept_time = (game.debug_mode) ? 1.5f : JOB_ACCEPT_TIME;
        if (j->timer.time >= accept_time) {
            j->timer.time = 0.0f;
            j->current_job.status = INACTIVE;
            return; //Job has been offered and not accepted - next frame, clock starts for new job offer
        }
    }

    if (j->current_job.status == INACTIVE && j->timer.time >= NEW_JOB_TIME) {
        j->create_job = true;
        j->timer.time = 0.0f;
    } 
    return;
}

void create_job(Job_Manager* j)
{
    Job job = (Job){0};
    job.status = OFFERED;
    int chance = GetRandomValue(1, 10);
    //printf("%d\n", chance);
    Drop first_drop;
    Drop second_drop;
    for (int i = 0; i < 3; i++) {
        job.complete[i] = true;
        Drop to_add = NONE;
        if (i == 0) {
            to_add = GetRandomValue(1, NUM_DROPS - 1); //::todo: change this to a random from KNOWN drops
            int amount = GetRandomValue(1, 10); //::todo: tier this somehow based on drop rarity
            job.requirements[0] = to_add;
            job.amount[0] = amount;
            job.in_inventory[0] = 0;
            job.complete[0] = false;
            job.true_amount += amount;
            first_drop = to_add;
        }
        if (i == 1 && chance <= 6) {
            do {
                to_add = GetRandomValue(1, NUM_DROPS - 1);
            } while (to_add == first_drop);
            int amount = GetRandomValue(1, 10);
            if (job.true_amount + amount > INVENTORY_SLOTS) {
                amount = INVENTORY_SLOTS - job.true_amount;
            }
            job.requirements[1] = to_add;
            job.amount[1] = amount;
            job.in_inventory[1] = 0;
            job.complete[1] = false;
            job.true_amount += amount;
            second_drop = to_add;
        }
        if (i == 2 && chance <= 3 && job.true_amount < INVENTORY_SLOTS) {
            do {
                to_add = GetRandomValue(1, NUM_DROPS - 1);
            } while (to_add == first_drop || to_add == second_drop);
            int amount = GetRandomValue(1, 10);
            if (job.true_amount + amount > INVENTORY_SLOTS) {
                amount = INVENTORY_SLOTS - job.true_amount;
            }
            job.requirements[2] = to_add;
            job.amount[2] = amount;
            job.in_inventory[2] = 0;
            job.complete[2] = false;
            job.true_amount += amount;
        }
    }
    //::todo: apply some logic to these
    job.time_to_complete = job.time_taken = 0.0f;
    job.reward = 50;

    j->current_job = job; 
}

void accept_job(Job_Manager* jm)
{
    jm->current_job.status = ACCEPTED;
    jm->timer.time = 0.0f;
    jm->sell_timer.time = 0.0f;
    jm->sell_timer.trigger_time = jm->sell_timer.max_time;
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
    src_array[SHOP]   = (Rectangle) {.x = 256.0f, .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[ROCK]   = (Rectangle) {.x = 160.0f, .y = 0.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
    src_array[EMPTY_ROCK] = (Rectangle) {.x = 160.0f, .y = 32.0f, .width = SPRITE_SIZE, .height = SPRITE_SIZE};
}

void load_drop_images(GameState* g)
{
    Rectangle* drop_array = g->sprites.drop_source;
    drop_array[EMPTY] = (Rectangle) {0,0,0,0};
    drop_array[LOG]   = (Rectangle) {.x = 32,  .y = 32, .width = 16, .height = 16};
    drop_array[IVY]   = (Rectangle) {.x = 128, .y = 64, .width = 16, .height = 16};
    drop_array[ORE]   = (Rectangle) {.x = 160, .y = 64, .width = 16, .height = 16};
}

void update_ui_elements(GameState* g)
{

    if (g->jobs.current_job.status == OFFERED) {
        g->ui.offer.accept_percent = (JOB_ACCEPT_TIME - g->jobs.timer.time) / JOB_ACCEPT_TIME; 
    } else {
        g->ui.offer.accept_percent = 1.0f;
    }
}

void draw_display_ui(GameState* g)
{
    Vector2 origin = (Vector2) {0,0};
    Texture2D sheet = g->sprites.spritesheet;
    Rectangle* drop_images = g->sprites.drop_source; 

    if (g->ui.showing) {
        Inventory inventory = g->ui.inventory;
        Rectangle inv_shape = inventory.slot_size;
        DrawRectangleRec(g->ui.inventory.space, P_BLACK);
        for (int i = 0; i < INVENTORY_SLOTS; i++) {
            Drop d = inventory.slots[i];
            Rectangle dest = (Rectangle) {
                .x = inv_shape.x,
                .y = inv_shape.y,
                .width = HALF_GAME_TILE_WIDTH,
                .height = HALF_GAME_TILE_HEIGHT,
            };
            DrawTexturePro(sheet, drop_images[d], dest, origin, 0.0f, P_WHITE);
            inv_shape.x += inv_shape.width;
            if ((i + 1) % 4 == 0) {
                inv_shape.y += inv_shape.height;
                inv_shape.x = inventory.slot_size.x;
            }
        }
    }

    Job current_job = g->jobs.current_job;
    if (current_job.status == OFFERED) {
        DrawRectangleRec(g->ui.offer.space, P_BLACK);
        Rectangle offer_shape = g->ui.offer.slot_size;
        DrawTextEx(g->ui_font, "$0000.00",g->ui.offer.reward_pos, 48, 2, P_YELLOW);
        for (int i = 0; i < 3; i++) {
            Drop d = current_job.requirements[i];
            if (d != NONE) {
                Rectangle dest = (Rectangle) {
                    .x = offer_shape.x,
                    .y = offer_shape.y,
                    .width = GAME_TILE_WIDTH,
                    .height = GAME_TILE_HEIGHT,
                };
                DrawTexturePro(sheet, drop_images[d], dest, origin, 0.0f, P_WHITE);
                offer_shape.x += offer_shape.width;
            }
        }
        DrawRectangleRec(g->ui.offer.accept_button.shape, P_BLACK);
        DrawRectangleLinesEx(g->ui.offer.accept_button.shape, 1.0f, P_LIGHTGREEN);
        Rectangle accept = g->ui.offer.accept_button.shape;
        accept.width = g->ui.offer.accept_percent * g->ui.offer.accept_button.shape.width;
        DrawRectangleRec(accept, P_GREEN);
        DrawTextEx(g->ui_font, "ACCEPT?",g->ui.offer.accept_button.accept_text_pos, 32, 2, P_WHITE);
    }

    if (g->ui.showing) {
        if (current_job.status == ACCEPTED) {
            DrawRectangleRec(g->ui.active_job.space, P_BLACK);
            Rectangle job_req_size = g->ui.active_job.slot_size;
            DrawTextEx(g->ui_font, "Payout: $0000.00",g->ui.active_job.reward_pos, 30, 2, P_YELLOW);

            for (int i = 0; i < 3; i++) {
                Drop d = current_job.requirements[i];
                if (d != NONE) {
                    //text
                    char buffer[10];
                    snprintf(buffer, 10, "%0d/%0d", current_job.in_inventory[i], current_job.amount[i]);
                    Vector2 pos = get_centered_text_rec(g->ui_font, buffer, job_req_size, 28, 2);
                    pos.y += (0.6f * job_req_size.height);
                    Color t_col = (current_job.complete[i]) ? P_LIGHTGREEN : P_WHITE;
                    DrawTextEx(g->ui_font,buffer, pos, 28, 2, t_col);
                    //drop
                    Rectangle dest = {
                        .x = job_req_size.x,
                        .y = job_req_size.y,
                        .width = 0.75f * GAME_TILE_WIDTH,
                        .height = 0.75f * GAME_TILE_HEIGHT,
                    };
                    DrawTexturePro(sheet, drop_images[d], dest, origin, 0.0f, P_WHITE);
                    job_req_size.x += job_req_size.width + INV_INITIAL_OFFSET;
                }
            }
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
    SetTextureWrap(screen.texture, TEXTURE_WRAP_CLAMP);

    create_entities(entities);

    game.sprites.spritesheet = LoadTexture("assets/spritesheet.png");
    SetTextureFilter(game.sprites.spritesheet, TEXTURE_FILTER_POINT);
    SetTextureWrap(game.sprites.spritesheet, TEXTURE_WRAP_CLAMP);

    game.game_font = LoadFont("assets/fonts/Kobata-Bold.otf");
    //game.ui_font   = LoadFont("assets/fonts/3270-Regular.otf");
    game.ui_font   = LoadFont("assets/fonts/Kobata-Regular.otf");
    reset_game(&game);
    load_sprite_sources(&game);
    load_drop_images(&game);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_LEFT_CONTROL)) {
            game.debug_mode = !game.debug_mode;
        }
        //printf("%f\n", GetFrameTime());
        int fps = GetFPS();
        if (fps < 60) printf("FPS dropped to %d\n", fps);
        update_game(&game);
    //Drawing to 320x240 render texture
        BeginTextureMode(screen);
            ClearBackground(WHITE);
            Map m = *game.current_map; 
            for (int i = 0; i < ROWS; i++) {
                for (int j = 0; j < COLS; j++) {
                    Tile m_entity = m.tiles[j][i];
                    Rectangle dest = (Rectangle) {.x = j * TILE_WIDTH, .y = i * TILE_HEIGHT, .width = TILE_WIDTH, .height = TILE_HEIGHT};
                    Rectangle source = game.sprites.source[m_entity.entity.type];
                    Vector2 dest_vec = (Vector2) {0,0};
                    //draw a background for all tiles
                    DrawTexturePro(game.sprites.spritesheet, game.sprites.source[GRASS], dest, dest_vec, 0.0f, WHITE);

                    if (m_entity.entity.type == TREE) {
                        float height_factor = m_entity.entity.health / 100.0f;
                        float chop = 1.0f - height_factor;
                        source.y = source.y + (chop * SPRITE_SIZE);
                        source.height =  height_factor * SPRITE_SIZE;
                        dest.y = ((float) i * TILE_HEIGHT) + (chop * TILE_WIDTH);
                        dest.height = height_factor * TILE_HEIGHT;
                    }
                    DrawTexturePro(game.sprites.spritesheet, source, dest, dest_vec, 0.0f, WHITE);
                    //if (game.debug_mode) DrawRectangleLinesEx((Rectangle){dest.x, dest.y, dest.width, dest.height}, 0.5f, P_WHITE);
                }
            }
            //::draw player
            Entity p = game.player;
            Rectangle dest_rect = (Rectangle) {.x = p.pos[0] * TILE_WIDTH, .y = p.pos[1] * TILE_HEIGHT, .width = TILE_WIDTH, .height = TILE_HEIGHT};
            DrawTexturePro(game.sprites.spritesheet, game.sprites.source[PLAYER], dest_rect, (Vector2) {0,0}, 0.0f, WHITE);
            //::draw selected icon
            if (game.selected_tile != NULL) {
                Entity selected = game.selected_tile->entity;
                int select_x = selected.pos[0];
                int select_y = selected.pos[1];
                DrawCircle(select_x * TILE_WIDTH + HALF_TILE_WIDTH, select_y * TILE_HEIGHT + HALF_TILE_HEIGHT, 2, WHITE);
            }
            //::draw moving objects
            if (game.physics_queue.count > 0) {
                int counter = 0;
                int limit = game.physics_queue.count;
                Physics_Object* q = game.physics_queue.queue;
                for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
                    if (counter == limit) break;
                    if (!q[i].active) {
                        continue;
                    }
                    Rectangle src = q[i].sprite_array[q[i].sprite_index];
                    DrawTexturePro(game.sprites.spritesheet, src, q[i].shape, (Vector2){0,0}, 0.0f, WHITE);
                    counter++;
                    //DrawRectangleLines(q[i].shape.x, q[i].shape.y, q[i].shape.width, q[i].shape.height, GREEN);
                }
            }
        EndTextureMode();

        //Drawing to frame buffer
        BeginDrawing();
            ClearBackground(WHITE);
            DrawTexturePro(
                screen.texture,
                (Rectangle){0, 0, WIDTH, -HEIGHT},
                (Rectangle){0, 0, G_WIDTH, G_HEIGHT},
                (Vector2){0,0},
                0.0f,
                WHITE
            );

            draw_display_ui(&game);
            if (game.debug_mode) {
                DrawTextEx(game.game_font, "Debug", (Vector2) {10, 10}, 48, FONT_SPACING, P_RED);
                char buffer[15];
                snprintf(buffer, 15, "world_pos: %d,%d", game.world_pos.y, game.world_pos.x);
                DrawTextEx(game.game_font, buffer, (Vector2) {500, 10}, 48, FONT_SPACING, P_RED);
                DrawFPS(100, 10);
            }
            if (game.hover_text.active) {
                Hover_Text t = game.hover_text;
                DrawTextEx(game.game_font, t.string,(Vector2){t.pos[0], t.pos[1]}, FONT_SIZE, FONT_SPACING, WHITE);
            }
        EndDrawing();
    }

    UnloadRenderTexture(screen);
    CloseWindow();
    
}