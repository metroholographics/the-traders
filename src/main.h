#ifndef MAIN_H
#define MAIN_H

#define WIDTH 320
#define HEIGHT 240
#define SCALE 4
#define G_WIDTH (WIDTH * SCALE)
#define G_HEIGHT (HEIGHT * SCALE)

#define P_WHITE (Color) {254, 246, 221, 255}
#define P_RED (Color) {225, 32, 45, 255}

#define ROWS 12
#define COLS 16
#define TILE_WIDTH (float) (WIDTH / COLS)
#define HALF_TILE_WIDTH (float) (TILE_WIDTH * 0.5f)
#define TILE_HEIGHT (float) (HEIGHT / ROWS)
#define HALF_TILE_HEIGHT (float) (TILE_HEIGHT * 0.5f)
#define GAME_TILE_WIDTH (float) (G_WIDTH / COLS)
#define GAME_TILE_HEIGHT (float) (G_HEIGHT / ROWS)
#define SPRITE_SIZE 32.0f
#define MAX_HOVER_TEXT_LEN 64
#define FONT_SIZE 24
#define FONT_SPACING 2

#define NUM_MAPS 1
#define MAP_ENTITY_NUM (ROWS * COLS)
#define INVENTORY_SLOTS 16
#define INV_INITIAL_OFFSET 3
#define INV_Y_POS_FACTOR 0.58f
#define INV_WIDTH_FACTOR 0.25f

#define NEW_JOB_TIME 2.0f

typedef enum {
    EMPTY = 0,
    PLAYER,
    TREE,
    STUMP,
    GRASS,
    RUIN,
    CLEAN_RUIN,
    NUM_ENTITY_TYPES
} Entity_Type;

typedef enum {
    NONE = 0,
    LOG,
    IVY,
    NUM_DROPS
} Drop;

typedef enum {
    NO_ACTION = 0,
    CUT,
    GROW,
    HARVEST,
    NUM_ACTIONS
} Action;

typedef enum {
    INACTIVE = 0,
    OFFERED,
    ACCEPTED
} Job_State;

typedef struct Timer {
    float time;
} Timer;

typedef struct Entity {
    int pos[2];
    Entity_Type type;
    bool walkable;
    Action action;
    struct Entity* target;
    int max_health;
    int health;
    float action_rate;
    int growth_per_tick;
    Timer timer;
    Drop drop;
} Entity;

typedef struct EntityQueue {
    Entity* queue[MAP_ENTITY_NUM];
    int count;
} Entity_Queue;

typedef struct tile {
    Entity entity;
    Entity previous;
} Tile;

typedef struct Sprite {
    Texture2D spritesheet;
    Rectangle source[NUM_ENTITY_TYPES];
    Rectangle drop_source[NUM_DROPS];
} Sprites;

typedef struct map {
    Tile tiles[COLS][ROWS];
    Entity_Queue entity_queue;
    int biome;
} Map;

typedef struct player_stats {
    int woodcut_dmg;
} Player_Stats;

typedef struct hover_text {
    bool active;
    char string[MAX_HOVER_TEXT_LEN];
    Timer time;
    int pos[2];
} Hover_Text;

typedef struct inventory {
    Rectangle space;
    Rectangle slot_size;
    Drop slots[INVENTORY_SLOTS];
} Inventory;

typedef struct ui {
    Rectangle canvas;
    Inventory inventory;
} UI;

typedef struct job {
    Drop requirements[3];
    int amount[3];
    float time_to_complete;
    float time_taken;
    int reward;
    Job_State status;
} Job;

typedef struct job_manager {
    Timer timer;
    bool create_job;
    Job current_job;
    Job next_job;
} Job_Manager;

typedef struct game_state {
    Entity player;
    Player_Stats stats;
    int inventory_count[NUM_DROPS];
    Job_Manager jobs;
    Map maps[NUM_MAPS];
    Map* current_map;
    Sprites sprites;
    Hover_Text hover_text;
    Inventory inventory;
    Tile* selected_tile;
    Font game_font;
    bool debug_mode;
} GameState;

Map empty_map(void);
void reset_game(GameState* g);
void create_entities(Entity* e);
void update_game(GameState* g);
void handle_input(Entity* p);
Vector2 screen_to_world(int x, int y);
Tile create_tile(Entity_Type current, Entity_Type previous, int x, int y);
Tile* get_selected_tile(Map* m);
void register_action(Entity* p, Tile* target);
bool tile_in_bounds(int x, int y);
void handle_action(Entity* p);
void cut_target(Entity* p, Entity* t);
void harvest_target(Entity* p, Entity* t);
void add_to_map_queue(Map* m, int x, int y);
void handle_map_queue(Map* m, Entity_Queue* eq);
void grow_stump(Map* m, Entity* e, int index);
bool entities_adjacent(Entity e, Entity target);
void load_sprite_sources(GameState* g);
void set_hover_text(Hover_Text* t, Tile target, char* msg);
void update_hover_text(Hover_Text* t);
void load_drop_images(GameState* g);
void add_to_inventory(Drop drop, GameState* g);
void draw_display_ui(GameState* g);
void tick_job_queue(Job_Manager* j);
void create_job(Job_Manager* j); 


#endif
