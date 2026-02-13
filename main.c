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
        Update(&GState);
        Render(&GState);
        EndDrawing();
    }
    CloseWindow();
    freeAll(&GState);
    return 0;
}
