#include "shared.h"
#include "game.h"

Vector2 APointOnLine(Vector2 p1, Vector2 p2, float ratio){
    float x = p1.x + (ratio * (p2.x - p1.x));
    float y = p1.y + (ratio * (p2.y - p1.y));
    return (Vector2) {x, y};
}

void freeAll(State* GState){
    free(GState->Objects_buffer);
}

State SetupBoard(int w, int h){
    State GState = {
        .n = 0,
        .TrayObjectBuff = {0},
        .allocated = INIT_NUMBER_OF_SHAPES_ALLOC,
        .mode = Free,
        .Shape_Equipped = Rectangle_sh,
        .bg = (Color) {0 , 0, 0, 255},
        .prevMouse = (Vector2) {0, 0}
    };
    GState.Objects_buffer = malloc(sizeof(Object) *  INIT_NUMBER_OF_SHAPES_ALLOC);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(w, h, "Backbuffer");
    InitTray(&GState);
    SetTargetFPS(60);               
    return GState;
}

void InitTray(State *GState){
    float ScreenHeight = GetScreenHeight(); 
    GState->tray = (Tray_args) {
        .selectedIndex = 0,
        .padding = 20,
        .slot_w = TOOL_TRAY_WIDTH,
        .height = MAX_TOOL_TRAY_HEIGHT,
        .width = TOOL_TRAY_WIDTH,
        .x = TOOL_TRAY_MX,
        .y = ScreenHeight/2 - MAX_TOOL_TRAY_HEIGHT/2,
        .gap = 5,
    };

    Object *TrayBuff = GState->TrayObjectBuff;
    Object Tray = (Object) {
        .type = Rectangle_sh,
        .thickness = 3,
        .stroke = GRAY,
        .shape.Rect = (Rectangle) {
            GState->tray.x, 
            GState->tray.y, 
            GState->tray.width, 
            GState->tray.height, 
        }
    };
    TrayBuff[0] = Tray;

    Vector2 slot = (Vector2) {
        GState->tray.x, 
        GState->tray.y
    };
    float slot_w = GState->tray.slot_w; 
    float padding = GState->tray.padding; 
    float gap = GState->tray.gap;

    Object RectObj = (Object) {
        .type = Rectangle_sh,
        .thickness = 3,
        .stroke = GRAY,
        .shape.Rect = (Rectangle) {
            .x = slot.x + padding/2, 
            .y = slot.y + padding/2,
            .width = slot_w - padding,
            .height = slot_w - padding 
        }
    };
    TrayBuff[1] = RectObj;
    slot.y += slot_w + gap;

    Object ElObj = (Object) {
        .type = Ellipse_sh,
        .stroke = GRAY,
        .thickness = 3,
        .shape.El = (Ellipse) {
            .x = slot.x + slot_w/2, 
            .y = slot.y + slot_w/2, 
            .rh = slot_w/2 - padding/2,
            .rv = slot_w/2 - padding/2,
        },
    };
    TrayBuff[2] = ElObj;
    slot.y += slot_w + gap;

    Object LineObj = (Object) {
        .type = Line_sh,
        .stroke = GRAY,
        .thickness = 3,
        .shape.Line = (Line) {
            .start = (Vector2) {slot.x + padding/2, slot.y + slot_w - padding},
            .end = (Vector2) {slot.x + slot_w - padding/2, slot.y + padding/2}
        },
    };
    TrayBuff[3] = LineObj;
    slot.y += slot_w + gap;

    Vector2 start = (Vector2) {slot.x + padding/2, slot.y + slot_w/2 - padding/2};
    Vector2 end = (Vector2) {slot.x + slot_w - padding/2, slot.y + slot_w/2 - padding/2};
    Vector2 p = APointOnLine(start, end, 0.7);
    Object ArrowObj = (Object) {
        .type = Arrow_sh,
        .stroke = GRAY,
        .thickness = 3,
        .shape.Arr = (Arrow) {
            .start = start,
            .end = end,
            .h1 = (Vector2) {p.x, p.y + 10},
            .h2 = (Vector2) {p.x, p.y - 10},
        },
    };
    TrayBuff[4] = ArrowObj;
    slot.y += slot_w;

}
void ChangeShapeToIndex(int i, State* GState){
    switch(i){
        case 0:
            GState->Shape_Equipped = Rectangle_sh;
            break;
        case 1:
            GState->Shape_Equipped = Ellipse_sh;
            break;
        case 2:
            GState->Shape_Equipped = Line_sh;
            break;
        case 3:
            GState->Shape_Equipped = Arrow_sh;
            break;
    }
    GState->tray.selectedIndex = i;
}

void TrackScrollWheel(State* GState){
    Vector2 scroll = GetMouseWheelMoveV();
    if(0.7 < scroll.y){
        GState->tray.selectedIndex--;
    }
    else if(scroll.y < -0.7){
        GState->tray.selectedIndex++;
    }
    int remainder = GState->tray.selectedIndex % N_SHAPES ;
    GState->tray.selectedIndex = remainder< 0 ? N_SHAPES + remainder: remainder;
    ChangeShapeToIndex(GState->tray.selectedIndex, GState);
}
bool isInRectangle(Vector2 p, Rectangle Rect){
    return (
        Rect.y < p.y && 
        p.y < Rect.y + Rect.height && 
        Rect.x < p.x && 
        p.x < Rect.x + Rect.width 
    );
}
bool isPointOnObjectBorder(Vector2 p, Object obj, int margin){
    switch(obj.type){
        case Rectangle_sh:
            bool insideOuterMargin = isInRectangle(
                p,
                (Rectangle) {
                    obj.shape.Rect.x - margin, 
                    obj.shape.Rect.y - margin,
                    obj.shape.Rect.width + margin * 2,
                    obj.shape.Rect.height + margin * 2
                }
            ); 
            bool insideInnerMargin = isInRectangle(
                p,
                (Rectangle) {
                    obj.shape.Rect.x + margin, 
                    obj.shape.Rect.y + margin,
                    obj.shape.Rect.width - margin * 2,
                    obj.shape.Rect.height - margin * 2
                }
            ); 
            return insideOuterMargin && !insideInnerMargin;

        case Ellipse_sh:
            float dx = p.x - obj.shape.El.x;
            float dy = p.y - obj.shape.El.y;
            float rx = obj.shape.El.rh;
            float ry = obj.shape.El.rv;
            // Ellipse equation: (x/rx)^2 + (y/ry)^2
            float distance = (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry);
            float outerBound = 1.0f + margin / fmin(rx, ry);
            float innerBound = 1.0f - margin / fmin(rx, ry);
            return distance >= innerBound && distance <= outerBound;

        case Line_sh:{
            float x1 = obj.shape.Line.start.x;
            float y1 = obj.shape.Line.start.y;
            float x2 = obj.shape.Line.end.x;
            float y2 = obj.shape.Line.end.y;
            float m = (y2 - y1)/(x2 - x1); 
            float c = y1 - m * x1;
            float c_dash = p.y - m * p.x;
            float err = abs(c - c_dash);
            float inLineHitBox = isInRectangle(p, 
                (Rectangle) {
                    obj.hitBox.x,
                    obj.hitBox.y,
                    obj.hitBox.width,
                    obj.hitBox.height,
                }
            );
            return (err < margin && inLineHitBox);
        }
        case Arrow_sh:{
            float x1 = obj.shape.Arr.start.x;
            float y1 = obj.shape.Arr.start.y;
            float x2 = obj.shape.Arr.end.x;
            float y2 = obj.shape.Arr.end.y;
            float m = (y2 - y1)/(x2 - x1); 
            float c = y1 - m * x1;
            float c_dash = p.y - m * p.x;
            float err = abs(c - c_dash);
            return err < margin;
        }
    }
}
void HandleDrawing(State *GState){
    Vector2 epos = GetMousePosition();

    // Initiate Drawing 
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && GState->mode == Free){
        GState->Equip_pos = epos; 
        GState->mode = Drawing;
        Object Obj = {
            .type = GState->Shape_Equipped,
            .thickness = 3,
            .stroke = GRAY,
            .showHitBox = true,
        };
        switch(GState->Shape_Equipped){
            case Rectangle_sh:
                Obj.shape.Rect = (Rectangle) {0, 0, 0, 0};
                break;

            case Ellipse_sh:
                Obj.shape.El = (Ellipse) {0, 0, 0, 0};
                break;

            case Line_sh:
                Obj.shape.Line = (Line) {epos, (Vector2){0, 0}};
                break;

            case Arrow_sh:
                Obj.shape.Arr = (Arrow) {
                    epos, 
                    (Vector2){0, 0},
                    (Vector2){0, 0},
                    (Vector2){0, 0},
                };
                break;
        };
        GState->beingDrawn = Obj;
    }
    
    // Updating Drawing;
    if(GState->mode == Drawing){
        Vector2 epos = GState->Equip_pos;
        Vector2 mpos = GetMousePosition();
        Object *Obj = &GState->beingDrawn;
        Obj->stroke = WHITE;
        switch(GState->Shape_Equipped){
            case Rectangle_sh:{
                float x = fmin(epos.x, mpos.x);
                float y = fmin(epos.y, mpos.y);
                float w = abs((int)mpos.x -(int) epos.x);
                float h = abs((int)mpos.y - (int)epos.y);
                Obj->shape.Rect.x = x;
                Obj->shape.Rect.y = y;
                Obj->shape.Rect.width = w;
                Obj->shape.Rect.height = h;
                Obj->hitBox = (Rectangle) {
                    x, y, w, h
                };
                break;
            }
            case Ellipse_sh:{
                float x = (mpos.x + epos.x) / 2;
                float y = (mpos.y + epos.y) / 2;
                float rv = abs((int)mpos.y - (int)epos.y)/2;
                float rh = abs((int)mpos.x - (int)epos.x)/2;
                Obj->shape.El.x = x;
                Obj->shape.El.y = y;
                Obj->shape.El.rh= rh;
                Obj->shape.El.rv = rv;
                Obj->hitBox = (Rectangle) {
                    x - rh, y - rv, 
                    rh * 2, rv * 2
                };
                break;
            }
            case Line_sh:{
                Obj->shape.Line.end.x = mpos.x;
                Obj->shape.Line.end.y = mpos.y;
                Obj->hitBox = (Rectangle) {
                    fmin(epos.x, mpos.x), 
                    fmin(epos.y, mpos.y),
                    fmax(epos.x, mpos.x) - fmin(epos.x, mpos.x),
                    fmax(epos.y, mpos.y) - fmin(epos.y, mpos.y),
                };
                break;
            }
            case Arrow_sh:{
                Obj->shape.Arr.end.x = mpos.x;
                Obj->shape.Arr.end.y = mpos.y;
                Vector2 p = APointOnLine(epos, mpos, 0.9);
                Obj->shape.Arr.h1 = (Vector2) {p.x, p.y + 10};
                Obj->shape.Arr.h2 = (Vector2) {p.x, p.y - 10};
                Obj->hitBox = (Rectangle) {
                    fmin(epos.x, mpos.x), fmin(epos.y, mpos.y),
                    fmax(epos.x, mpos.x) - fmin(epos.x, mpos.x),
                    fmax(epos.y, mpos.y) - fmin(epos.y, mpos.y),
                };
                break;
            }
       }
    }

    // Ending Drawing  
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && GState->mode == Drawing){
       Object Obj = GState->beingDrawn;
       Obj.stroke = GRAY;
       Obj.showHitBox = false;
       GState->Objects_buffer[GState->n] = Obj;
       GState->n++;
       GState->mode= Free;

    }
}
void HandleDragging(State *GState){
    Vector2 epos = GetMousePosition();
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && GState->mode == Pointing){
        GState->Equip_pos = epos; 
        GState->mode = Dragging;
    }
    if(GState->mode == Dragging){
        Object *obj = GState->pointingTo;
        switch(obj->type){
            case Rectangle_sh:
                obj->shape.Rect.x += epos.x - GState->Equip_pos.x;
                obj->shape.Rect.y += epos.y - GState->Equip_pos.y;
                break;
            case Ellipse_sh:
                obj->shape.El.x += epos.x - GState->Equip_pos.x;
                obj->shape.El.y += epos.y - GState->Equip_pos.y;
                break;
            case Line_sh:
                obj->shape.Line.start.x += epos.x - GState->Equip_pos.x;
                obj->shape.Line.start.y += epos.y - GState->Equip_pos.y;
                obj->shape.Line.end.x += epos.x - GState->Equip_pos.x;
                obj->shape.Line.end.y += epos.y - GState->Equip_pos.y;
                break;
            case Arrow_sh:
                obj->shape.Arr.h1.x += epos.x - GState->Equip_pos.x;
                obj->shape.Arr.h1.y += epos.y - GState->Equip_pos.y;
                obj->shape.Arr.h2.x += epos.x - GState->Equip_pos.x;
                obj->shape.Arr.h2.y += epos.y - GState->Equip_pos.y;

                obj->shape.Arr.start.x += epos.x - GState->Equip_pos.x;
                obj->shape.Arr.start.y += epos.y - GState->Equip_pos.y;
                obj->shape.Arr.end.x += epos.x - GState->Equip_pos.x;
                obj->shape.Arr.end.y += epos.y - GState->Equip_pos.y;
                break;
        }
        obj->hitBox.x += epos.x - GState->Equip_pos.x;
        obj->hitBox.y += epos.y - GState->Equip_pos.y;
        GState->Equip_pos = epos;
    }
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && GState->mode == Dragging){
       GState->mode= Free;
    }

}
void Update(State *GState){
    TrackScrollWheel(GState);
    Vector2 epos = GetMousePosition();
    int KeyPressed = GetKeyPressed();

    if(KeyPressed > 0){
        ChangeShapeToIndex(KeyPressed - KEY_ONE, GState);
    }

    // Hovering -> appears hitbox 
    if(!(epos.x == GState->prevMouse.x && epos.y == GState->prevMouse.y) && (GState->mode == Free || GState->mode == Pointing) ){
        GState->prevMouse = epos;
        Object *hoveredObj = NULL;
        for (int i = 0; i <= GState->n; ++i){
            Object *obj = &GState->Objects_buffer[i];
            if(isPointOnObjectBorder(epos, *obj, 10)){
                hoveredObj = obj;
            }
        }
        if(hoveredObj){
            GState->mode = Pointing;
            GState->pointingTo = hoveredObj;
        }
        else {
            GState->pointingTo = NULL;
            GState->mode = Free;
        }
    }
    HandleDrawing(GState);
    HandleDragging(GState);
}

void DrawObject(Object obj){
    switch(obj.type){
        case Rectangle_sh:  
            DrawRectangleLinesEx((Rectangle){ 
                    obj.shape.Rect.x,
                    obj.shape.Rect.y, 
                    obj.shape.Rect.width, 
                    obj.shape.Rect.height
            },  obj.thickness, obj.stroke);
            break;
        case Ellipse_sh:
            DrawEllipseLines(
                obj.shape.El.x,
                obj.shape.El.y,
                obj.shape.El.rh,
                obj.shape.El.rv,
                obj.stroke
            );
            DrawEllipseLines(
                obj.shape.El.x,
                obj.shape.El.y,
                obj.shape.El.rh+1,
                obj.shape.El.rv+1,
                obj.stroke
            );
            DrawEllipseLines(
                obj.shape.El.x,
                obj.shape.El.y,
                obj.shape.El.rh+2,
                obj.shape.El.rv+2,
                obj.stroke
            );
            break;

        case Line_sh:
            DrawLineEx(
                (Vector2) {obj.shape.Line.start.x, obj.shape.Line.start.y},
                (Vector2) {obj.shape.Line.end.x, obj.shape.Line.end.y},
                obj.thickness,
                obj.stroke
            );
            break;

        case Arrow_sh:
            DrawLineEx(
                (Vector2) {obj.shape.Arr.start.x, obj.shape.Arr.start.y},
                (Vector2) {obj.shape.Arr.end.x, obj.shape.Arr.end.y},
                obj.thickness,
                obj.stroke
            );
            DrawLineEx(
                (Vector2) {obj.shape.Arr.h1.x, obj.shape.Arr.h1.y}, 
                (Vector2) {obj.shape.Arr.end.x, obj.shape.Arr.end.y},
                obj.thickness,
                obj.stroke
            );
            DrawLineEx(
                (Vector2) {obj.shape.Arr.h2.x, obj.shape.Arr.h2.y},
                (Vector2) {obj.shape.Arr.end.x, obj.shape.Arr.end.y},
                obj.thickness,
                obj.stroke
            );
            break;
    }
    if(obj.showHitBox){
        DrawRectangleLinesEx((Rectangle){ 
            obj.hitBox.x,
            obj.hitBox.y,
            obj.hitBox.width,
            obj.hitBox.height,
        },  1, RED);
    }
}


void Render(State *GState){  
    if(IsWindowResized()){
        InitTray(GState);
    }
    ClearBackground(GState->bg);

    // Tool Tray
    DrawRectangle(GState->tray.x, GState->tray.y + GState->tray.selectedIndex * GState->tray.slot_w, GState->tray.slot_w, GState->tray.slot_w, WHITE);
    for(int i = 0; i < N_SHAPES + 1; ++i){
        DrawObject(GState->TrayObjectBuff[i]);
    }

    // Main Board Objects
    for(int i = 0; i <= GState->n; ++i){
        Object obj = GState->Objects_buffer[i];
        DrawObject(obj);
    }

    // hitbox on the object being pointed at;
    if(GState->mode == Pointing){
        Object obj = *GState->pointingTo;
        DrawRectangleLinesEx((Rectangle){ 
            obj.hitBox.x,
            obj.hitBox.y,
            obj.hitBox.width,
            obj.hitBox.height,
        },  1, RED);
    }

   if(GState->mode == Drawing) DrawObject(GState->beingDrawn);

    // cursor maniup
    switch(GState->mode){
        case Drawing:
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            break;

        case Free:
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            break;

        case Dragging:
        case Pointing:
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            break;
    }
}
