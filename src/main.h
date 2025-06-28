#ifndef MAIN_H
#define MAIN_H

#define WIDTH 480
#define HEIGHT 320
#define G_WIDTH 1440//(WIDTH * SCALE)
#define G_HEIGHT 960//(HEIGHT * SCALE)

#define P_WHITE      (Color) {254, 246, 221, 255}
#define P_BLACK      (Color) { 13,  28,  36, 255}
#define P_RED        (Color) {151,  59,  57, 255}
#define P_LIGHTGRAY  (Color) {148, 142, 149, 255}
#define P_GRAY       (Color) {125, 132, 159, 255}
#define P_DARKGRAY   (Color) { 26,  43,  45, 255}
#define P_LIGHTGREEN (Color) { 39, 133,  75, 255}
#define P_GREEN      (Color) { 36, 106,  68, 255}
#define P_DARKGREEN  (Color) { 34,  74,  61, 255}
#define P_LIGHTOLIVE (Color) {167, 165,  81, 255}
#define P_OLIVE      (Color) {130, 150,  75, 255}
#define P_DARKOLIVE  (Color) { 93, 136,  82, 255}

#define P_YELLOW     (Color) {217, 176,  46, 255}

#define ROWS 10
#define COLS 15
#define TILE_WIDTH (float) (WIDTH / COLS)
#define HALF_TILE_WIDTH (float) (TILE_WIDTH * 0.5f)
#define TILE_HEIGHT (float) (HEIGHT / ROWS)
#define HALF_TILE_HEIGHT (float) (TILE_HEIGHT * 0.5f)
#define GAME_TILE_WIDTH (float) (G_WIDTH / COLS)
#define GAME_TILE_HEIGHT (float) (G_HEIGHT / ROWS)
#define SPRITE_SIZE 32.0f
#define MAX_HOVER_TEXT_LEN 64
#define FONT_SIZE 26
#define FONT_SPACING 2

#define NUM_MAPS 1
#define MAP_ENTITY_NUM (ROWS * COLS)
#define MAX_PHYSICS_OBJECTS 64
#define INVENTORY_SLOTS 16
#define INV_INITIAL_OFFSET 3
#define INV_Y_POS_FACTOR 0.58f
#define INV_WIDTH_FACTOR 0.25f
#define OFFER_Y_FACTOR 0.0f

#define NEW_JOB_TIME 3.0f
#define JOB_ACCEPT_TIME 5.0f
#define SELL_TIMER_MAX 1.0f
#define SELL_TIMER_MIN 0.4f
#define SELL_TIME_FACTOR 0.8f

typedef enum {
    EMPTY = 0,
    PLAYER,
    TREE,
    STUMP,
    GRASS,
    RUIN,
    CLEAN_RUIN,
    SHOP,
    ROCK,
    EMPTY_ROCK,
    NUM_ENTITY_TYPES
} Entity_Type;

typedef enum {
    NONE = 0,
    LOG,
    IVY,
    ORE,
    NUM_DROPS
} Drop;

typedef enum {
    NO_ACTION = 0,
    CUT,
    GROW,
    HARVEST,
    MINE,
    SELL,
    NUM_ACTIONS
} Action;

typedef enum {
    INACTIVE = 0,
    OFFERED,
    ACCEPTED
} Job_State;

typedef enum {
    INVENTORY = 0,
    JOB_OFFER,
    JOB_ACCEPT_BUTTON,
    JOB_ACTIVE,
    NUM_UI_ELEMENTS,
} UI_ELEMENTS;

typedef struct Timer {
    float time;
    float trigger_time;
    float max_time;
    float min_time;
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
    int mine_dmg;
} Player_Stats;

typedef struct hover_text {
    bool active;
    char string[MAX_HOVER_TEXT_LEN];
    Timer time;
    float max_time;
    int pos[2];
} Hover_Text;

typedef struct button {
    bool clickable;
    Rectangle shape;
    Vector2 accept_text_pos;
} Button;

typedef struct inventory {
    Rectangle space;
    Rectangle slot_size;
    Drop slots[INVENTORY_SLOTS];
} Inventory;

typedef struct job_offer {
    Button accept_button;
    float accept_percent;
    Rectangle space;
    Rectangle slot_size;
    Vector2 reward_pos;
    Vector2 estimate_pos;
} Job_Offer;

typedef struct accepted_job {
    Rectangle space;
    Rectangle slot_size;
    Vector2 reward_pos;
} Accepted_Job;

typedef struct ui {
    Inventory inventory;
    Job_Offer offer;
    Accepted_Job active_job;
    Rectangle shapes[NUM_UI_ELEMENTS];
} UI;

typedef struct job {
    Drop requirements[3];
    int amount[3];
    int in_inventory[3];
    bool complete[3];
    int true_amount;
    float time_to_complete;
    float time_taken;
    int reward;
    Job_State status;
} Job;

typedef struct job_manager {
    Timer timer;
    Timer sell_timer;
    bool create_job;
    Job current_job;
    Job next_job;
} Job_Manager;

typedef struct physics_object {
    bool active;
    Rectangle shape;
    Vector2 start_pos;
    Vector2 end_pos;
    Vector2 velocity;
    float time_to_travel;
    Rectangle* sprite_array;
    int sprite_index;
} Physics_Object;

typedef struct physics_queue {
    Physics_Object queue[MAX_PHYSICS_OBJECTS];
    int count;
} Physics_Queue;

typedef struct game_state {
    Entity player;
    Player_Stats stats;
    int inventory_count[NUM_DROPS];
    Physics_Queue physics_queue;
    Job_Manager jobs;
    Map maps[NUM_MAPS];
    Map* current_map;
    Sprites sprites;
    Hover_Text hover_text;
    UI ui;
    Tile* selected_tile;
    Font game_font;
    Font ui_font;
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
void end_action(Entity* p);
void cut_target(Entity* p, Entity* t);
void harvest_target(Entity* p, Entity* t);
void mine_target(Entity* p, Entity* t);
void add_to_map_queue(Map* m, int x, int y);
void handle_map_queue(Map* m, Entity_Queue* eq);
void grow_entity(Map* m, Entity* e, int index);
bool entities_adjacent(Entity e, Entity target);
void load_sprite_sources(GameState* g);
void set_hover_text(Hover_Text* t, char* msg, float time_limit);
void update_hover_text(Hover_Text* t);
void load_drop_images(GameState* g);
void add_to_inventory(Drop drop, GameState* g);
void remove_from_inventory(Drop d, int* state_inventory, Inventory* ui_inventory);
void draw_display_ui(GameState* g);
void tick_job_queue(Job_Manager* j);
void create_job(Job_Manager* j); 
void update_ui_elements(GameState* g);
bool handle_ui_clicks(Vector2 mouse_pos, GameState* g);
bool mouse_in_rec(Vector2 mouse_pos, Rectangle rec);
void set_entity_action_text(char* b, Entity e);
Vector2 get_centered_text_rec(Font font, const char* t, Rectangle rec, int font_size, int font_spacing);
void update_job_requirements(GameState* g, Job* j);
void sell_job_items(Entity* p, Entity* shop);
bool check_job_complete(Job* j);
void accept_job(Job_Manager* jm);
void add_to_physics_queue(Physics_Object obj, Rectangle shape);
void handle_physics_queue(Physics_Queue* queue);
#endif
