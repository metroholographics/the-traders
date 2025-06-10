#ifndef MAIN_H
#define MAIN_H

#define WIDTH 320
#define HEIGHT 240
#define SCALE 4
#define G_WIDTH (WIDTH * SCALE)
#define G_HEIGHT (HEIGHT * SCALE)

#define ROWS 12
#define COLS 16
#define TILE_WIDTH (WIDTH / COLS)
#define HALF_TILE_WIDTH (TILE_WIDTH * 0.5f)
#define TILE_HEIGHT (HEIGHT / ROWS)
#define HALF_TILE_HEIGHT (TILE_HEIGHT * 0.5f)

#define NUM_MAPS 1

typedef enum {
    EMPTY,
    PLAYER,
    TREE,
    NUM_ENTITY_TYPES
} Entity_Type;

typedef enum {
    NO_ACTION,
    CUT,
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
    Timer timer;
} Entity;

typedef struct tile {
    Entity entity;
    Entity previous;
} Tile;

typedef struct map {
    Tile tiles[COLS][ROWS];
    int biome;
} Map;

typedef struct game_state {
    Entity player;
    Tile* selected_tile;
    Map maps[NUM_MAPS];
    Texture2D images[NUM_ENTITY_TYPES];
} GameState;

Map empty_map(void);
void reset_game(GameState* g);
void create_entities(Entity* e);
void update_game(GameState* g);
void handle_input(Entity* p);
Vector2 screen_to_world(int x, int y);
Tile create_tile(Entity current, Entity previous, int x, int y);
Tile* get_selected_tile(Map* m);
void register_action(Entity* p, Tile* target);
bool tile_in_bounds(int x, int y);
void handle_action(Entity* p);
void cut_target(Entity* p, Entity* t);


#endif