#include "iGraphics.h"
#include <math.h>
#include <iostream>
#include "iSound.h"
#include <time.h> // Ensure this is included for rand()
#include <vector> // Using vector for dynamic enemy management, much easier than raw arrays
#include <chrono> // For high-resolution time

bool homepage = true, startPage = false, creditPage = false, instructionPage = false; // page con
double screen_height = 750, screen_width = 1850;
double px = 10, py = screen_height / 2; // Space ship position

// Player variables
double misile_speed = 50;
int my_health = 200; // Initial health to 200

// Game state variables
int score = 0;
char score_string[20], my_health_string[20];
int difficulty = 0;

const int TOTAL_ENEMIES_TO_DEFEAT = 30; // Total number of enemies before game won
int enemies_killed_total = 0; // Tracks total enemies killed across all types
int current_wave_enemies_spawned = 0; // Tracks enemies spawned in the current wave type (regular/boss wave)

bool game_over = false;
bool game_won = false;

// Global variables for managing end-game display
bool show_end_screen = false;
// Use std::chrono::steady_clock for reliable time tracking
std::chrono::steady_clock::time_point end_screen_start_time;
const long long END_SCREEN_DURATION_MS = 3000; // 3 seconds in milliseconds

// Health scaling parameters
const int BASE_REGULAR_ENEMY_HEALTH_HITS = 2; // Regular enemy takes 2 hits initially
const int BASE_BOSS_ENEMY_HEALTH_HITS = 5; // Boss takes 5 hits initially
const int HEALTH_INCREASE_HITS_PER_5_ENEMIES = 1; // 1 extra hit for regular enemies every 5 regular spawns
const int BOSS_HEALTH_INCREASE_HITS_PER_10_ENEMIES = 1; // 1 extra hit for boss every 10 total spawns

// Enemy structure
struct Enemy {
    double x, y;
    double speed;
    int health; // Represents number of hits required
    bool isBoss;
    bool active;
    double target_x_pos; // Where the enemy stops horizontally

    Enemy(double start_x, double start_y, double spd, int hp, bool bossType)
        : x(start_x), y(start_y), speed(spd), health(hp), isBoss(bossType), active(true) {
        target_x_pos = isBoss ? screen_width - 550 : screen_width - 450;
    }
};

// Use a vector to manage multiple enemies
std::vector<Enemy> activeEnemies;
int max_simultaneous_enemies = 2; // How many enemies can be on screen at once

// Boss Wave Management
int regular_enemies_killed_since_last_boss_wave = 0;
const int REGULAR_ENEMIES_BEFORE_BOSS_WAVE = 5;
const int INITIAL_REGULAR_ENEMIES_FOR_FIRST_BOSS = 4; // Number of regular enemies to kill before first boss can appear
bool in_boss_wave = false;
int boss_enemies_in_current_wave = 0; // How many bosses we've spawned in the current boss wave (should be 2)


struct Bullet
{
    double x, y;
};

struct EnemyBullet
{
    double x, y;
    double speed;
    bool isBossBullet; // Flag to indicate if it's a boss bullet
};

// Bullet arrays
Bullet bullets[50];
int bulletCount = 0;
EnemyBullet enemyBullets[100]; // Increased capacity for more enemy bullets
int enemyBulletCount = 0;

// Bullet speeds (for player bullets, enemy bullets will have their own speed member)
double playerBulletSpeed = 50;
double regularEnemyBulletSpeed = 20;
double bossEnemyBulletSpeed = 60;


// Image variables
Image planeImg, bulletImg, backgroundImg, instructionImg, creditsImg, enemy_shipsImg, finalBoss_shipImg;
int HomeMusic, PlayMusic; // Sound Storing Variable

bool isColliding(double x1, double y1, double w1, double h1, double x2, double y2, double w2, double h2)
{
    return !(x2 > x1 + w1 ||
             x2 + w2 < x1 ||
             y2 > y1 + h1 ||
             y2 + h2 < y1);
}

void enemyShoot()
{
    // Only shoot if game is active and not showing end screen
    if (!startPage || game_over || show_end_screen) return;

    for (size_t i = 0; i < activeEnemies.size(); ++i) {
        if (activeEnemies[i].active && activeEnemies[i].x < screen_width && activeEnemies[i].x > 0 && enemyBulletCount < 100) {
            enemyBullets[enemyBulletCount].x = activeEnemies[i].x;
            enemyBullets[enemyBulletCount].y = activeEnemies[i].y + (activeEnemies[i].isBoss ? 60 : 30);
            enemyBullets[enemyBulletCount].speed = activeEnemies[i].isBoss ? bossEnemyBulletSpeed : regularEnemyBulletSpeed;
            enemyBullets[enemyBulletCount].isBossBullet = activeEnemies[i].isBoss; // Set the flag here
            enemyBulletCount++;
        }
    }
}

void enemyBulletMovement()
{
    // Only move if game is active and not showing end screen
    if (!startPage || game_over || show_end_screen) return;

    for (int i = 0; i < enemyBulletCount; i++)
    {
        enemyBullets[i].x -= enemyBullets[i].speed; // Use bullet's individual speed
    }

    int validIndex = 0;
    for (int i = 0; i < enemyBulletCount; i++)
    {
        if (enemyBullets[i].x >= 0)
        {
            enemyBullets[validIndex++] = enemyBullets[i];
        }
    }
    enemyBulletCount = validIndex;
}


void missileMovement()
{
    // Only move if game is active and not showing end screen
    if (!startPage || game_over || show_end_screen) return;

    for (int i = 0; i < bulletCount; i++)
    {
        bullets[i].x += playerBulletSpeed;
    }

    int validIndex = 0;
    for (int i = 0; i < bulletCount; i++)
    {
        if (bullets[i].x <= screen_width)
        {
            bullets[validIndex++] = bullets[i];
        }
    }
    bulletCount = validIndex;
}

void shootBullet()
{
    // Only shoot if game is active and not showing end screen
    if (startPage && bulletCount < 50 && !game_over && !show_end_screen)
    {
        bullets[bulletCount].x = px + 90;
        bullets[bulletCount].y = py + 33;
        bulletCount++;
    }
}

void spawnEnemies()
{
    if (enemies_killed_total >= TOTAL_ENEMIES_TO_DEFEAT)
    {
        game_over = true;
        game_won = true;
        return; // No more spawning if game is won
    }

    // Only spawn if not showing end screen
    if (show_end_screen) return;

    activeEnemies.clear(); // Clear existing enemies to prepare for new spawns

    // Calculate dynamic health based on overall enemies killed for scaling
    int regular_enemy_health_hits = BASE_REGULAR_ENEMY_HEALTH_HITS + (enemies_killed_total / 5) * HEALTH_INCREASE_HITS_PER_5_ENEMIES;
    int boss_enemy_health_hits = BASE_BOSS_ENEMY_HEALTH_HITS + (enemies_killed_total / 10) * BOSS_HEALTH_INCREASE_HITS_PER_10_ENEMIES;

    // Boss wave logic
    // A boss wave can only start IF enough regular enemies have been killed overall
    if (regular_enemies_killed_since_last_boss_wave >= REGULAR_ENEMIES_BEFORE_BOSS_WAVE &&
        enemies_killed_total >= INITIAL_REGULAR_ENEMIES_FOR_FIRST_BOSS &&
        !in_boss_wave)
    {
        in_boss_wave = true;
        boss_enemies_in_current_wave = 0;
        std::cout << "Starting Boss Wave!" << std::endl;
    }

    if (in_boss_wave) {
        // Spawn 2 boss enemies
        if (boss_enemies_in_current_wave < 2) { // Ensure we only spawn 2 bosses for the wave
            for (int i = 0; i < 2; ++i) {
                activeEnemies.emplace_back(
                    screen_width + 10 + i * 150, // Stagger their starting X
                    rand() % (int)(screen_height - 150),
                    10, // Boss speed
                    boss_enemy_health_hits,
                    true // It's a boss
                );
                boss_enemies_in_current_wave++;
                current_wave_enemies_spawned++; // Count for overall defeat
            }
        }
    } else {
        // Spawn max_simultaneous_enemies (2) normal or random boss type enemies
        for (int i = 0; i < max_simultaneous_enemies; ++i) {
            // Boss type can only spawn randomly if initial regular enemies have been killed
            bool isBossType = (rand() % 100 < 25) && (enemies_killed_total >= INITIAL_REGULAR_ENEMIES_FOR_FIRST_BOSS);
            activeEnemies.emplace_back(
                screen_width + 10 + i * 100, // Stagger their starting X
                rand() % (int)(screen_height - (isBossType ? 150 : 100)),
                isBossType ? 10 : 6, // Speed
                isBossType ? boss_enemy_health_hits : regular_enemy_health_hits,
                isBossType
            );
            current_wave_enemies_spawned++; // Count for overall defeat
        }
    }
}

void enemy_movement()
{
    // Only move if game is active and not showing end screen
    if (!startPage || game_over || show_end_screen)
        return;

    for (size_t i = 0; i < activeEnemies.size(); ++i)
    {
        if (activeEnemies[i].active)
        {
            Enemy& enemy = activeEnemies[i]; // Use a reference for easier access

            // Move enemy horizontally into the screen
            if (enemy.x > enemy.target_x_pos)
            {
                enemy.x -= enemy.speed;
            }
            else
            {
                // Fix horizontal position, move only vertically
                enemy.x = enemy.target_x_pos;

                enemy.y += enemy.speed;

                double enemy_height = enemy.isBoss ? 150 : 100; // Approx height for boundary checks
                if (enemy.y > screen_height - enemy_height || enemy.y < 0)
                {
                    enemy.speed = -enemy.speed; // reverse vertical direction
                }
            }
        }
    }
}

// Function to reset all game state variables to their initial values
void resetGameState() {
    px = 10;
    py = screen_height / 2;
    my_health = 200;
    score = 0;
    enemies_killed_total = 0;
    current_wave_enemies_spawned = 0;
    game_over = false;
    game_won = false;
    bulletCount = 0;
    enemyBulletCount = 0;
    activeEnemies.clear();
    regular_enemies_killed_since_last_boss_wave = 0;
    in_boss_wave = false;
    boss_enemies_in_current_wave = 0;
    show_end_screen = false; // Ensure end screen flag is false
    iResumeTimer(0); // Resume all game timers
}


void startGame()
{
    // If game is over and we are showing the end screen, just return; iDraw will handle the display
    if (game_over && show_end_screen) {
        return;
    }
    // If game is just over (but not showing end screen yet), set up the end screen
    if (game_over && !show_end_screen) {
        show_end_screen = true;
        end_screen_start_time = std::chrono::steady_clock::now(); // Record current time
        iPauseTimer(0); // Pause all game timers immediately
        return; // Exit startGame to avoid drawing game elements
    }

    iShowLoadedImage(0, 0, &backgroundImg);
    iWrapImage(&backgroundImg, -10);
    iShowLoadedImage(px, py, &planeImg);

    // Draw active enemy ships
    for (size_t i = 0; i < activeEnemies.size(); ++i) {
        if (activeEnemies[i].active) {
            if (activeEnemies[i].isBoss)
                iShowLoadedImage(activeEnemies[i].x, activeEnemies[i].y, &finalBoss_shipImg);
            else
                iShowLoadedImage(activeEnemies[i].x, activeEnemies[i].y, &enemy_shipsImg);
        }
    }

    // Draw player bullets
    for (int i = 0; i < bulletCount; i++)
        iShowLoadedImage(bullets[i].x, bullets[i].y, &bulletImg);

    // Draw enemy bullets
    for (int i = 0; i < enemyBulletCount; i++)
        iShowLoadedImage(enemyBullets[i].x, enemyBullets[i].y, &bulletImg);

    // Check for enemy missile hitting player
    int validEnemyBulletIndex = 0;
    for (int i = 0; i < enemyBulletCount; i++)
    {
        if (isColliding(enemyBullets[i].x, enemyBullets[i].y, 20, 20, px, py, 90, 90))
        {
            my_health -= (enemyBullets[i].isBossBullet ? 10 : 5);
            enemyBullets[i].x = -100; // Remove bullet (move off-screen)
        }
        else if (enemyBullets[i].x >= 0) // Keep bullets that are still on screen and didn't hit
        {
            enemyBullets[validEnemyBulletIndex++] = enemyBullets[i];
        }
    }
    enemyBulletCount = validEnemyBulletIndex;


    if (my_health <= 0)
    {
        my_health = 0;
        game_over = true;
        game_won = false; // Player lost
        // Don't pause timer here, startGame will handle it upon next call
        return;
    }


    // Check if player bullet hits an active enemy
    int enemiesStillActiveInThisFrame = 0;
    for (size_t i = 0; i < activeEnemies.size(); ++i)
    {
        if (activeEnemies[i].active)
        {
            double enemy_width = activeEnemies[i].isBoss ? 120 : 100;
            double enemy_height = activeEnemies[i].isBoss ? 120 : 100;

            for (int j = 0; j < bulletCount; j++)
            {
                if (isColliding(bullets[j].x, bullets[j].y, 20, 20, activeEnemies[i].x, activeEnemies[i].y, enemy_width, enemy_height))
                {
                    bullets[j].x = screen_width + 100; // Remove player bullet

                    activeEnemies[i].health--; // Decrement enemy's hit points
                    score += 5; // Score for hitting

                    if (activeEnemies[i].health <= 0)
                    {
                        activeEnemies[i].active = false; // Mark enemy as inactive
                        enemies_killed_total++; // Increment total killed count
                        score += (activeEnemies[i].isBoss ? 500 : 100); // Bonus for killing

                        // Health bonus for killing enemies
                        if (activeEnemies[i].isBoss) {
                            my_health += 10; // Gain 10 health for killing boss
                        } else {
                            my_health += 5; // Gain 5 health for killing regular enemy
                            regular_enemies_killed_since_last_boss_wave++;
                        }
                    }
                }
            }
            if (activeEnemies[i].active) {
                enemiesStillActiveInThisFrame++;
            }
        }
    }

    // After checking all collisions and marking enemies inactive, clean up the vector
    // This is important to remove "dead" enemies, so new ones can be spawned
    std::vector<Enemy> nextActiveEnemies;
    for(size_t i = 0; i < activeEnemies.size(); ++i) {
        if(activeEnemies[i].active) {
            nextActiveEnemies.push_back(activeEnemies[i]);
        }
    }
    activeEnemies = nextActiveEnemies;

    // If no enemies are active and we are not in a boss wave or just finished one
    // AND total enemies killed is less than max
    if (activeEnemies.empty() && enemies_killed_total < TOTAL_ENEMIES_TO_DEFEAT) {
        if (in_boss_wave) { // If we just cleared a boss wave
            in_boss_wave = false;
            regular_enemies_killed_since_last_boss_wave = 0; // Reset regular enemy counter
        }
        spawnEnemies(); // Spawn next wave
    }
    else if (enemies_killed_total >= TOTAL_ENEMIES_TO_DEFEAT && activeEnemies.empty()) {
        game_over = true;
        game_won = true; // Player won!
        // Don't pause timer here, startGame will handle it upon next call
        return;
    }


    // HUD
    iSetColor(240, 240, 240);
    sprintf(my_health_string, "HEALTH: %d", my_health);
    sprintf(score_string, "SCORE: %d", score);
    iTextAdvanced(20, screen_height - 50, my_health_string, 0.15, 1); // health
    iTextAdvanced(20, screen_height - 100, score_string, 0.15, 1);    // score
    iTextAdvanced(550, 20, "Press p to pause, r to resume and space to shoot", 0.10, 1);
}

void showHomepage()
{
    iShowLoadedImage(0, 0, &backgroundImg);
    iSetColor(240, 240, 240);
    iTextAdvanced(480, 550, "SPACE SHOOTER", 0.5, 7.5); // space shooter text

    // Overlay text on buttons for clarity
    iSetColor(0, 0, 0); // Black text

    // Draw text for buttons, THEN draw the button images over them
    // The previous issue was the global text being drawn *before* the button images were drawn.

    // PLAY button
    iTextAdvanced(screen_width / 2 - 190, screen_height - 275, "PLAY", 0.2, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 300, "SelectedAssets/start.png");

    // INSTRUCTIONS button
    iTextAdvanced(screen_width / 2 - 225, screen_height - 345, "INSTRUCTIONS", 0.15, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 370, "SelectedAssets/start.png");

    // CREDITS button
    iTextAdvanced(screen_width / 2 - 190, screen_height - 415, "CREDITS", 0.15, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 440, "SelectedAssets/start.png");

    // QUIT button
    iTextAdvanced(screen_width / 2 - 190, screen_height - 485, "QUIT", 0.15, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 510, "SelectedAssets/Quit.png");
}

void showInstructions()
{
    iShowLoadedImage(0, 0, &instructionImg);
    iSetColor(255, 255, 255); // White text for BACK
    iTextAdvanced(60, 650, "BACK", 0.15, 1);
}

void showCredits()
{
    iShowLoadedImage(0, 0, &creditsImg);
    iSetColor(255, 255, 255); // White text for BACK
    iTextAdvanced(60, 620, "BACK", 0.15, 1);
}

void soundWorks()
{
    if (startPage == true)
    {
        iPauseSound(HomeMusic);
        PlayMusic = iPlaySound("SelectedAssets/playMuic.wav", true, 100);
    }
    else
    {
        iPauseSound(PlayMusic);
        HomeMusic = iPlaySound("SelectedAssets/Menu.wav", true, 100);
    }
}

void iDraw()
{
    iClear();
    if (homepage == true)
    {
        showHomepage();
    }
    else if (startPage == true)
    {
        // If game is over and we need to show the end screen
        if (game_over && show_end_screen) {
            iClear(); // Clear the screen completely
            iSetColor(255, 0, 0); // Red for Game Over, Green for You Won
            if (game_won)
            {
                iSetColor(0, 255, 0); // Green for "You Won"
                iTextAdvanced(screen_width / 2 - 200, screen_height / 2, "🎉 YOU WON!", 0.3, 2);
            }
            else
            {
                iSetColor(255, 0, 0); // Red for "Game Over"
                iTextAdvanced(screen_width / 2 - 200, screen_height / 2, "💀 GAME OVER!", 0.3, 2);
            }

            // Check if 3 seconds have passed
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - end_screen_start_time).count();

            if (duration >= END_SCREEN_DURATION_MS) {
                startPage = false; // Turn off game page
                homepage = true; // Go to home page
                resetGameState(); // Reset all game-related variables
                soundWorks(); // Switch music back to menu
            }
        } else {
            // Normal game drawing if not in an end-screen state
            startGame();
        }
    }
    else if (creditPage == true)
    {
        showCredits();
    }
    else if (instructionPage == true)
    {
        showInstructions();
    }
}

void iMouseMove(int mx, int my)
{
    // Unused for now
}

void iMouseDrag(int mx, int my)
{
    // Unused for now
}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        std::cout << mx << " " << my << std::endl; // For debugging click coordinates

        if (homepage == true)
        {
            if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                my >= (screen_height - 300) && my <= (screen_height - 300) + 70) // PLAY button area
            {
                homepage = false;
                startPage = true;
                resetGameState(); // Reset game state fully on new game start
                spawnEnemies(); // Spawn the first wave of enemies
                soundWorks();
            }
            else if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                     my >= (screen_height - 370) && my <= (screen_height - 370) + 70) // INSTRUCTION button area
            {
                homepage = false;
                instructionPage = true;
            }
            else if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                     my >= (screen_height - 440) && my <= (screen_height - 440) + 70) // CREDIT button area
            {
                homepage = false;
                creditPage = true;
            }
            else if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                     my >= (screen_height - 510) && my <= (screen_height - 510) + 70) // QUIT button area
            {
                exit(0);
            }
        }
        // Back buttons for instruction and credit pages
        if (instructionPage == true && mx >= 49 && mx <= 176 && my >= 627 && my <= 725) // Approx coordinates for back on instructions
        {
            instructionPage = false;
            homepage = true;
        }
        if (creditPage == true && mx >= 46 && mx <= 152 && my >= 595 && my <= 682) // Approx coordinates for back on credits
        {
            creditPage = false;
            homepage = true;
        }
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // Unused for now
    }
}

void iMouseWheel(int dir, int mx, int my)
{
    // Unused for now
}

void iKeyboard(unsigned char key)
{
    if (key == 'p')
    {
        if (startPage && !game_over && !show_end_screen) { // Only pause if game is active
            iPauseTimer(0);
        }
    }
    else if (key == 'r')
    {
        if (startPage && !game_over && !show_end_screen) { // Only resume if game is active
            iResumeTimer(0);
        }
    }
    else if (key == 'q')
    {
        exit(0);
    }
    else if (key == 'b') // 'b' for back to homepage
    {
        // This 'b' key handling is now mostly redundant for game over/won,
        // but keeps functionality for other pages
        if (instructionPage == true)
        {
            instructionPage = false;
            homepage = true;
        }
        else if (creditPage == true)
        {
            creditPage = false;
            homepage = true;
        }
        else if (startPage == true) // If in game, go back to homepage
        {
            startPage = false;
            homepage = true;
            soundWorks(); // Switch back to menu music
            resetGameState(); // Reset all game-related variables
        }
    }
    else if (key == ' ') // Space to shoot
    {
        shootBullet();
    }
}

/*
function iSpecialKeyboard() is called whenver user hits special keys likefunction
keys, home, end, pg up, pg down, arraows etc. you have to use
appropriate constants to detect them. A list is:
GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11,
GLUT_KEY_F12, GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN,
GLUT_KEY_PAGE_UP, GLUT_KEY_PAGE_DOWN, GLUT_KEY_HOME, GLUT_KEY_END,
GLUT_KEY_INSERT */
void iSpecialKeyboard(unsigned char key)
{
    // Only allow movement if game is active and not showing end screen
    if (startPage && !game_over && !show_end_screen) {
        if (key == GLUT_KEY_RIGHT && px <= screen_width - 100)
        {
            px += 15;
        }
        if (key == GLUT_KEY_LEFT && px >= 0)
        {
            px -= 15;
        }
        if (key == GLUT_KEY_UP && py <= screen_height - 100)
        {
            py += 15;
        }
        if (key == GLUT_KEY_DOWN && py >= 0)
        {
            py -= 15;
        }
    }
}

void loadResources()
{
    iLoadImage(&backgroundImg, "SelectedAssets/background.png"); // background
    iResizeImage(&backgroundImg, screen_width, screen_height);
    iLoadImage(&planeImg, "SelectedAssets/MainSpaceShip.png"); // Main SpaceShip
    iResizeImage(&planeImg, 90, 90);
    iLoadImage(&instructionImg, "SelectedAssets/instructionsFinal.png"); // instructino page
    iResizeImage(&instructionImg, screen_width - 350, screen_height);
    iLoadImage(&creditsImg, "SelectedAssets/creditsfinal.png"); // credits page
    iResizeImage(&creditsImg, screen_width - 350, screen_height);
    iLoadImage(&enemy_shipsImg, "SelectedAssets/Ship3.png"); // enemy ship
    iResizeImage(&enemy_shipsImg, 100, 100);
    iLoadImage(&finalBoss_shipImg, "SelectedAssets/FinalBoss.png"); // final boss
    iResizeImage(&finalBoss_shipImg, 120, 120);
    iLoadImage(&bulletImg, "SelectedAssets/bullet.png"); // bullet
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    iInitializeSound();
    PlayMusic = iPlaySound("SelectedAssets/playMuic.wav", true, 100);
    HomeMusic = iPlaySound("SelectedAssets/Menu.wav", true, 100);
    iPauseSound(PlayMusic);

    // Game timers
    iSetTimer(50, missileMovement);
    iSetTimer(1000, enemyShoot);        // Enemy (regular or boss) shoots
    iSetTimer(50, enemyBulletMovement); // Enemy (regular or boss) bullets move
    iSetTimer(50, enemy_movement);      // Enemy (regular or boss) movement

    // Initialize random seed
    srand(time(0));

    iInitialize(screen_width, screen_height, "Space Shooter");
    return 0;
}