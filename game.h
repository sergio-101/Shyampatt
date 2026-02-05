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

static float TOOL_TRAY_WIDTH = 60; 
static float MAX_TOOL_TRAY_HEIGHT = 400;
static float TOOL_TRAY_MX = 50;
static float margin = 10;

typedef enum Mode {
    Drawing = 0, 
    Free,
    Pointing,
    Selected,
    Reshaping,
    Dragging,
} Mode;

typedef enum Side {
    Left, Top, Right, Bottom 
} Side;

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

typedef struct Tray_args {
    int selectedIndex;
    float padding;
    float slot_w;
    float gap;
    float height;
    float width;
    float x;
    float y;
} Tray_args;

typedef struct State {
    Object *Objects_buffer;
    Object TrayObjectBuff[N_SHAPES+1];
    int n;
    int allocated;
    int cursor;
    Object *pointingTo;
    Object *selected;
    Object beingDrawn;
    Mode mode;
    Side reshapeModeSide; 
    Vector2 *grabbed;
    Shapes Shape_Equipped;
    Vector2 prevMouse;
    Color bg;
    Vector2 Equip_pos;
    struct Tray_args tray;
} State;

State SetupBoard(int w, int h);
void Update(State *GState);
void Render(State *GState);
void freeAll(State *GState);
void CheckShapeChange(State* GState);
void InitTray(State* GState);
