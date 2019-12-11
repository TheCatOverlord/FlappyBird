// Include stuffs
#include <raylib.h>
#include <stdlib.h>
#include <time.h>

#define NUM_OF_PIPES 500

// ENUM for the current screen
typedef enum GameScreen { LOGO, TITLE, GAMEPLAY } GameScreen;

// struct for player data
typedef struct Player
{
    // Our textures
    Texture2D flap;
    Texture2D idle;
    Texture2D dead;
    // Positions
    Vector2 pos;
    Vector2 centerOffset;
    // Physics
    Vector2 velocity;
    Vector2 colliderPos;
    Vector2 colliderSize;
    int jumpForce;
    // misc
    bool isDead;
    bool isColliding;
    int currentTexture;
    float scale;
    int score;
} Player;

typedef struct floorDataStruct
{
    // Texture
    Texture2D texture;
    // Position
    Vector2 pos;
    // Collisions
    Vector2 colliderPos;
    Vector2 colliderSize;
    // Misc
    bool isColliding;

} floorDataStruct;

typedef struct Sky 
{
    Texture2D texture;
    Vector2 pos; 
} Sky;

typedef struct Pipe
{
    // Textures;
    Texture2D texture;
    Texture2D textureFlipped;
    Vector2 pos;

    // Misc
    float scale;
    bool isCollected;
    bool isCollidingTop;
    bool isCollidingBot;
    bool isCollidingScore;

    // Collisions
    Vector2 colliderPosTop;
    Vector2 colliderSizeTop;
    Vector2 colliderPosBot;
    Vector2 colliderSizeBot;
    Vector2 colliderPosScore;
    Vector2 colliderSizeScore;
    int scoreHeight;
} Pipe;

// Global Vars
Player bird = { 0 };
Sky sky = { 0 };
floorDataStruct floorData = { 0 };
GameScreen screen = { 0 };
Pipe pipes[NUM_OF_PIPES] = { 0 };
bool debug = false;
// how many pixels per meter
int worldScale = 60; 
int frameCounter = 0;

void UpdateBirdAnimation(void)
{
    if (!bird.isDead)
    {
        if ((frameCounter/10)%2 == 0)
        { bird.currentTexture = 0; }
        if ((frameCounter/10)%2 == 1)
        { bird.currentTexture = 1; }
    } else bird.currentTexture = 2;
}

void UpdateBirdColliderPos(void)
{ bird.colliderPos = (Vector2) { bird.pos.x - bird.colliderSize.x / 2, bird.pos.y - bird.colliderSize.y / 2 }; }

void CheckCollisions(void)
{
    bird.isColliding = false;
    if (CheckCollisionRecs( (Rectangle) { bird.colliderPos.x, bird.colliderPos.y, bird.colliderSize.x, bird.colliderSize.y },
                            (Rectangle) { floorData.colliderPos.x, floorData.colliderPos.y, floorData.colliderSize.x, floorData.colliderSize.y }))
                            { bird.isColliding = true; floorData.isColliding = true; bird.isDead = true; } else { floorData.isColliding = false; }
    for (int i = 0; i < NUM_OF_PIPES; i++)
    {
        // Check bird + top of pipe
        if (CheckCollisionRecs( (Rectangle) { bird.colliderPos.x, bird.colliderPos.y, bird.colliderSize.x, bird.colliderSize.y },
                                (Rectangle) { pipes[i].colliderPosTop.x, pipes[i].colliderPosTop.y, pipes[i].colliderSizeTop.x, pipes[i].colliderSizeTop.y }))
                                { pipes[i].isCollidingTop = true; bird.isColliding = true; bird.isDead = true; } else { pipes[i].isCollidingTop = false; }

        // Check bird + bottom of pipe
        if (CheckCollisionRecs( (Rectangle) { bird.colliderPos.x, bird.colliderPos.y, bird.colliderSize.x, bird.colliderSize.y },
                                (Rectangle) { pipes[i].colliderPosBot.x, pipes[i].colliderPosBot.y, pipes[i].colliderSizeBot.x, pipes[i].colliderSizeBot.y }))
                                { pipes[i].isCollidingBot = true; bird.isColliding = true; bird.isDead = true; } else { pipes[i].isCollidingBot = false; }

        // Check bird + score of pipe
        if (CheckCollisionRecs( (Rectangle) { bird.colliderPos.x, bird.colliderPos.y, bird.colliderSize.x, bird.colliderSize.y },
                                (Rectangle) { pipes[i].colliderPosScore.x, pipes[i].colliderPosScore.y, pipes[i].colliderSizeScore.x, pipes[i].colliderSizeScore.y }))
                                { pipes[i].isCollidingScore = true; bird.isColliding = true; if (!pipes[i].isCollected) {bird.score++;} pipes[i].isCollected = true; } else { pipes[i].isCollidingScore = false; }

    }
}

void CheckSkyPos(void)
{
    if (sky.pos.x < -sky.texture.width)
    { sky.pos.x = 0; }
}

void UpdatePhysics(void)
{
    floorData.pos.x -= bird.velocity.x * GetFrameTime() * worldScale;
    sky.pos.x -= (bird.velocity.x / 2)* GetFrameTime() * worldScale;
    bird.velocity.y = bird.velocity.y + (GetFrameTime() * 9.8 * worldScale);

    for (int i = 0; i < NUM_OF_PIPES; i++)
    {
        pipes[i].pos.x -= bird.velocity.x * GetFrameTime() * worldScale;
        pipes[i].colliderPosBot = (Vector2) { pipes[i].pos.x -((pipes[i].texture.width / 2) / pipes[i].scale), pipes[i].pos.y + ((pipes[i].scoreHeight / 2) / pipes[i].scale)};
        pipes[i].colliderSizeBot = (Vector2) { pipes[i].texture.width / pipes[i].scale, pipes[i].texture.height / pipes[i].scale };
        pipes[i].colliderPosTop = (Vector2) { pipes[i].pos.x -((pipes[i].texture.width / 2) / pipes[i].scale), pipes[i].pos.y -((pipes[i].texture.height + (pipes[i].scoreHeight / 2)) / pipes[i].scale)};
        pipes[i].colliderSizeTop = (Vector2) { pipes[i].texture.width / pipes[i].scale, pipes[i].texture.height / pipes[i].scale };
        pipes[i].colliderPosScore = (Vector2) { pipes[i].pos.x -((pipes[i].texture.width / 2) / pipes[i].scale), pipes[i].pos.y -((pipes[i].scoreHeight / 2) / pipes[i].scale)};
        pipes[i].colliderSizeScore = (Vector2) { pipes[i].texture.width / pipes[i].scale, pipes[i].scoreHeight / 2};
    }

    // Bird hit the floor
    if (bird.isDead && !debug)
    {
        bird.velocity = (Vector2){ 0, 0 };
        bird.currentTexture = 2;
    }

    bird.pos.y = bird.pos.y + (bird.velocity.y * GetFrameTime());
}

void UpdateFloorColliderPos(void)
{ floorData.colliderPos = (Vector2) { floorData.pos.x, floorData.pos.y + floorData.texture.height / 2 }; }

void CheckFloorPos(void)
{
    if (floorData.pos.x < -floorData.texture.width)
    { floorData.pos.x = 0; }
    UpdateFloorColliderPos();
}

Pipe GenPipe(int x, int y, Texture2D texture, int scoreHeight, float scale)
{
    Pipe pipeTmp = { 0 };
    Image imageTmp;
    pipeTmp.pos = (Vector2) { x, y };
    pipeTmp.colliderPosBot = (Vector2) { x -((texture.width / 2) / scale), y + ((scoreHeight / 2) / scale)};
    pipeTmp.colliderSizeBot = (Vector2) { texture.width / scale, texture.height / scale };
    pipeTmp.colliderPosTop = (Vector2) { x -((texture.width / 2) / scale), y -((texture.height + (scoreHeight / 2)) / scale)};
    pipeTmp.colliderSizeTop = (Vector2) { texture.width / scale, texture.height / scale };
    pipeTmp.colliderPosScore = (Vector2) { x -((texture.width / 2) / scale), y -((scoreHeight / 2) / scale)};
    pipeTmp.colliderSizeScore = (Vector2) { texture.width / scale, scoreHeight / 2};
    pipeTmp.texture = texture;
    imageTmp = GetTextureData(texture);
    ImageFlipVertical(&imageTmp);
    pipeTmp.textureFlipped = LoadTextureFromImage(imageTmp);
    pipeTmp.scale = scale;
    pipeTmp.scoreHeight = scoreHeight;
    pipeTmp.isCollected = false;
    pipeTmp.isCollidingTop = false;
    pipeTmp.isCollidingBot = false;
    pipeTmp.isCollidingScore = false;
    return pipeTmp;
}

int main(void)
{
    // Init Raylib
    const int screenWidth = 600;
    const int screenHeight = 600;
    srand(time(NULL));
    InitWindow(screenWidth, screenHeight, "Flappy bird");

    // Init the screen
    screen = LOGO;

    // Init the sky
    sky.texture = LoadTexture("SkyTileSprite.png");
    sky.pos = (Vector2) { 0, -sky.texture.height * 1/4 };

    // Init the floorData
    floorData.texture = LoadTexture("GrassThinSprite.png");
    floorData.pos = (Vector2) { 0, (screenHeight * 7/8) - floorData.texture.height / 2 };
    floorData.colliderPos = (Vector2) { floorData.pos.x, floorData.pos.y + floorData.texture.height / 2 };
    floorData.colliderSize = (Vector2) { floorData.texture.width * 2, floorData.texture.height / 2 };

    // Init the player
    bird.flap = LoadTexture("flap.png");
    bird.idle = LoadTexture("idle.png");
    bird.dead = LoadTexture("dead.png");
    bird.scale = 2.0f;
    bird.score = 0;
    bird.centerOffset.x = (bird.idle.width / 2.0f) / bird.scale;
    bird.centerOffset.y = (bird.idle.height / 2.0f) / bird.scale;
    bird.pos.x = screenWidth/4;
    bird.pos.y = screenHeight/2;
    bird.currentTexture = 0;
    bird.jumpForce = 5;
    bird.velocity = (Vector2) { 3, 0 };
    bird.colliderSize = (Vector2) { 112 / bird.scale, 68 / bird.scale };
    UpdateBirdColliderPos();

    // Init the pipes
    for (int i = 0; i < NUM_OF_PIPES; i++)
    {
        pipes[i] = GenPipe(screenWidth + 100 + (i * 5 * worldScale), (rand()% 6 * worldScale) + 2 * worldScale, LoadTexture("ColumnSprite.png"), 300, 2.0f);
    }

    // Init other varibles
    bool showFPS = false;
    bool paused = false;
    int screenWidthRT;
    int screenHeightRT;


    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        // Update
        switch (screen)
        {
            case LOGO:
            {
                frameCounter++;
                if (frameCounter > 180)
                {
                    screen = TITLE;
                    frameCounter = 0;
                }
            } break;

            case TITLE:
            {
                frameCounter++;
                if (IsKeyPressed(KEY_ENTER)) { screen = GAMEPLAY; frameCounter = 0; } 
            } break;

            case GAMEPLAY:
            {
                frameCounter++;
                if (IsKeyPressed(KEY_F)) showFPS = !showFPS;
                if (IsKeyPressed(KEY_P)) paused = !paused;
                if (IsKeyPressed(KEY_D)) debug = !debug;
                if (!paused)
                {
                    if (bird.velocity.y < 0)
                    { UpdateBirdAnimation(); }
                    UpdateBirdColliderPos();
                    CheckFloorPos();
                    CheckSkyPos();
                    CheckCollisions();
                    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    { bird.velocity.y = -bird.jumpForce * worldScale; }
                    UpdatePhysics();
                    UpdateBirdColliderPos();
                    UpdateFloorColliderPos();
                }
            } break;
            default: break;
        }
        // Draw
        screenWidthRT = GetScreenWidth();
        screenHeightRT = GetScreenHeight();
        BeginDrawing();
            ClearBackground(BLACK);
            switch (screen)
            {
                case LOGO:
                {
                    DrawText("FireWire.dev", 20, 20, 20, LIGHTGRAY);
                } break;

                case TITLE:
                {
                    DrawText("Flappy Bird", screenWidthRT/2 - MeasureText("Flappy Bird", 40)/2, screenHeightRT / 2, 40, GRAY);
                    if ((frameCounter/30)%2 == 0) DrawText("PRESS [ENTER] to START", screenWidthRT/2 - MeasureText("PRESS [ENTER] to START", 20)/2, screenHeightRT/2 + 60, 20, DARKGRAY);
                } break;

                case GAMEPLAY:
                {
                    // Draw the Sky
                    DrawTextureEx(sky.texture, sky.pos, 0.0f, 1.0f, WHITE);
                    DrawTextureEx(sky.texture, (Vector2) { sky.pos.x + sky.texture.width, sky.pos.y }, 0.0f, 1.0f, WHITE);
                    // Draw the pipes
                    for (int i = 0; i < NUM_OF_PIPES; i++)
                    {
                        DrawTextureEx(pipes[i].texture, pipes[i].colliderPosBot, 0.0f, 1.0f / pipes[i].scale, WHITE);
                        DrawTextureEx(pipes[i].textureFlipped, pipes[i].colliderPosTop, 0.0f, 1.0f / pipes[i].scale, WHITE);
                    }
                    
                    // Draw the floor
                    DrawTextureEx(floorData.texture, floorData.pos, 0.0f, 1.0f, WHITE);
                    DrawTextureEx(floorData.texture, (Vector2) { floorData.pos.x + floorData.texture.width, floorData.pos.y }, 0.0f, 1.0f, WHITE);
                    // Draw the bird
                    switch (bird.currentTexture)
                    {
                        case 0: { DrawTextureEx(bird.idle, (Vector2) { bird.pos.x - bird.centerOffset.x, bird.pos.y - bird.centerOffset.y }, 0.0f, 1.0f / bird.scale, WHITE); } break;
                        case 1: { DrawTextureEx(bird.flap, (Vector2) { bird.pos.x - bird.centerOffset.x, bird.pos.y - bird.centerOffset.y }, 0.0f, 1.0f / bird.scale, WHITE); } break;
                        case 2: { DrawTextureEx(bird.dead, (Vector2) { bird.pos.x - bird.centerOffset.x, bird.pos.y - bird.centerOffset.y }, 0.0f, 1.0f / bird.scale, WHITE); } break;
                        default: break;
                    }
                    DrawText(TextFormat("Score: %d", bird.score), 20, 20, 15, WHITE);

                    if (debug)
                    {
                        if (bird.isColliding) DrawRectangleLines(bird.colliderPos.x, bird.colliderPos.y, bird.colliderSize.x, bird.colliderSize.y, RED);
                        else DrawRectangleLines(bird.colliderPos.x, bird.colliderPos.y, bird.colliderSize.x, bird.colliderSize.y, GREEN);
                        if (floorData.isColliding) DrawRectangleLines(floorData.colliderPos.x, floorData.colliderPos.y, floorData.colliderSize.x, floorData.colliderSize.y, RED);
                        else DrawRectangleLines(floorData.colliderPos.x, floorData.colliderPos.y, floorData.colliderSize.x, floorData.colliderSize.y, GREEN);

                        for (int i = 0; i < NUM_OF_PIPES; i++)
                        {
                            if (pipes[i].isCollidingTop)
                            { DrawRectangleLines(pipes[i].colliderPosTop.x, pipes[i].colliderPosTop.y, pipes[i].colliderSizeTop.x, pipes[i].colliderSizeTop.y, RED); }
                            else DrawRectangleLines(pipes[i].colliderPosTop.x, pipes[i].colliderPosTop.y, pipes[i].colliderSizeTop.x, pipes[i].colliderSizeTop.y, GREEN);
                            if (pipes[i].isCollidingBot)
                            { DrawRectangleLines(pipes[i].colliderPosBot.x, pipes[i].colliderPosBot.y, pipes[i].colliderSizeBot.x, pipes[i].colliderSizeBot.y, RED); }
                            else DrawRectangleLines(pipes[i].colliderPosBot.x, pipes[i].colliderPosBot.y, pipes[i].colliderSizeBot.x, pipes[i].colliderSizeBot.y, GREEN);
                            if (pipes[i].isCollidingScore)
                            { DrawRectangleLines(pipes[i].colliderPosScore.x, pipes[i].colliderPosScore.y, pipes[i].colliderSizeScore.x, pipes[i].colliderSizeScore.y, RED); }
                            else DrawRectangleLines(pipes[i].colliderPosScore.x, pipes[i].colliderPosScore.y, pipes[i].colliderSizeScore.x, pipes[i].colliderSizeScore.y, GREEN);

                        }

                    }
                    if (paused && !bird.isDead) DrawText("PAUSED", screenWidthRT / 2 - MeasureText("PAUSED", 40) / 2, screenHeightRT/2, 40, LIGHTGRAY );
                    if (bird.isDead) DrawText("You've died!", screenWidthRT / 2 - MeasureText("You've died!", 40) / 2, screenHeightRT/2, 40, LIGHTGRAY );
                } break;
                default: break;
            }
            if (showFPS) DrawFPS(screenWidthRT - 30, screenHeightRT - 30);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
