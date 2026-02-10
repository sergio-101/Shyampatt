#include "shared.h"
#include "game.h"

float dist(Vector2 p1, Vector2 p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return sqrtf(dx * dx + dy * dy);
}
bool isOffBy(float p1, float p2, float m){
    return (p2 - m < p1 && p1 < p2 + m);
}

Vector2 APointOnLine(Vector2 p1, Vector2 p2, float ratio){
    float x = p1.x + (ratio * (p2.x - p1.x));
    float y = p1.y + (ratio * (p2.y - p1.y));
    return (Vector2) {x, y};
}

Rectangle AdjustRectangle(Rectangle rect){
   if(rect.width < 0){
            rect.x += rect.width;
            rect.width *= -1;
        }
    if(rect.height < 0){
        rect.y += rect.height;
        rect.height *= -1;
    }
    return rect;
}

Ellipse AdjustEllipse(Ellipse el){
    el.rh = abs(el.rh);
    el.rv = abs(el.rv);
    return el;
}

void DrawHitbox(Object obj){
    obj.hitBox = AdjustRectangle(obj.hitBox);
    DrawRectangleLinesEx((Rectangle){ 
        obj.hitBox.x - margin,
        obj.hitBox.y - margin,
        obj.hitBox.width + margin * 2,
        obj.hitBox.height + margin * 2,
    },  2, RED);
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
        .cursor = MOUSE_CURSOR_DEFAULT,
        .bg = (Color) {0 , 0, 0, 255},
        .prevMouse = (Vector2) {0, 0}
    };
    GState.Objects_buffer = malloc(sizeof(Object) *  INIT_NUMBER_OF_SHAPES_ALLOC);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(w, h, "Shyampatt");
    InitTray(&GState);
    SetTargetFPS(60);               
    return GState;
}

void InitTray(State *GState){
    float ScreenHeight = GetScreenHeight(); 
    GState->tray = (Tray_args) {
        .selectedIndex = 0,
        .padding = margin * 2,
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
            .h1 = (Vector2) {p.x, p.y + margin},
            .h2 = (Vector2) {p.x, p.y - margin},
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
bool isInsideRectangle(Vector2 p, Rectangle Rect){
    return (
        Rect.y < p.y && 
        p.y < Rect.y + Rect.height && 
        Rect.x < p.x && 
        p.x < Rect.x + Rect.width 
    );
}
int isOnEdgeWithMargin(Vector2 p, Rectangle rect, int margin){
    bool InsideOuter = isInsideRectangle(p, (Rectangle) {
            rect.x-margin, 
            rect.y-margin, 
            rect.width+margin*2, 
            rect.height+margin*2
    });
    bool InsideInner = isInsideRectangle(p, (Rectangle) {
            rect.x+margin, 
            rect.y+margin, 
            rect.width-margin*2, 
            rect.height-margin*2
    });
    if(InsideOuter && !InsideInner){
        float x = rect.x;
        float y = rect.y;
        float w = rect.width;
        float h = rect.height;

        Edges edge;
        if (dist(p, (Vector2) {x, y}) < 10) edge = Top_Left;
        else if (dist(p, (Vector2) {x + w, y}) < 10) edge = Top_Right;
        else if (dist(p, (Vector2) {x + w, y + h}) < 10) edge = Bottom_Right;
        else if (dist(p, (Vector2) {x , y + h}) < 10) edge = Bottom_Left;
        else if(isOffBy(p.x, x, margin)) edge = Left;
        else if(isOffBy(p.x, x + w, margin)) edge = Right;
        else if(isOffBy(p.y, y + h, margin)) edge = Bottom;
        else if(isOffBy(p.y, y, margin)) edge = Top;
        return edge;
    }
    else return -1;
}
bool isPointOnObject(Vector2 p, Object obj, int margin){
    switch(obj.type){
        case Rectangle_sh:
            bool insideOuterMargin = isInsideRectangle(
                p,
                (Rectangle) {
                    obj.shape.Rect.x - margin, 
                    obj.shape.Rect.y - margin,
                    obj.shape.Rect.width + margin * 2,
                    obj.shape.Rect.height + margin * 2
                }
            ); 
            bool insideInnerMargin = isInsideRectangle(
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
            if(rx == 0 || ry == 0) return false;

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
            float inLineHitBox = isInsideRectangle(p, 
                (Rectangle) {
                    obj.hitBox.x,
                    obj.hitBox.y,
                    obj.hitBox.width,
                    obj.hitBox.height,
                }
            );
            float distFromEnd = dist(p, obj.shape.Line.end);
            float distFromStart = dist(p, obj.shape.Line.start);
            return (err < margin && inLineHitBox || distFromStart < margin || distFromEnd < margin);
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
                Obj->shape.Arr.h1 = (Vector2) {p.x, p.y + margin};
                Obj->shape.Arr.h2 = (Vector2) {p.x, p.y - margin};
                Obj->hitBox = (Rectangle) {
                    fmin(epos.x, mpos.x), fmin(epos.y, mpos.y),
                    fmax(epos.x, mpos.x) - fmin(epos.x, mpos.x),
                    fmax(epos.y, mpos.y) - fmin(epos.y, mpos.y),
                };
                break;
            }
       }
    }
}

void HandleDragging(State *GState){
    Vector2 epos = GetMousePosition();
    if(GState->mode == Dragging){
        Object *obj = GState->selected;
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
}


void HandleReshaping(State *GState){
    Vector2 epos = GetMousePosition();
    if(GState->mode == Reshaping){
       Object *obj = GState->selected;
       Vector2 shift = (Vector2){epos.x - GState->Equip_pos.x, epos.y - GState->Equip_pos.y};
       Edges edge = GState->reshapeModeEdge;
       switch(obj->type){
           case Rectangle_sh:
               if(edge == Right || edge == Top_Right || edge == Bottom_Right) obj->shape.Rect.width += shift.x;
               if(edge == Bottom || edge == Bottom_Right || edge == Bottom_Left) obj->shape.Rect.height += shift.y;;
               if(edge == Left || edge == Top_Left || edge == Bottom_Left) {
                   obj->shape.Rect.x += shift.x;
                   obj->shape.Rect.width -= shift.x;
               }
               if(edge == Top || edge == Top_Right || edge == Top_Left){
                   obj->shape.Rect.y += shift.y;
                   obj->shape.Rect.height -= shift.y;
               }
               obj->hitBox = obj->shape.Rect;
               break;
       
           case Ellipse_sh:
               if(edge == Right || edge == Top_Right || edge == Bottom_Right) {
                   obj->shape.El.x += shift.x/2;
                   obj->shape.El.rh += shift.x/2;
               }
               if(edge == Bottom || edge == Bottom_Right || edge == Bottom_Left) {
                   obj->shape.El.y += shift.y/2;
                   obj->shape.El.rv += shift.y/2;
               }
               if(edge == Left || edge == Top_Left || edge == Bottom_Left) {
                   obj->shape.El.x += shift.x/2;
                   obj->shape.El.rh -= shift.x/2;
               }
               if(edge == Top || edge == Top_Right || edge == Top_Left){
                   obj->shape.El.y += shift.y/2;
                   obj->shape.El.rv -= shift.y/2;
               }
               obj->hitBox = (Rectangle) {
                   obj->shape.El.x - abs(obj->shape.El.rh), obj->shape.El.y - abs(obj->shape.El.rv), 
                   abs(obj->shape.El.rh) * 2, abs(obj->shape.El.rv) * 2
               };
               break;

           case Line_sh:{
               Vector2 *grabbed = GState->grabbed;
               if(grabbed){
                    grabbed->x += shift.x; 
                    grabbed->y += shift.y; 
                    Vector2 start = obj->shape.Line.start;
                    Vector2 end = obj->shape.Line.end;
                    obj->hitBox = (Rectangle) {
                        fmin(start.x, end.x), 
                        fmin(start.y, end.y),
                        fmax(start.x, end.x) - fmin(start.x, end.x),
                        fmax(start.y, end.y) - fmin(start.y, end.y),
                    };
               }
               break;
           }
           case Arrow_sh:{
               Vector2 *grabbed = GState->grabbed;
               if(grabbed){
                    grabbed->x += shift.x; 
                    grabbed->y += shift.y; 
                    Vector2 start = obj->shape.Arr.start;
                    Vector2 end = obj->shape.Arr.end;

                    Vector2 p = APointOnLine(start, end, 0.9);
                    obj->shape.Arr.h1 = (Vector2) {p.x, p.y + margin};
                    obj->shape.Arr.h2 = (Vector2) {p.x, p.y - margin};
                    obj->hitBox = (Rectangle) {
                        fmin(start.x, end.x), 
                        fmin(start.y, end.y),
                        fmax(start.x, end.x) - fmin(start.x, end.x),
                        fmax(start.y, end.y) - fmin(start.y, end.y),
                    };
               }
               break;
          }
       }
       GState->Equip_pos = epos;
    }
}
void MoveObject(Object *obj, Vector2 vec){
    float x = vec.x;
    float y = vec.y;
    switch(obj->type){
        case Rectangle_sh:
            obj->shape.Rect.x += x;
            obj->shape.Rect.y += y;
            break;
        case Ellipse_sh:
            obj->shape.El.x += x;
            obj->shape.El.y += y;
            break;
        case Line_sh:
            obj->shape.Line.start.x += x;
            obj->shape.Line.start.y += y;
            obj->shape.Line.end.x += x;
            obj->shape.Line.end.y += y;
            break;
        case Arrow_sh:
            obj->shape.Arr.start.x += x;
            obj->shape.Arr.start.y += y;

            obj->shape.Arr.end.x += x;
            obj->shape.Arr.end.y += y;

            obj->shape.Arr.h1.x += x;
            obj->shape.Arr.h1.y += y;

            obj->shape.Arr.h2.x += x;
            obj->shape.Arr.h2.y += y;
            break;
    }
}
void HandleFOV(State* GState){
    if(GState->mode == FOV_Move){
        Vector2 epos = GetMousePosition();
        Vector2 shift = (Vector2){epos.x - GState->Equip_pos.x, epos.y - GState->Equip_pos.y};

        for(int i = 0; i <= GState->n; ++i){
            Object *obj = &GState->Objects_buffer[i];
            MoveObject(obj, shift);
            obj->hitBox.x += shift.x;
            obj->hitBox.y += shift.y;
        }


        GState->Equip_pos = epos;
    }
}
void Update(State *GState){
    TrackScrollWheel(GState);
    Vector2 epos = GetMousePosition();
    int KeyPressed = GetKeyPressed();

    if(KeyPressed >= KEY_ONE && KeyPressed <= KEY_NINE){
        ChangeShapeToIndex(KeyPressed - KEY_ONE, GState);
    }

    // FOV Grabbed
    if(IsKeyDown(KEY_SPACE)){
        GState->cursor = MOUSE_CURSOR_RESIZE_ALL;
    }

    // Mouse Moves when FOV isn't grabbed
    else if(!(epos.x == GState->prevMouse.x && epos.y == GState->prevMouse.y)){
        GState->prevMouse = epos;
        Object *onBorderObj = NULL;
        for (int i = 0; i <= GState->n; ++i){
            Object *obj = &GState->Objects_buffer[i];
            if(isPointOnObject(epos, *obj, margin)){
                onBorderObj = obj;
            }
        } 
        GState->pointingTo = onBorderObj;
        if(GState->mode == Free) GState->cursor = onBorderObj ? MOUSE_CURSOR_POINTING_HAND: MOUSE_CURSOR_DEFAULT;
        if(GState->mode == Selected){
            Object *obj = GState->selected;
            float x = obj->hitBox.x;
            float y = obj->hitBox.y;
            float w = obj->hitBox.width;
            float h = obj->hitBox.height;
            bool isInsideOuterBound = isInsideRectangle(epos, (Rectangle){x - margin, y - margin, w + margin * 2, h + margin * 2 });  
            if(!isInsideOuterBound) GState->cursor = MOUSE_CURSOR_DEFAULT;
            else {
                if(obj->type == Ellipse_sh || obj->type == Rectangle_sh){
                    int edge = isOnEdgeWithMargin(epos, obj->hitBox, margin);
                    if(edge < 0) GState->cursor = MOUSE_CURSOR_RESIZE_ALL;
                    switch(edge){
                        // Vertices
                        case Top_Right:
                        case Bottom_Left:
                            GState->cursor = MOUSE_CURSOR_RESIZE_NESW;
                            break;

                        case Top_Left:
                        case Bottom_Right:
                            GState->cursor = MOUSE_CURSOR_RESIZE_NWSE;
                            break;

                        // Actual Sides. Bad terminology, I know.
                        case Top:
                        case Bottom:
                            GState->cursor = MOUSE_CURSOR_RESIZE_NS;
                            break;

                        case Left:
                        case Right:
                            GState->cursor = MOUSE_CURSOR_RESIZE_EW;
                            break;
                    }
                }
                else{
                    float distFromEnd = dist(epos, obj->type == Line_sh? obj->shape.Line.end: obj->shape.Arr.end);
                    float distFromStart = dist(epos, obj->type == Line_sh? obj->shape.Line.start: obj->shape.Arr.start);
                    if(distFromEnd < margin || distFromStart < margin) GState->cursor = MOUSE_CURSOR_POINTING_HAND;
                    else{
                        GState->cursor = MOUSE_CURSOR_RESIZE_ALL;
                    }
                }
            }
        }
    }
    

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        // CTRL -> FOV Changes 
        if(IsKeyDown(KEY_SPACE)){
            GState->mode = FOV_Move;
            GState->Equip_pos = epos; 
        }
        // FREE & POINTING -> POINTING
        else if(GState->mode == Free && GState->pointingTo){
            GState->mode = Selected;
            GState->selected = GState->pointingTo;
        }

        // FREE & NOT-POINTING -> DRAW 
        else if(GState->mode == Free && !GState->pointingTo){
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

        // CLICK WHILE SELECTED 
        else if(GState->mode == Selected){
            Object *obj = GState->selected;
            bool InsideOuterBound = isInsideRectangle(epos, (Rectangle) {
                    obj->hitBox.x-margin, 
                    obj->hitBox.y-margin, 
                    obj->hitBox.width+margin*2, 
                    obj->hitBox.height+margin*2
            });

            // IF NOT INSIDE HITBOX -> FREE
            if(!InsideOuterBound){
                if(GState->pointingTo){
                    GState->selected = GState->pointingTo; 
                }
                else {
                    GState->mode = Free;
                    GState->selected = NULL; 
                }
            }

            // DRAGGING / RESHAPE;
            else{
                Object *obj = GState->selected;
                if(obj->type == Ellipse_sh || obj->type == Rectangle_sh){
                    int edge = isOnEdgeWithMargin(epos, obj->hitBox, margin);
                    if(edge < 0){
                        GState->Equip_pos = epos; 
                        GState->mode = Dragging;
                    }
                    else{
                        switch(edge){
                            // Vertices
                            case Top_Right:
                            case Bottom_Left:
                                GState->cursor = MOUSE_CURSOR_RESIZE_NESW;
                                break;

                            case Top_Left:
                            case Bottom_Right:
                                GState->cursor = MOUSE_CURSOR_RESIZE_NWSE;
                                break;

                            // Actual Sides. Bad terminology, I know.
                            case Top:
                            case Bottom:
                                GState->cursor = MOUSE_CURSOR_RESIZE_NS;
                                break;

                            case Left:
                            case Right:
                                GState->cursor = MOUSE_CURSOR_RESIZE_EW;
                                break;
                        }
                        GState->Equip_pos = epos; 
                        GState->mode = Reshaping;
                        GState->reshapeModeEdge = edge;
                    }
                }
                else{
                    float d_start = dist(epos, obj->type == Line_sh? obj->shape.Line.start: obj->shape.Arr.start);
                    float d_end= dist(epos, obj->type == Line_sh? obj->shape.Line.end: obj->shape.Arr.end);
                    if(d_end < margin || d_start < margin){
                        if(d_start == fmin(d_start, d_end)){
                            GState->grabbed = &obj->shape.Line.start;
                        }
                        else {
                            GState->grabbed = &obj->shape.Line.end;
                        }
                        GState->mode = Reshaping;
                    }
                    else{
                        GState->grabbed = NULL;
                        GState->mode = Dragging;
                    }
                    GState->Equip_pos = epos; 
                }
            }
        }
    }

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        if(GState->mode == Drawing){
           Object Obj = GState->beingDrawn;
           Obj.stroke = GRAY;
           Obj.showHitBox = false;
           GState->Objects_buffer[GState->n] = Obj;
           GState->n++;
           GState->mode= Free;
        }
        if(GState->mode == Reshaping){
            switch(GState->selected->type){
                case Rectangle_sh:
                    Rectangle adjustedRect = AdjustRectangle(GState->selected->shape.Rect);
                    GState->selected->shape.Rect = adjustedRect;
                    GState->selected->hitBox = adjustedRect;
                    break;
                case Ellipse_sh:
                    GState->selected->shape.El = AdjustEllipse(GState->selected->shape.El);
                    break;
            }
            GState->mode = Selected;
        }
        if(GState->mode == Dragging){
            GState->mode = Selected;
        }
        if(GState->mode == FOV_Move){
            GState->mode = Free;
        }
    }
    HandleDrawing(GState);
    HandleDragging(GState);
    HandleReshaping(GState);
    HandleFOV(GState);
}

void DrawObject(Object obj){
    switch(obj.type){
        case Rectangle_sh:  
            obj.shape.Rect = AdjustRectangle(obj.shape.Rect);
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
        DrawHitbox(obj);
    }
}


void Render(State *GState){  
    if(IsWindowResized()){
        InitTray(GState);
    }
    ClearBackground(GState->bg);


    // Main Board Objects
    for(int i = 0; i <= GState->n; ++i){
        Object obj = GState->Objects_buffer[i];
        DrawObject(obj);
    }

    // hitbox on the object selected;
    if(GState->mode == Selected){
        Object obj = *GState->selected;
        obj.stroke = WHITE;
        DrawHitbox(obj);
        DrawObject(obj);
        if(obj.type == Line_sh){
            Line res = obj.shape.Line;
            DrawCircle(res.start.x, res.start.y, 4, WHITE);
            DrawCircle(res.end.x, res.end.y, 4, WHITE);
        }
        else if(obj.type == Arrow_sh){
            Arrow res = obj.shape.Arr;
            DrawCircle(res.start.x, res.start.y, 4, WHITE);
            DrawCircle(res.end.x, res.end.y, 4, WHITE);
        };
    }

    if(GState->mode == Drawing) DrawObject(GState->beingDrawn);

    // Tool Tray
    DrawRectangle(GState->tray.x, GState->tray.y, GState->tray.width, GState->tray.height, BLACK);
    DrawRectangle(GState->tray.x, GState->tray.y + GState->tray.selectedIndex * GState->tray.slot_w, GState->tray.slot_w, GState->tray.slot_w, WHITE);
    for(int i = 0; i < N_SHAPES + 1; ++i){
        DrawObject(GState->TrayObjectBuff[i]);
    }

    SetMouseCursor(GState->cursor);
}
