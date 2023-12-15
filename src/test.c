#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 600

#define PLAYER_WIDTH 24
#define PLAYER_HEIGHT 40
#define PLAYER_MAX_SPEED 0.3
#define PLAYER_ACCEL 0.002
#define PLAYER_JUMP_SPEED 0.35
#define PLAYER_FRICTION 0.001

#define GRAVITY 0.0005

#define BOX_MIN_SPEED 1
#define BOX_MAX_SPEED 3
#define BOX_SPAWN_RATE 2

#define CAMERA_SPEED 0.0007

enum Direction {
    RIGHT,
    LEFT,
    UP,
    DOWN
};

typedef struct Box Box;
struct Box
{
    Rectangle rect;
    Color color;
    float speed;
    Box* next;
};

Box* boxes = NULL;

void SpawnBox(float playerY);
void DrawBoxes();
void MoveBoxes();
void BoxesCollide();
void BoxesCollideGround(Rectangle ground);

int main(void) { 
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Template-4.0.0");

    double lastTime = GetTime();
    double time = lastTime;

    Vector2 gravity = {0};
    gravity.y = GRAVITY;

    Rectangle ground;
    ground.x = 0;
    ground.y = SCREEN_HEIGHT - 50;
    ground.height = SCREEN_HEIGHT;
    ground.width = SCREEN_WIDTH;

    Vector2 playerMovement;
    playerMovement.x = 0;
    playerMovement.y = 0;

    Rectangle player = {0};
    player.x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2; 
    player.y = SCREEN_HEIGHT - 50 - PLAYER_HEIGHT;
    player.height = PLAYER_HEIGHT;
    player.width = PLAYER_WIDTH;

    Camera2D camera = { 0 };
    camera.target = (Vector2) { player.x + player.width/2, player.y + player.height/2 + -200};
    camera.offset = (Vector2) { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    int score = 0;

    double lastSpawnTime = 0;
    bool canJump;

    int boxCollidePlayer;

    bool gameOver = false;
    int colDir;

    while (!WindowShouldClose() && !gameOver) {
        time = GetTime();
        canJump = false;
        boxCollidePlayer = false;
        // Calculate physics every 60 seconds
        if( time - lastTime > 1 / 60) { 
            if(time - lastSpawnTime > BOX_SPAWN_RATE) {
                SpawnBox(player.y);
                lastSpawnTime = time;
            }

            // Add gravity to movement
            playerMovement = Vector2Add(playerMovement, gravity);

            // Move player on keypress
            if(IsKeyDown(KEY_A)){
                if(fabs(playerMovement.x) < PLAYER_MAX_SPEED){
                    playerMovement.x -= PLAYER_ACCEL;
                }
            }
            if(IsKeyDown(KEY_D)){
                if(fabs(playerMovement.x) < PLAYER_MAX_SPEED){
                    playerMovement.x += PLAYER_ACCEL;
                }
            }

            // Slow down player due to friction
            if(playerMovement.x > 0.0){
                if (playerMovement.x < PLAYER_FRICTION) {
                    playerMovement.x = 0;
                } else {
                    playerMovement.x -= PLAYER_FRICTION;
                }
            } else if(playerMovement.x < 0.0){
                if (playerMovement.x > -PLAYER_FRICTION) {
                    playerMovement.x = 0;
                } else {
                    playerMovement.x += PLAYER_FRICTION;
                }
            }
            
            BoxesCollide();
            BoxesCollideGround(ground);

            MoveBoxes();
            // Check if box collided with player
            for(Box* iter = boxes; iter != NULL; iter = iter->next){
                while(CheckCollisionRecs(player, iter->rect)){ 
                    player.y += 0.1;
                    playerMovement.y = 0;
                    boxCollidePlayer = true;
                }
            }
            // Move player
            player.x += playerMovement.x;
            player.y += playerMovement.y;

            // If outside of screen
            if (player.x < 0) {
                player.x = 0;
            } else if (player.x + player.width > SCREEN_WIDTH) {
                player.x = SCREEN_WIDTH - player.width;
            }
            
            // If touching ground
            if(CheckCollisionRecs(player, ground)){
                // Move outside of ground
                while(CheckCollisionRecs(player, ground)) {
                    player.y -= 0.01;
                }
                playerMovement.y = 0;

                canJump = true;
                colDir = DOWN;
            }

            // Check if player is colliding with any boxes
            for(Box* iter = boxes; iter != NULL; iter = iter->next){
                if(CheckCollisionRecs(player, iter->rect)){ 
                    // Check which side of the box the player hit
                    player.x -= playerMovement.x;
                    if(!CheckCollisionRecs(player, iter->rect)){ // Collided along the x axis
                        if (playerMovement.x > 0) { // Collided on left of box
                            colDir = LEFT;
                        } else { // Collided on right of box
                            colDir = RIGHT;
                        }
                    } else {    // Collided along the y axis
                        if (playerMovement.y > 0) { // Collided on top of box
                            colDir = UP;
                        } else { // Collided on bottom of box
                            colDir = DOWN;
                        }
                    }
                    player.x += playerMovement.x;

                    // Move player out of box
                    while(CheckCollisionRecs(player, iter->rect)) {
                        switch (colDir)
                        {
                        case UP:
                            player.y -= 0.01;
                            playerMovement.y = 0;
                            break;
                        case DOWN:
                            player.y += 0.01;
                            playerMovement.y = 0;
                            break;
                        case RIGHT:
                            player.x += 0.1;
                            break;
                        case LEFT:
                            player.x -= 0.1;
                            break;
                        default:
                            break;
                        }
                    }

                    if (colDir != DOWN) {
                        canJump = true;
                    }  
                    if (boxCollidePlayer && colDir == DOWN) {
                        gameOver = true;
                    }
                }
            }

            // Jump on keyPress if touching ground or wall
            if(IsKeyDown(KEY_W) && canJump){
                playerMovement.y = 0;
                playerMovement.y -= PLAYER_JUMP_SPEED;
                if(colDir == RIGHT) {
                    playerMovement.x += PLAYER_JUMP_SPEED * 2;
                } else if(colDir == LEFT) {
                    playerMovement.x -= PLAYER_JUMP_SPEED * 2;
                }
            }

            // Calculate score
            if (ground.y - (player.y + player.height) > score) {
                score = ground.y - (player.y + player.height);
            }
            // Camera follows player
            camera.target = Vector2Add(
                camera.target, 
                (Vector2){ 0, ((player.y - 150 - camera.target.y) * 4) * CAMERA_SPEED }
            );

            // Update last time physics were calculated
            lastTime = time;
        }

        // Draw everything
        BeginDrawing();
            BeginMode2D(camera);
                ClearBackground(RAYWHITE);
                DrawRectangleRec(player, BLACK);
                DrawRectangleRec(ground, BROWN);
                DrawBoxes();
            EndMode2D();
            DrawText(TextFormat("Score: %d", score), 10, 10, 25, BLACK);
            if (gameOver){
                DrawText(TextFormat("GAME OVER"), 50, 50, 25, BLACK);
            }

        EndDrawing();
    }

    while(!WindowShouldClose() && gameOver) {
        WaitTime(0.1);
    }
    CloseWindow();
    
    return 0;
}

void SpawnBox(float playerY) {
    Box* newBox = MemAlloc(sizeof(Box));
    
    newBox->color = (Color) {GetRandomValue(30, 200), GetRandomValue(30, 200), GetRandomValue(30, 200), 255};
    newBox->next = NULL;
    newBox->speed = (float) GetRandomValue(BOX_MIN_SPEED, BOX_MAX_SPEED) / 10.0;
    newBox->rect.width = GetRandomValue(30, 200);
    newBox->rect.height = GetRandomValue(30, 200);
    newBox->rect.x = GetRandomValue(0, SCREEN_WIDTH - newBox->rect.width);
    newBox->rect.y = playerY - SCREEN_HEIGHT * 3;

    Box* iter = boxes;
    if(iter == NULL) {
        boxes = newBox;
    } else {
        while(iter->next != NULL) {
            iter = iter->next;
        }
        iter->next = newBox;
    }
};

void DrawBoxes() {
    Box* iter = boxes;
    if(iter == NULL) {
        return;
    } 

    while(iter->next != NULL) {
        DrawRectangleRec(iter->rect, iter->color);
        iter = iter->next;
    }
}

void MoveBoxes() {
    Box* iter = boxes;
    if(iter == NULL) {
        return;
    } 

    while(iter->next != NULL) {
        iter->rect.y += iter->speed;
        iter = iter->next;
    }
}

void BoxesCollide() {
    if(boxes == NULL) {
        return;
    } 

    Box* iter1 = boxes;
    Box* iter2 = boxes;

    while(iter1->next != NULL) {
        iter2 = boxes;
        while(iter2->next != NULL) {
            if(CheckCollisionRecs(iter1->rect, iter2->rect) && iter1 != iter2) {
                if (iter1->speed > iter2->speed)
                    iter1->speed = iter2->speed;
                else    
                    iter2->speed = iter1->speed;
            }
            iter2 = iter2->next;
        }
        iter1 = iter1->next;
    }
}

void BoxesCollideGround(Rectangle ground) {
    if(boxes == NULL) {
        return;
    } 

    Box* iter = boxes;
    while(iter->next != NULL) {
        if(CheckCollisionRecs(iter->rect, ground)) {
            iter->speed = 0.0;
        }
        iter = iter->next;
    }
}