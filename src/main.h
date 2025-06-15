#ifndef MAIN_H
#define MAIN_H

#define WIDTH 320
#define HEIGHT 240
#define SCALE 4
#define G_WIDTH (WIDTH * SCALE)
#define G_HEIGHT (HEIGHT * SCALE)

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
#define FONT_SIZE 18

#define NUM_MAPS 1
#define MAP_ENTITY_NUM (ROWS * COLS)
#define INVENTORY_SLOTS 16
#define INV_INITIAL_OFFSET 3

typedef enum {
    EMPTY = 0,
    PLAYER,
    TREE,
    STUMP,
    GRASS,
    NUM_ENTITY_TYPES
} Entity_Type;

typedef enum {
    NONE = 0,
    LOG,
    NUM_DROPS
} Drop;

typedef enum {
    NO_ACTION = 0,
    CUT,
    GROW,
    NUM_ACTIONS
} Action;

typedef struct Timer {
    float time;
} Timer;

typedef struct Entity {
    int pos[2];
    Entity_Type type;
    bool walkable;
    Action action;
    struct Entity* target;
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

typedef struct game_state {
    Entity player;
    Player_Stats stats;
    int inventory_count[NUM_DROPS];
    Map maps[NUM_MAPS];
    Map* current_map;
    Sprites sprites;
    Hover_Text hover_text;
    Inventory inventory;
    Tile* selected_tile;
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


#endif
