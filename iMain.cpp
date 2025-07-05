#include "iGraphics.h"
#include <math.h>
#include <iostream>
#include "iSound.h"
#include <time.h>
#include <vector>
#include <chrono>

bool homepage = true, startPage = false, creditPage = false, instructionPage = false;
double screen_height = 750, screen_width = 1850;
double px = 10, py = screen_height / 2;

double misile_speed = 50;
int my_health = 200;

int score = 0;
char score_string[20], my_health_string[20];
int difficulty = 0;

const int TOTAL_ENEMIES_TO_DEFEAT = 30;
int enemies_killed_total = 0;
int current_wave_enemies_spawned = 0;

bool game_over = false;
bool game_won = false;

bool show_end_screen = false;
std::chrono::steady_clock::time_point end_screen_start_time;
const long long END_SCREEN_DURATION_MS = 3000;

const int BASE_REGULAR_ENEMY_HEALTH_HITS = 2;
const int BASE_BOSS_ENEMY_HEALTH_HITS = 5;
const int HEALTH_INCREASE_HITS_PER_5_ENEMIES = 1;
const int BOSS_HEALTH_INCREASE_HITS_PER_10_ENEMIES = 1;

struct Enemy {
    double x, y;
    double speed;
    int health;
    bool isBoss;
    bool active;
    double target_x_pos;

    Enemy(double start_x, double start_y, double spd, int hp, bool bossType)
        : x(start_x), y(start_y), speed(spd), health(hp), isBoss(bossType), active(true) {
        target_x_pos = isBoss ? screen_width - 550 : screen_width - 450;
    }
};

std::vector<Enemy> activeEnemies;
int max_simultaneous_enemies = 2;

int regular_enemies_killed_since_last_boss_wave = 0;
const int REGULAR_ENEMIES_BEFORE_BOSS_WAVE = 5;
const int INITIAL_REGULAR_ENEMIES_FOR_FIRST_BOSS = 4;
bool in_boss_wave = false;
int boss_enemies_in_current_wave = 0;

struct Bullet
{
    double x, y;
};

struct EnemyBullet
{
    double x, y;
    double speed;
    bool isBossBullet;
};

Bullet bullets[50];
int bulletCount = 0;
EnemyBullet enemyBullets[100];
int enemyBulletCount = 0;

double playerBulletSpeed = 50;
double regularEnemyBulletSpeed = 20;
double bossEnemyBulletSpeed = 60;

Image planeImg, bulletImg, backgroundImg, instructionImg, creditsImg, enemy_shipsImg, finalBoss_shipImg;
int HomeMusic, PlayMusic;

bool isColliding(double x1, double y1, double w1, double h1, double x2, double y2, double w2, double h2)
{
    return !(x2 > x1 + w1 ||
             x2 + w2 < x1 ||
             y2 > y1 + h1 ||
             y2 + h2 < y1);
}

void enemyShoot()
{
    if (!startPage || game_over || show_end_screen) return;

    for (size_t i = 0; i < activeEnemies.size(); ++i) {
        if (activeEnemies[i].active && activeEnemies[i].x < screen_width && activeEnemies[i].x > 0 && enemyBulletCount < 100) {
            enemyBullets[enemyBulletCount].x = activeEnemies[i].x;
            enemyBullets[enemyBulletCount].y = activeEnemies[i].y + (activeEnemies[i].isBoss ? 60 : 30);
            enemyBullets[enemyBulletCount].speed = activeEnemies[i].isBoss ? bossEnemyBulletSpeed : regularEnemyBulletSpeed;
            enemyBullets[enemyBulletCount].isBossBullet = activeEnemies[i].isBoss;
            enemyBulletCount++;
        }
    }
}

void enemyBulletMovement()
{
    if (!startPage || game_over || show_end_screen) return;

    for (int i = 0; i < enemyBulletCount; i++)
    {
        enemyBullets[i].x -= enemyBullets[i].speed;
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
        return;
    }

    if (show_end_screen) return;

    activeEnemies.clear();

    int regular_enemy_health_hits = BASE_REGULAR_ENEMY_HEALTH_HITS + (enemies_killed_total / 5) * HEALTH_INCREASE_HITS_PER_5_ENEMIES;
    int boss_enemy_health_hits = BASE_BOSS_ENEMY_HEALTH_HITS + (enemies_killed_total / 10) * BOSS_HEALTH_INCREASE_HITS_PER_10_ENEMIES;

    if (regular_enemies_killed_since_last_boss_wave >= REGULAR_ENEMIES_BEFORE_BOSS_WAVE &&
        enemies_killed_total >= INITIAL_REGULAR_ENEMIES_FOR_FIRST_BOSS &&
        !in_boss_wave)
    {
        in_boss_wave = true;
        boss_enemies_in_current_wave = 0;
        std::cout << "Starting Boss Wave!" << std::endl;
    }

    if (in_boss_wave) {
        if (boss_enemies_in_current_wave < 2) {
            for (int i = 0; i < 2; ++i) {
                activeEnemies.emplace_back(
                    screen_width + 10 + i * 150,
                    rand() % (int)(screen_height - 150),
                    10,
                    boss_enemy_health_hits,
                    true
                );
                boss_enemies_in_current_wave++;
                current_wave_enemies_spawned++;
            }
        }
    } else {
        for (int i = 0; i < max_simultaneous_enemies; ++i) {
            bool isBossType = (rand() % 100 < 25) && (enemies_killed_total >= INITIAL_REGULAR_ENEMIES_FOR_FIRST_BOSS);
            activeEnemies.emplace_back(
                screen_width + 10 + i * 100,
                rand() % (int)(screen_height - (isBossType ? 150 : 100)),
                isBossType ? 10 : 6,
                isBossType ? boss_enemy_health_hits : regular_enemy_health_hits,
                isBossType
            );
            current_wave_enemies_spawned++;
        }
    }
}

void enemy_movement()
{
    if (!startPage || game_over || show_end_screen)
        return;

    for (size_t i = 0; i < activeEnemies.size(); ++i)
    {
        if (activeEnemies[i].active)
        {
            Enemy& enemy = activeEnemies[i];

            if (enemy.x > enemy.target_x_pos)
            {
                enemy.x -= enemy.speed;
            }
            else
            {
                enemy.x = enemy.target_x_pos;

                enemy.y += enemy.speed;

                double enemy_height = enemy.isBoss ? 150 : 100;
                if (enemy.y > screen_height - enemy_height || enemy.y < 0)
                {
                    enemy.speed = -enemy.speed;
                }
            }
        }
    }
}

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
    show_end_screen = false;
    iResumeTimer(0);
}

void startGame()
{
    if (game_over && show_end_screen) {
        return;
    }
    if (game_over && !show_end_screen) {
        show_end_screen = true;
        end_screen_start_time = std::chrono::steady_clock::now();
        iPauseTimer(0);
        return;
    }

    iShowLoadedImage(0, 0, &backgroundImg);
    iWrapImage(&backgroundImg, -10);
    iShowLoadedImage(px, py, &planeImg);

    for (size_t i = 0; i < activeEnemies.size(); ++i) {
        if (activeEnemies[i].active) {
            if (activeEnemies[i].isBoss)
                iShowLoadedImage(activeEnemies[i].x, activeEnemies[i].y, &finalBoss_shipImg);
            else
                iShowLoadedImage(activeEnemies[i].x, activeEnemies[i].y, &enemy_shipsImg);
        }
    }

    for (int i = 0; i < bulletCount; i++)
        iShowLoadedImage(bullets[i].x, bullets[i].y, &bulletImg);

    for (int i = 0; i < enemyBulletCount; i++)
        iShowLoadedImage(enemyBullets[i].x, enemyBullets[i].y, &bulletImg);

    int validEnemyBulletIndex = 0;
    for (int i = 0; i < enemyBulletCount; i++)
    {
        if (isColliding(enemyBullets[i].x, enemyBullets[i].y, 20, 20, px, py, 90, 90))
        {
            my_health -= (enemyBullets[i].isBossBullet ? 10 : 5);
            enemyBullets[i].x = -100;
        }
        else if (enemyBullets[i].x >= 0)
        {
            enemyBullets[validEnemyBulletIndex++] = enemyBullets[i];
        }
    }
    enemyBulletCount = validEnemyBulletIndex;

    if (my_health <= 0)
    {
        my_health = 0;
        game_over = true;
        game_won = false;
        return;
    }

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
                    bullets[j].x = screen_width + 100;

                    activeEnemies[i].health--;
                    score += 5;

                    if (activeEnemies[i].health <= 0)
                    {
                        activeEnemies[i].active = false;
                        enemies_killed_total++;
                        score += (activeEnemies[i].isBoss ? 500 : 100);

                        if (activeEnemies[i].isBoss) {
                            my_health += 10;
                        } else {
                            my_health += 5;
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

    std::vector<Enemy> nextActiveEnemies;
    for(size_t i = 0; i < activeEnemies.size(); ++i) {
        if(activeEnemies[i].active) {
            nextActiveEnemies.push_back(activeEnemies[i]);
        }
    }
    activeEnemies = nextActiveEnemies;

    if (activeEnemies.empty() && enemies_killed_total < TOTAL_ENEMIES_TO_DEFEAT) {
        if (in_boss_wave) {
            in_boss_wave = false;
            regular_enemies_killed_since_last_boss_wave = 0;
        }
        spawnEnemies();
    }
    else if (enemies_killed_total >= TOTAL_ENEMIES_TO_DEFEAT && activeEnemies.empty()) {
        game_over = true;
        game_won = true;
        return;
    }

    iSetColor(240, 240, 240);
    sprintf(my_health_string, "HEALTH: %d", my_health);
    sprintf(score_string, "SCORE: %d", score);
    iTextAdvanced(20, screen_height - 50, my_health_string, 0.15, 1);
    iTextAdvanced(20, screen_height - 100, score_string, 0.15, 1);
    iTextAdvanced(550, 20, "Press p to pause, r to resume and space to shoot", 0.10, 1);
}

void showHomepage()
{
    iShowLoadedImage(0, 0, &backgroundImg);
    iSetColor(240, 240, 240);
    iTextAdvanced(480, 550, "SPACE SHOOTER", 0.5, 7.5);

    iSetColor(0, 0, 0);

    iTextAdvanced(screen_width / 2 - 190, screen_height - 275, "PLAY", 0.2, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 300, "SelectedAssets/start.png");

    iTextAdvanced(screen_width / 2 - 225, screen_height - 345, "INSTRUCTIONS", 0.15, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 370, "SelectedAssets/start.png");

    iTextAdvanced(screen_width / 2 - 190, screen_height - 415, "CREDITS", 0.15, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 440, "SelectedAssets/start.png");

    iTextAdvanced(screen_width / 2 - 190, screen_height - 485, "QUIT", 0.15, 1);
    iShowImage(screen_width / 2 - 250, screen_height - 510, "SelectedAssets/Quit.png");
}

void showInstructions()
{
    iShowLoadedImage(0, 0, &instructionImg);
    iSetColor(255, 255, 255);
    iTextAdvanced(60, 650, "BACK", 0.15, 1);
}

void showCredits()
{
    iShowLoadedImage(0, 0, &creditsImg);
    iSetColor(255, 255, 255);
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
        if (game_over && show_end_screen) {
            iClear();
            iSetColor(255, 0, 0);
            if (game_won)
            {
                iSetColor(0, 255, 0);
                iTextAdvanced(screen_width / 2 - 200, screen_height / 2, "🎉 YOU WON!", 0.3, 2);
            }
            else
            {
                iSetColor(255, 0, 0);
                iTextAdvanced(screen_width / 2 - 200, screen_height / 2, "💀 GAME OVER!", 0.3, 2);
            }

            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - end_screen_start_time).count();

            if (duration >= END_SCREEN_DURATION_MS) {
                startPage = false;
                homepage = true;
                resetGameState();
                soundWorks();
            }
        } else {
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
}

void iMouseDrag(int mx, int my)
{
}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        std::cout << mx << " " << my << std::endl;

        if (homepage == true)
        {
            if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                my >= (screen_height - 300) && my <= (screen_height - 300) + 70)
            {
                homepage = false;
                startPage = true;
                resetGameState();
                spawnEnemies();
                soundWorks();
            }
            else if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                     my >= (screen_height - 370) && my <= (screen_height - 370) + 70)
            {
                homepage = false;
                instructionPage = true;
            }
            else if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                     my >= (screen_height - 440) && my <= (screen_height - 440) + 70)
            {
                homepage = false;
                creditPage = true;
            }
            else if (mx >= (screen_width / 2 - 250) && mx <= (screen_width / 2 - 250) + 250 &&
                     my >= (screen_height - 510) && my <= (screen_height - 510) + 70)
            {
                exit(0);
            }
        }
        if (instructionPage == true && mx >= 49 && mx <= 176 && my >= 627 && my <= 725)
        {
            instructionPage = false;
            homepage = true;
        }
        if (creditPage == true && mx >= 46 && mx <= 152 && my >= 595 && my <= 682)
        {
            creditPage = false;
            homepage = true;
        }
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
    }
}

void iMouseWheel(int dir, int mx, int my)
{
}

void iKeyboard(unsigned char key)
{
    if (key == 'p')
    {
        if (startPage && !game_over && !show_end_screen) {
            iPauseTimer(0);
        }
    }
    else if (key == 'r')
    {
        if (startPage && !game_over && !show_end_screen) {
            iResumeTimer(0);
        }
    }
    else if (key == 'q')
    {
        exit(0);
    }
    else if (key == 'b')
    {
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
        else if (startPage == true)
        {
            startPage = false;
            homepage = true;
            soundWorks();
            resetGameState();
        }
    }
    else if (key == ' ')
    {
        shootBullet();
    }
}

void iSpecialKeyboard(unsigned char key)
{
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
    iLoadImage(&backgroundImg, "SelectedAssets/background.png");
    iResizeImage(&backgroundImg, screen_width, screen_height);
    iLoadImage(&planeImg, "SelectedAssets/MainSpaceShip.png");
    iResizeImage(&planeImg, 90, 90);
    iLoadImage(&instructionImg, "SelectedAssets/instructionsFinal.png");
    iResizeImage(&instructionImg, screen_width - 350, screen_height);
    iLoadImage(&creditsImg, "SelectedAssets/creditsfinal.png");
    iResizeImage(&creditsImg, screen_width - 350, screen_height);
    iLoadImage(&enemy_shipsImg, "SelectedAssets/Ship3.png");
    iResizeImage(&enemy_shipsImg, 100, 100);
    iLoadImage(&finalBoss_shipImg, "SelectedAssets/FinalBoss.png");
    iResizeImage(&finalBoss_shipImg, 120, 120);
    iLoadImage(&bulletImg, "SelectedAssets/bullet.png");
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    iInitializeSound();
    PlayMusic = iPlaySound("SelectedAssets/playMuic.wav", true, 100);
    HomeMusic = iPlaySound("SelectedAssets/Menu.wav", true, 100);
    iPauseSound(PlayMusic);

    iSetTimer(50, missileMovement);
    iSetTimer(1000, enemyShoot);
    iSetTimer(50, enemyBulletMovement);
    iSetTimer(50, enemy_movement);

    srand(time(0));

    iInitialize(screen_width, screen_height, "Space Shooter");
    return 0;
}