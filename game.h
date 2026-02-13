#pragma once
#include "shared.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define MID_SCREEN (Vector2){GetScreenWidth()/2,GetScreenHeight()/2}
#define WINDOW_SIZE (Vector2){GetScreenWidth(),GetScreenHeight()}
#define INIT_NUMBER_OF_SHAPES_ALLOC 20
#define N_SHAPES 4

static float TOOL_TRAY_MX = 0;
static float TOOL_TRAY_GAP = 5;
static float TOOL_TRAY_WIDTH = 40; 
static float TOOL_TRAY_SLOT_SIDE = 40;
static float TOOL_TRAY_PADDING = 20;
static float margin = 10;

typedef enum Mode {
    Drawing = 0, 
    Free,
    Pointing,
    Selected,
    Reshaping,
    Dragging,
    FOV_Move
} Mode;

typedef enum Edges {
    Left, Top, Right, Bottom, Top_Left, Top_Right, Bottom_Right, Bottom_Left
} Edges;

typedef enum Shapes {
    Rectangle_sh = 0, // _sh (shape)
    Ellipse_sh,
    Line_sh,
    Arrow_sh,
} Shapes;

typedef struct Ellipse {
    float x, y, rh, rv;
} Ellipse;

typedef struct Line{
    Vector2 start;
    Vector2 end;
} Line;

typedef struct Arrow {
    Vector2 start;
    Vector2 end;
    Vector2 h1;
    Vector2 h2;
} Arrow;

typedef struct Object {
    Shapes type;
    float thickness;
    Color stroke;
    Rectangle hitBox;
    bool showHitBox;
    union {
        Rectangle Rect;
        Ellipse El;
        Line Line; 
        Arrow Arr; 
    } shape;
} Object;

typedef struct Tray {
    int selectedIndex;
    float padding;
    float slot_s;
    float gap;
    float height;
    float width;
    float x;
    float y;
    Object TrayObjectBuff[N_SHAPES+1];
} Tray;

typedef struct State {
    int n;
    int allocated;
    int cursor;
    Object *pointingTo;
    Object *selected;
    Object beingDrawn;
    char *instruction;
    bool show_instruction;
    Mode mode;
    Vector2 FOV_O;
    Edges reshapeModeEdge; 
    Vector2 *grabbed;
    Shapes Shape_Equipped;
    Vector2 prevMouse;
    Vector2 Equip_pos;
    Tray tray;
    Texture2D background;
    Object *Objects_buffer;
} State;

State SetupBoard(int w, int h);
void Update(State *GState);
void Render(State *GState);
void freeAll(State *GState);
void CheckShapeChange(State* GState);
void InitTray(State* GState);
