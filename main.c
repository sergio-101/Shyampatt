#include "shared.h"
#include "game.h"

static int SCREEN_WIDTH = 800;
static int SCREEN_HEIGHT = 450;

int main(void)
{
    State GState = SetupBoard(SCREEN_WIDTH, SCREEN_HEIGHT);
    while (!WindowShouldClose())    
    {
        BeginDrawing();
        DrawTexturePro(
            background,
            (Rectangle){ 0, 0, background.width, background.height },  // source
            (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() },   // dest
            (Vector2){ 0, 0 },                                          // origin
            0.0f,                                                        // rotation
            WHITE
        );
        Update(&GState);
        Render(&GState);
        EndDrawing();
    }
    CloseWindow();
    freeAll(&GState);
    return 0;
}
