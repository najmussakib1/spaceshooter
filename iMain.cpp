#include "iGraphics.h"
#include <math.h>
#include "iSound.h"
#include <time.h>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cctype>
#include "iFont.h"

struct PlayerScore {
    char name[51];
    int score;
};

bool isnameinputactive = false;
char current_player_name[51] = "";
int name_input_index = 0;

bool ishomepageactive = true;
bool isgamerunning = false;
bool iscreditpageactive = false;
bool isinstructionpageactive = false;
bool isleaderboardpageactive=false;
bool isyouwonpageactive = false; // New state for "You Won" page
bool isgameoverpageactive = false; // New state for "Game Over" page

bool isgamepaused = false;

double screenheight = 750;
double screenwidth = 1850;
double playerx = 10;
double playery = screenheight / 2;

double playermissilespeed = 50;
int playerhealth = 200;

const int max_player_health = 400;

int gamescore = 0;
char scoredisplaystring[20];

const int total_enemies_required_to_win = 50;
const int total_bosses_in_game = 25;
int totalenemieskilled = 0;
int totaleniemiesspawned = 0;
int totalbossesspawned = 0;

bool isgameover = false;
bool hasgamebeenwon = false;

bool showendgamescreen = false; // This flag can be reused for timing, but separate state booleans for win/lose screens are better for clarity
int endgametimercounter = 0;
const int you_won_screen_duration_ticks = 40; // 2 seconds at 50ms/tick (40 * 50ms = 2000ms)

// Added for score saving
bool isscoresaved = false;

// Updated enemy health values (twice the previous)
const int regular_enemy_base_health_hits = 4;
const int boss_enemy_base_health_hits = 10;
const int regular_enemy_health_increase_per_5_kills = 2; // Twice the previous
const int boss_health_increase_per_10_kills = 2; // Twice the previous

#define MAX_ENEMIES 100
struct Enemy {
    double positionx, positiony;
    double movementspeed;
    int currenthealth;
    bool isbossenemy;
    bool isactive;
    double targethorizontalposition;
};
struct Explosion {
    double x, y;
    bool isActive;
    int startTime;
};
#define MAX_EXPLOSIONS 50
Explosion activeExplosions[MAX_EXPLOSIONS];
Image explosionSprites[6];


Enemy activeenemies[MAX_ENEMIES];
int currentactiveenemycount = 0;

int maxsimultaneousenemies = 2;

int regularenemieskilledsincelastbosswave = 0;
const int regular_enemies_before_boss_wave = 5;
const int initial_regular_enemies_for_first_boss = 5;
bool isinbosswave = false;
int currentbossenemiesinwave = 0;

struct FoodItem {
    double positionx;
    double positiony;
    bool isactive;
    bool issuperpOwer;
};

#define MAX_FOOD_ITEMS_ON_SCREEN 5
FoodItem fooditemsonScreen[MAX_FOOD_ITEMS_ON_SCREEN];
int currentfooditemcount = 0;
int enemieskilledsincelastfooddrop = 0;
const int enemies_killed_for_food_drop = 20;

bool issuperpOweractive = false;
int superpowerrstarttime = 0;
const int superpower_duration_ticks = 100;
const int enemies_for_superpower_drop = 25;
int enemieskilledsincelastsuperpower = 0;

int superpowerbullettimer = 0;
const int superpower_bullet_interval = 2;

int gametickcount = 0;

struct Bullet {
    double positionx, positiony;
};

struct EnemyBullet {
    double positionx, positiony;
    double speed;
    bool firedbyboss;
};

Bullet playerbullets[50];
int currentplayerbulletcount = 0;
EnemyBullet enemybullets[200];
int currentenemybulletcount = 0;

double playerbulletmovementspeed = 50;
double regularenemybulletmovementspeed = 30;
double bossenemybulletmovementspeed = 70;

Image playershipimage, playerbulletimage, backgroundimage, instructionpageimage, creditspageimage, regularenemyshipimage, finalbossshipimage, fooditemimage, enemybulletimage, superpowerfoodimage;
int homescreenmusic, gameplaymusic;

int explosionsoundeffect;

const double player_hitbox_offset_x = 15;
const double player_hitbox_offset_y = 15;
const double player_hitbox_width = 90 - (2 * player_hitbox_offset_x);
const double player_hitbox_height = 90 - (2 * player_hitbox_offset_y);

const double regular_enemy_hitbox_offset_x = 20;
const double regular_enemy_hitbox_offset_y = 20;
const double regular_enemy_hitbox_width = 100 - (2 * regular_enemy_hitbox_offset_x);
const double regular_enemy_hitbox_height = 100 - (2 * regular_enemy_hitbox_offset_y);

const double boss_enemy_hitbox_offset_x = 25;
const double boss_enemy_hitbox_offset_y = 25;
const double boss_enemy_hitbox_width = 120 - (2 * boss_enemy_hitbox_offset_x);
const double boss_enemy_hitbox_height = 120 - (2 * boss_enemy_hitbox_offset_y);

const double bullet_width = 20;
const double bullet_height = 20;

const double food_width = 50;
const double food_height = 50;

bool compareScores(const PlayerScore& a, const PlayerScore& b) {
    return a.score > b.score;
}

void saveScoreToFile() {
    FILE *fp = fopen("leaderboard.txt", "a");
    if (fp != NULL) {
        fprintf(fp, "%s %d\n", current_player_name, gamescore);
        fclose(fp);
    }
}

void loadScoresFromFile(PlayerScore scores[], int& count) {
    FILE *fp = fopen("leaderboard.txt", "r");
    count = 0;
    if (fp != NULL) {
        while (fscanf(fp, "%s %d", scores[count].name, &scores[count].score) != EOF && count < 100) {
            count++;
        }
        fclose(fp);
    }
    std::sort(scores, scores + count, compareScores);
}

void showNameInputScreen() {
    iClear();
    iShowLoadedImage(0, 0, &backgroundimage);
    iSetColor(255, 255, 0);

    // This screen is now only for pre-game name input
    iTextAdvanced(screenwidth / 2 - 350, screenheight / 2 + 150, "Enter Your Name to Start Game:", 0.3, 2);

    iSetColor(255, 255, 255);
    iTextAdvanced(screenwidth / 2 - 250, screenheight / 2 + 50, "Enter Your Name:", 0.2, 1);
    iFilledRectangle(screenwidth / 2 - 250, screenheight / 2 - 20, 500, 50);
    iSetColor(0, 0, 0);
    iTextAdvanced(screenwidth / 2 - 240, screenheight / 2 - 5, current_player_name, 0.2, 1);
    iSetColor(255, 255, 255);
    iTextAdvanced(screenwidth / 2 - 250, screenheight / 2 - 100, "Press 'Enter' to confirm and start game.", 0.15, 1);
}

bool checkCollision(double x1, double y1, double w1, double h1, double x2, double y2, double w2, double h2)
{
    return !(x2 > x1 + w1 ||
             x2 + w2 < x1 ||
             y2 > y1 + h1 ||
             y2 + h2 < y1);
}

void fireenemybullets()
{
    if (!isgamerunning || isgameover || isgamepaused) return;

    for (int i = 0; i < currentactiveenemycount; ++i) {
        if (activeenemies[i].isactive && activeenemies[i].positionx < screenwidth && activeenemies[i].positionx > 0 && currentenemybulletcount < 200) {
            enemybullets[currentenemybulletcount].positionx = activeenemies[i].positionx;
            enemybullets[currentenemybulletcount].positiony = activeenemies[i].positiony + (activeenemies[i].isbossenemy ? 60 : 30);
            enemybullets[currentenemybulletcount].speed = activeenemies[i].isbossenemy ? bossenemybulletmovementspeed : regularenemybulletmovementspeed;
            enemybullets[currentenemybulletcount].firedbyboss = activeenemies[i].isbossenemy;
            currentenemybulletcount++;
        }
    }
}

void moveenemybullets()
{
    if (!isgamerunning || isgameover || isgamepaused) return;

    int validbulletindex = 0;
    for (int i = 0; i < currentenemybulletcount; i++)
    {
        enemybullets[i].positionx -= enemybullets[i].speed;
        if (enemybullets[i].positionx >= 0)
        {
            enemybullets[validbulletindex++] = enemybullets[i];
        }
    }
    currentenemybulletcount = validbulletindex;
}

void moveplayermissiles()
{
    if (!isgamerunning || isgameover || isgamepaused) return;

    int validbulletindex = 0;
    for (int i = 0; i < currentplayerbulletcount; i++)
    {
        playerbullets[i].positionx += playermissilespeed;
        if (playerbullets[i].positionx <= screenwidth)
        {
            playerbullets[validbulletindex++] = playerbullets[i];
        }
    }
    currentplayerbulletcount = validbulletindex;
}

void playerfiresbullet()
{
    if (isgamerunning && currentplayerbulletcount < 50 && !isgameover && !isgamepaused)
    {
        playerbullets[currentplayerbulletcount].positionx = playerx + 90;
        playerbullets[currentplayerbulletcount].positiony = playery + 33;
        currentplayerbulletcount++;
    }
}

void spawnnewenemies()
{
    if (totaleniemiesspawned >= total_enemies_required_to_win) return;
    if (showendgamescreen || isgamepaused || isgameover) return;

    if (totaleniemiesspawned < 10) {
        maxsimultaneousenemies = 2;
    } else {
        maxsimultaneousenemies = 3;
    }

    int activeenemycountnow = 0;
    for(int i = 0; i < currentactiveenemycount; ++i) {
        if(activeenemies[i].isactive) {
            activeenemycountnow++;
        }
    }

    int enemiestospawninthiscall = maxsimultaneousenemies - activeenemycountnow;
    if (enemiestospawninthiscall <= 0) return;

    if (totaleniemiesspawned + enemiestospawninthiscall > total_enemies_required_to_win) {
        enemiestospawninthiscall = total_enemies_required_to_win - totaleniemiesspawned;
    }
    if (enemiestospawninthiscall <= 0) return;

    if (currentactiveenemycount + enemiestospawninthiscall > MAX_ENEMIES) {
        enemiestospawninthiscall = MAX_ENEMIES - currentactiveenemycount;
        if (enemiestospawninthiscall <= 0) return;
    }

    for (int i = 0; i < enemiestospawninthiscall; ++i) {
        bool shouldspawnboss = false;

        if (totaleniemiesspawned < initial_regular_enemies_for_first_boss) {
            shouldspawnboss = false;
        } else {
            if (totalbossesspawned < total_bosses_in_game && totaleniemiesspawned < total_enemies_required_to_win) {
                int remainingenemies = total_enemies_required_to_win - totaleniemiesspawned;
                int remainingbosses = total_bosses_in_game - totalbossesspawned;
                int remainingregular = remainingenemies - remainingbosses;

                if (remainingregular <= 0 && remainingbosses > 0) {shouldspawnboss = true;}
                else if (remainingbosses <= 0) {shouldspawnboss = false;}
                else {
                    double bossprobability = (double)rand() / RAND_MAX;
                    if (bossprobability < (double)remainingbosses / remainingenemies) {
                        shouldspawnboss = true;
                    } else {
                        shouldspawnboss = false;
                    }
                }
            } else {
                shouldspawnboss = false;
            }
        }
        if (shouldspawnboss && totalbossesspawned >= total_bosses_in_game) {
            shouldspawnboss = false;
        }

        int enemyhealth = shouldspawnboss ? (boss_enemy_base_health_hits + (totalenemieskilled / 10) * boss_health_increase_per_10_kills) :
                                          (regular_enemy_base_health_hits + (totalenemieskilled / 5) * regular_enemy_health_increase_per_5_kills);
        double enemyspeed = shouldspawnboss ? 10 : 6;
        double enemyheight = shouldspawnboss ? 120 : 100;

        double spawnyposition;
        bool positionisclear = false;
        int attempts = 0;
        while (!positionisclear && attempts < 20) {
            spawnyposition = rand() % (int)(screenheight - enemyheight - 50) + 50;
            positionisclear = true;
            for (int k = 0; k < currentactiveenemycount; ++k) {
                if (activeenemies[k].isactive && abs(activeenemies[k].positiony - spawnyposition) < (enemyheight / 2 + 50)) {
                    positionisclear = false;
                    break;
                }
            }
            attempts++;
        }

        activeenemies[currentactiveenemycount].positionx = screenwidth + 10 + i * 100;
        activeenemies[currentactiveenemycount].positiony = spawnyposition;
        activeenemies[currentactiveenemycount].movementspeed = enemyspeed;
        activeenemies[currentactiveenemycount].currenthealth = enemyhealth;
        activeenemies[currentactiveenemycount].isbossenemy = shouldspawnboss;
        activeenemies[currentactiveenemycount].isactive = true;
        activeenemies[currentactiveenemycount].targethorizontalposition = shouldspawnboss ? screenwidth - 550 : screenwidth - 450;

        currentactiveenemycount++;

        if (shouldspawnboss) {
            totalbossesspawned++;
        }
        totaleniemiesspawned++;
    }

    if (enemieskilledsincelastfooddrop >= enemies_killed_for_food_drop && currentfooditemcount < MAX_FOOD_ITEMS_ON_SCREEN) {
        fooditemsonScreen[currentfooditemcount].positionx = screenwidth + 50 + (rand() % 200);
        fooditemsonScreen[currentfooditemcount].positiony = rand() % (int)(screenheight - 100);
        fooditemsonScreen[currentfooditemcount].isactive = true;
        fooditemsonScreen[currentfooditemcount].issuperpOwer = false;
        currentfooditemcount++;
        enemieskilledsincelastfooddrop = 0;
    }

    if (enemieskilledsincelastsuperpower >= enemies_for_superpower_drop && currentfooditemcount < MAX_FOOD_ITEMS_ON_SCREEN) {
        fooditemsonScreen[currentfooditemcount].positionx = screenwidth + 50 + (rand() % 200);
        fooditemsonScreen[currentfooditemcount].positiony = rand() % (int)(screenheight - 100);
        fooditemsonScreen[currentfooditemcount].isactive = true;
        fooditemsonScreen[currentfooditemcount].issuperpOwer = true;
        currentfooditemcount++;
        enemieskilledsincelastsuperpower = 0;
    }
}

void moveenemies()
{
    if (!isgamerunning || isgameover || isgamepaused) return;

    for (int i = 0; i < currentactiveenemycount; ++i)
    {
        if (activeenemies[i].isactive)
        {
            Enemy& enemy = activeenemies[i];

            if (enemy.positionx > enemy.targethorizontalposition)
            {
                enemy.positionx -= enemy.movementspeed;
            }
            else
            {
                enemy.positionx = enemy.targethorizontalposition;
                enemy.positiony += enemy.movementspeed;

                double currentenemyheight = enemy.isbossenemy ? 120 : 100;
                if (enemy.positiony > screenheight - currentenemyheight || enemy.positiony < 0)
                {
                    enemy.movementspeed = -enemy.movementspeed;
                }
            }
        }
    }
}

void resetallgamestates() {
    playerx = 10;
    playery = screenheight / 2;
    playerhealth = 200;
    gamescore = 0;
    totalenemieskilled = 0;
    totaleniemiesspawned = 0;
    totalbossesspawned = 0;
    isgameover = false;
    hasgamebeenwon = false;
    currentplayerbulletcount = 0;
    currentenemybulletcount = 0;
    currentactiveenemycount = 0;
    regularenemieskilledsincelastbosswave = 0;
    isinbosswave = false;
    currentbossenemiesinwave = 0;
    endgametimercounter = 0; // Corrected: Used endgametimercounter
    currentfooditemcount = 0;
    enemieskilledsincelastfooddrop = 0;
    enemieskilledsincelastsuperpower = 0;
    issuperpOweractive = false;
    superpowerrstarttime = 0;
    superpowerbullettimer = 0;
    gametickcount = 0; // Reset gametickcount here as well for a fresh start
    isgamepaused = false;
    isyouwonpageactive = false; // Reset "You Won" page state
    isgameoverpageactive = false; // Reset "Game Over" page state
    isscoresaved = false; // Reset score saved state
    // isnameinputactive, name_input_index, current_player_name are NOT reset here,
    // as they are handled explicitly when entering name input.
    iResumeTimer(0);
}
void drawExplosions() {
    if (isgamepaused) return;
    // Adjusted ticks_per_frame to show 6 images within approximately 0.1 seconds
    // (2 ticks/frame * 6 frames * 10ms/tick = 120ms = 0.12 seconds)
    const int ticks_per_frame = 2;
    const int total_frames = 6;
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (activeExplosions[i].isActive) {
            int ticks_since_start = gametickcount - activeExplosions[i].startTime;
            int frame_index = ticks_since_start / ticks_per_frame;
            if (frame_index >= total_frames) {
                activeExplosions[i].isActive = false;
            } else {
                iShowLoadedImage(activeExplosions[i].x, activeExplosions[i].y, &explosionSprites[frame_index]);
            }
        }
    }
}
void createExplosion(double x, double y) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!activeExplosions[i].isActive) {
            activeExplosions[i].x = x;
            activeExplosions[i].y = y;
            activeExplosions[i].isActive = true;
            activeExplosions[i].startTime = gametickcount;
            return;
        }
    }
}

void rungamelogicanddisplay()
{
    if (isgamepaused || isgameover) return;

    // gametickcount++ moved to iDraw()

    if (issuperpOweractive && (gametickcount - superpowerrstarttime >= superpower_duration_ticks)) {
        issuperpOweractive = false;
        superpowerbullettimer = 0;
    }

    iShowLoadedImage(0, 0, &backgroundimage);
    iWrapImage(&backgroundimage, -10);
    iShowLoadedImage(playerx, playery, &playershipimage);

    for (int i = 0; i < currentactiveenemycount; ++i) {
        if (activeenemies[i].isactive) {
            if (activeenemies[i].isbossenemy)
                iShowLoadedImage(activeenemies[i].positionx, activeenemies[i].positiony, &finalbossshipimage);
            else
                iShowLoadedImage(activeenemies[i].positionx, activeenemies[i].positiony, &regularenemyshipimage);
        }
    }

    // Draw explosions here, as requested
    drawExplosions();

    for (int i = 0; i < currentplayerbulletcount; i++)
        iShowLoadedImage(playerbullets[i].positionx, playerbullets[i].positiony, &playerbulletimage);

    for (int i = 0; i < currentenemybulletcount; i++)
        iShowLoadedImage(enemybullets[i].positionx, enemybullets[i].positiony, &enemybulletimage);

    int validfooditemindex = 0;
    for (int i = 0; i < currentfooditemcount; ++i) {
        if (fooditemsonScreen[i].isactive) {
            fooditemsonScreen[i].positionx -= 10;
            if (fooditemsonScreen[i].issuperpOwer) {
                iShowLoadedImage(fooditemsonScreen[i].positionx, fooditemsonScreen[i].positiony, &superpowerfoodimage);
            } else {
                iShowLoadedImage(fooditemsonScreen[i].positionx, fooditemsonScreen[i].positiony, &fooditemimage);
            }

            if (fooditemsonScreen[i].positionx >= -100) {
                fooditemsonScreen[validfooditemindex++] = fooditemsonScreen[i];
            } else {
                fooditemsonScreen[i].isactive = false;
            }
        }
    }
    currentfooditemcount = validfooditemindex;


    validfooditemindex = 0;
    for (int i = 0; i < currentfooditemcount; ++i) {
        if (fooditemsonScreen[i].isactive) {
            if (checkCollision(playerx, playery, 90, 90, fooditemsonScreen[i].positionx, fooditemsonScreen[i].positiony, food_width, food_height)) {
                if (fooditemsonScreen[i].issuperpOwer) {
                    issuperpOweractive = true;
                    superpowerrstarttime = gametickcount;
                    superpowerbullettimer = 0;
                } else {
                    playerhealth = std::min(playerhealth + 100, max_player_health);
                }
                fooditemsonScreen[i].isactive = false;
            }
            if (fooditemsonScreen[i].isactive) {
                fooditemsonScreen[validfooditemindex++] = fooditemsonScreen[i];
            }
        }
    }
    currentfooditemcount = validfooditemindex;


    int validenemybulletindex = 0;
    for (int i = 0; i < currentenemybulletcount; i++)
    {
        if (!issuperpOweractive && checkCollision(enemybullets[i].positionx, enemybullets[i].positiony, bullet_width, bullet_height,
                                                 playerx + player_hitbox_offset_x, playery + player_hitbox_offset_y, player_hitbox_width, player_hitbox_height))
        {
            playerhealth -= (enemybullets[i].firedbyboss ? 3 : 2); // Increased enemy missile power
            enemybullets[i].positionx = -100;
        }
        else if (enemybullets[i].positionx >= 0)
        {
            enemybullets[validenemybulletindex++] = enemybullets[i];
        }
    }
    currentenemybulletcount = validenemybulletindex;

    if (playerhealth <= 0 && !isgameover)
    {
        playerhealth = 0;
        isgameover = true;
        hasgamebeenwon = false;
        isgamerunning = false;
        isgameoverpageactive = true; // Activate "Game Over" page
        endgametimercounter = gametickcount; // Start timer for "Game Over" page
        isscoresaved = false; // Make sure score saving is ready for this end state
        // resetallgamestates(); // Will be called after the "Game Over" page times out
        iPauseTimer(0);
        return;
    }

    int nextactiveenemywriteindex = 0;
    for (int i = 0; i < currentactiveenemycount; ++i)
    {
        if (activeenemies[i].isactive)
        {
            double enemyhitboxoffsetx = activeenemies[i].isbossenemy ? boss_enemy_hitbox_offset_x : regular_enemy_hitbox_offset_x;
            double enemyhitboxoffsety = activeenemies[i].isbossenemy ? boss_enemy_hitbox_offset_y : regular_enemy_hitbox_offset_y;
            double enemyhitboxwidth = activeenemies[i].isbossenemy ? boss_enemy_hitbox_width : regular_enemy_hitbox_width;
            double enemyhitboxheight = activeenemies[i].isbossenemy ? boss_enemy_hitbox_height : regular_enemy_hitbox_height;

            int validplayerbulletindex = 0;
            for (int j = 0; j < currentplayerbulletcount; j++)
            {
                if (playerbullets[j].positionx <= screenwidth &&
                    checkCollision(playerbullets[j].positionx, playerbullets[j].positiony, bullet_width, bullet_height,
                                   activeenemies[i].positionx + enemyhitboxoffsetx, activeenemies[i].positiony + enemyhitboxoffsety,
                                   enemyhitboxwidth, enemyhitboxheight))
                {
                    playerbullets[j].positionx = screenwidth + 100;

                    activeenemies[i].currenthealth -= 2;

                    if (activeenemies[i].currenthealth <= 0)
                    {
                        createExplosion(activeenemies[i].positionx, activeenemies[i].positiony);
                        iPlaySound("SelectedAssets/explode.wav", false, 100);
                        activeenemies[i].isactive = false;
                        totalenemieskilled++;
                        if (activeenemies[i].isbossenemy) {
                            gamescore += 100;
                        } else {
                            gamescore += 50;
                        }
                        enemieskilledsincelastfooddrop++;
                        enemieskilledsincelastsuperpower++;
                    }
                }
                   if (playerbullets[j].positionx <= screenwidth) {
                    playerbullets[validplayerbulletindex++] = playerbullets[j];
                }
            }
            currentplayerbulletcount = validplayerbulletindex;


            if (activeenemies[i].isactive) {
                activeenemies[nextactiveenemywriteindex++] = activeenemies[i];
            }
        }
    }
    currentactiveenemycount = nextactiveenemywriteindex;


    if (totalenemieskilled < total_enemies_required_to_win) {
        spawnnewenemies();
    } else if (currentactiveenemycount == 0 && totalenemieskilled >= total_enemies_required_to_win && !isgameover) {
        isgameover = true;
        hasgamebeenwon = true;
        isgamerunning = false;
        isyouwonpageactive = true; // Activate "You Won" page
        endgametimercounter = gametickcount; // Start timer for "You Won" page
        isscoresaved = false; // Make sure score saving is ready for this end state
        // resetallgamestates(); // This will be called after "You Won" page times out
        iPauseTimer(0);
        return;
    }

    if (issuperpOweractive) {
        superpowerbullettimer++;
        if (superpowerbullettimer >= superpower_bullet_interval) {
            playerfiresbullet();
            superpowerbullettimer = 0;
        }
    }

    iSetColor(240, 240, 240);
    iTextAdvanced(20, screenheight - 50, "HEALTH:", 0.15, 1);

    double healthbarx = 130;
    double healthbary = screenheight - 50 - 2;
    double healthbarheight = 25;
    double maxhealthbarwidth = 200;

    double currentdisplayhealth = playerhealth;
    if (currentdisplayhealth > max_player_health) {
        currentdisplayhealth = max_player_health;
    }

    double currenthealthbarwidth = (currentdisplayhealth / (double)max_player_health) * maxhealthbarwidth;

    iSetColor(50, 50, 50);
    iFilledRectangle(healthbarx, healthbary, maxhealthbarwidth, healthbarheight);

    if (playerhealth < 75) {
        iSetColor(255, 0, 0);
    } else {
        iSetColor(255, 255, 0);
    }
    iFilledRectangle(healthbarx, healthbary, currenthealthbarwidth, healthbarheight);

    iSetColor(255, 255, 255);
    iRectangle(healthbarx, healthbary, maxhealthbarwidth, healthbarheight);

    iSetColor(240, 240, 240);
    iTextAdvanced(20, screenheight - 100, "SCORE:", 0.15, 1);
    sprintf(scoredisplaystring, "%d", gamescore);
    iTextAdvanced(130, screenheight - 100, scoredisplaystring, 0.15, 1);

    iTextAdvanced(550, 20, "Press p to pause, r to resume and space to shoot", 0.10, 1);

    if (issuperpOweractive) {
        iSetColor(0, 255, 255);
        iTextAdvanced(screenwidth / 2 - 100, screenheight - 50, "SUPERPOWER ACTIVATED!", 0.2, 1);
    }
}

bool play_button_color=false;
bool instruction_button_color=false;
bool credits_button_color=false;
bool exit_button_color=false;
bool leader_board_color=false;

void showgamehomepage()
{
    iShowLoadedImage(0, 0, &backgroundimage);
    iSetColor(240, 240, 240);
    // iTextAdvanced(480, 550, "SPACE SHOOTER", 0.5, 7.5);
    iShowText(480, 550, "SPACE SHOOTER", "assets/fonts/RubikDoodleShadow-Regular.ttf", 72);
    if(play_button_color==true){
        iSetColor(74, 240, 229);
    }else{
        iSetColor(255, 255, 255);
    }
    // iTextAdvanced(screenwidth / 2 - 225, screenheight - 275-50, "PLAY", 0.3, 5);
    iShowText(screenwidth / 2 - 225, screenheight - 275-50,"PLAY", "assets/fonts/Monoton-Regular.ttf", 35);

    if(leader_board_color==true){
        iSetColor(74, 240, 229);
    }else{
        iSetColor(255, 255, 255);
    }
    // iTextAdvanced(screenwidth / 2 - 245-80, screenheight - 345-50, "LEADER BOARD", 0.3, 5);
    iShowText(screenwidth / 2 - 245-80, screenheight - 345-50,"LEADER BOARD", "assets/fonts/Monoton-Regular.ttf", 35);

    if(instruction_button_color==true){
        iSetColor(74, 240, 229);
    }else{
        iSetColor(255, 255, 255);
    }
    // iTextAdvanced(screenwidth / 2 - 225-80, screenheight - 415-50, "INSTRUCTIONS", 0.3, 5);
    iShowText(screenwidth / 2 - 225-80, screenheight - 415-50, "INSTRUCTIONS", "assets/fonts/Monoton-Regular.ttf", 35);

    if(credits_button_color==true){
        iSetColor(74, 240, 229);
    }else{
        iSetColor(255, 255, 255);
    }
    // iTextAdvanced(screenwidth / 2 - 225-20, screenheight - 485-50, "CREDITS", 0.3, 5);
    iShowText(screenwidth / 2 - 225-20, screenheight - 485-50, "CREDITS", "assets/fonts/Monoton-Regular.ttf", 35);
    if(exit_button_color==true){
        iSetColor(74, 240, 229);
    }else{
        iSetColor(255, 255, 255);
    }
    // iTextAdvanced(screenwidth / 2 - 215, screenheight - 555-50, "QUIT", 0.3, 5);
    iShowText(screenwidth / 2 - 215, screenheight - 555-50, "QUIT", "assets/fonts/Monoton-Regular.ttf", 35);
}

void showleaderboard(){
    iShowLoadedImage(0, 0, &backgroundimage);
    iSetColor(255, 255, 0);
    iTextAdvanced(screenwidth / 2 - 250-90, screenheight - 100, "LEADERBOARD", 0.4, 5);

    PlayerScore top_scores[100];
    int score_count = 0;
    loadScoresFromFile(top_scores, score_count);

    iSetColor(255, 255, 255);
    int display_count = std::min(5, score_count); // Use std::min
    if (display_count == 0) {
        iTextAdvanced(screenwidth / 2 - 200-90, screenheight / 2, "No scores yet. Go play!", 0.2, 1);
    } else {
        char rank_str[10];
        char score_str[20];
        iTextAdvanced(screenwidth / 2 - 300-90, screenheight - 200, "RANK", 0.2, 1);
        iTextAdvanced(screenwidth / 2 - 150-90, screenheight - 200, "NAME", 0.2, 1);
        iTextAdvanced(screenwidth / 2 + 200-90, screenheight - 200, "SCORE", 0.2, 1);

        for (int i = 0; i < display_count; ++i) {
            double y_pos = screenheight - 280 - (i * 60);
            sprintf(rank_str, "%d.", i + 1);
            iTextAdvanced(screenwidth / 2 - 300-90, y_pos, rank_str, 0.2, 1);
            iTextAdvanced(screenwidth / 2 - 150-90, y_pos, top_scores[i].name, 0.2, 1);
            sprintf(score_str, "%d", top_scores[i].score);
            iTextAdvanced(screenwidth / 2 + 200-90, y_pos, score_str, 0.2, 1);
        }
    }
    iTextAdvanced(650, 20, "Press b to go back to homepage", 0.10, 1);
}


void showgameinstructions()
{
    iShowLoadedImage(0, 0, &instructionpageimage);
    iSetColor(255, 255, 255);
    // iTextAdvanced(60, 650, "BACK", 0.15, 1); // Removed
}

void showgamecredits()
{
    iShowLoadedImage(0, 0, &creditspageimage);
    iSetColor(255, 255, 255);
    // iTextAdvanced(60, 620, "BACK", 0.15, 1); // Removed
}
void managemusicplayback()
{
    if (isgamerunning == true)
    {
        iPauseSound(homescreenmusic);
        gameplaymusic = iPlaySound("SelectedAssets/playMuic.wav", true, 100);
    }
    else
    {
        iPauseSound(gameplaymusic);
        homescreenmusic = iPlaySound("SelectedAssets/Menu.wav", true, 100);
    }
}

void showyouwonpage() {
    iShowLoadedImage(0, 0, &backgroundimage);
    iSetColor(0, 255, 0); // Green color
    iTextAdvanced(screenwidth / 2 - 200, screenheight / 2, "YOU WON!", 0.5, 5);

    if (gametickcount - endgametimercounter >= you_won_screen_duration_ticks) {
        if (!isscoresaved) {
            saveScoreToFile();
            isscoresaved = true;
        }
        isyouwonpageactive = false;
        ishomepageactive = true;
        endgametimercounter = 0; // Reset timer for next win
        resetallgamestates(); // Reset after the "You Won" screen is done
        managemusicplayback(); // Play homepage music
    }
}

void showgameoverpage() {
    iShowLoadedImage(0, 0, &backgroundimage);
    iSetColor(255, 0, 0); // Red color
    iTextAdvanced(screenwidth / 2 - 200, screenheight / 2, "GAME OVER!", 0.5, 5);

    if (gametickcount - endgametimercounter >= you_won_screen_duration_ticks) { // Reusing same duration
        if (!isscoresaved) {
            saveScoreToFile();
            isscoresaved = true;
        }
        isgameoverpageactive = false;
        ishomepageactive = true;
        endgametimercounter = 0; // Reset timer
        resetallgamestates(); // Reset after the "Game Over" screen is done
        managemusicplayback(); // Play homepage music
    }
}




void iDraw()
{
    // Increment gametickcount at the start of iDraw() to ensure it always progresses
    gametickcount++;

    iClear();
    if (ishomepageactive == true)
    {
        showgamehomepage();
    }
    else if (isgamerunning == true)
    {
        rungamelogicanddisplay();
        if (isgamepaused) {
            iSetColor(0, 0, 0);
            iFilledRectangle(0, 0, screenwidth, screenheight);
            iSetColor(255, 255, 255);
            iTextAdvanced(screenwidth / 2 - 100, screenheight / 2 + 20, "PAUSED", 0.25, 2);
            iTextAdvanced(screenwidth / 2 - 170, screenheight / 2 - 30, "Press 'r' to resume", 0.15, 1);
        }
    }
    else if (isnameinputactive)
    {
        showNameInputScreen();
    }
    else if (iscreditpageactive == true)
    {
        showgamecredits();
    }
    else if (isinstructionpageactive == true)
    {
        showgameinstructions();
    }else if(isleaderboardpageactive==true){
        showleaderboard();
    }
    else if (isyouwonpageactive == true) // New condition for "You Won" page
    {
        showyouwonpage();
    }
    else if (isgameoverpageactive == true) // New condition for "Game Over" page
    {
        showgameoverpage();
    }
}

void iMouseMove(int mousex, int mousey)
{
    if(ishomepageactive==true){
        if(701<=mousex && mousex<=794 && 426<=mousey && mousey<=457){
            play_button_color=true;
        }else if(620<=mousex && mousex<=880 && 285<=mousey && mousey<=317){
            instruction_button_color=true;
        }else if(683<=mousex && mousex<=831 && 213<=mousey && mousey<=249){
            credits_button_color=true;
        }else if(710<=mousex && mousex<=790 && 145<=mousey && mousey<=181){
            exit_button_color=true;
        }else if(600<=mousex && mousex<=896 && 353<=mousey && mousey<=388){
            leader_board_color=true;
        }
        else{
            play_button_color=false;
            instruction_button_color=false;
            credits_button_color=false;
            exit_button_color=false;
            leader_board_color=false;
        }
    }

}

void iMouseDrag(int mousex, int mousey)
{
}

void iMouse(int button, int state, int mousex, int mousey)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        std::cout<<mousex<<" "<<mousey<<std::endl; // Use std::cout
        if (ishomepageactive == true)
        {
            if (703<=mousex && mousex<=793 && 426<=mousey && mousey<=457)
            {
                ishomepageactive = false;
                isnameinputactive = true; // Activate name input screen
                strcpy(current_player_name, ""); // Clear previous name
                name_input_index = 0; // Reset name input index
            }
            else if(600<=mousex && mousex<=896 && 353<=mousey && mousey<=388)
            {
                isleaderboardpageactive=true;
                ishomepageactive=false;
            }
            else if (621<=mousex && mousex<=881 && 285<=mousey && mousey<=317)
            {
                ishomepageactive = false;
                isinstructionpageactive = true;
            }
            else if (682<=mousex && mousex<=829 && 213<=mousey && mousey<=249)
            {
                ishomepageactive = false;
                iscreditpageactive = true;
            }
            else if (710<=mousex && mousex<=790 && 145<=mousey && mousey<=181)
            {
                exit(0);
            }
        }
        // These checks remain valid for navigating back if not from clicking a specific "BACK" text
        if (isinstructionpageactive == true && mousex >= 49 && mousex <= 176 && mousey >= 627 && mousey <= 725)
        {
            isinstructionpageactive = false;
            ishomepageactive = true;
        }
        if (iscreditpageactive == true && mousex >= 46 && mousex <= 152 && mousey >= 595 && mousey <= 682)
        {
            iscreditpageactive = false;
            ishomepageactive = true;
        }
    }
}

void iMouseWheel(int direction, int mousex, int mousey)
{
}

void iKeyPress(unsigned char key)
{
    if (isnameinputactive) {
        if (key == '\r') {
            if (name_input_index > 0) {
                current_player_name[name_input_index] = '\0';
                // Score is saved after game completion, not here (pre-game input)
            }
            isnameinputactive = false;
            ishomepageactive = false; // Go to game screen
            isgamerunning = true; // Start the game
            resetallgamestates(); // Reset and prepare game variables
            spawnnewenemies(); // Spawn initial enemies
            managemusicplayback(); // Start game music
        } else if (key == '\b') {
            if (name_input_index > 0) {
                name_input_index--;
                current_player_name[name_input_index] = '\0';
            }
        } else if (name_input_index < 50 && isalnum(key)) {
            current_player_name[name_input_index] = key;
            name_input_index++;
            current_player_name[name_input_index] = '\0';
        }
        return; // Consume the key press
    }

    if (key == 'p' || key == 'P')
    {
        if (isgamerunning && !isgameover) {
            isgamepaused = true;
        }
    }
    else if (key == 'r' || key == 'R')
    {
        if (isgamerunning && !isgameover) {
            isgamepaused = false;
        }
    }
    else if (key == 'q' || key == 'Q')
    {
        exit(0);
    }
    else if (key == 'b' || key == 'B')
    {
        if (isinstructionpageactive == true)
        {
            isinstructionpageactive = false;
            ishomepageactive = true;
        }
        else if (iscreditpageactive == true)
        {
            iscreditpageactive = false;
            ishomepageactive = true;
        }
        else if (isgamerunning == true && !isgameover)
        {
            isgamerunning = false;
            ishomepageactive = true;
            managemusicplayback();
            resetallgamestates();
        }
        else if(isleaderboardpageactive==true){
            isleaderboardpageactive=false;
            ishomepageactive=true;
        }

    }
    else if (key == ' ')
    {
        if (!issuperpOweractive && isgamerunning && !isgameover && !isgamepaused) {
            playerfiresbullet();
        }
    }
}

void iSpecialKeyPress(unsigned char key)
{
    if (isgamerunning && !isgameover && !isgamepaused) {
        if (key == GLUT_KEY_RIGHT && playerx <= (screenwidth / 2) - 100)
        {
            playerx += 15;
        }
        if (key == GLUT_KEY_LEFT && playerx >= 0)
        {
            playerx -= 15;
        }
        if (key == GLUT_KEY_UP && playery <= screenheight - 100)
        {
            playery += 15;
        }
        if (key == GLUT_KEY_DOWN && playery >= 0)
        {
            playery -= 15;
        }
    }
}
void loadallgameresources()
{
    iLoadImage(&backgroundimage, "SelectedAssets/background.png");
    iResizeImage(&backgroundimage, screenwidth, screenheight);
    iLoadImage(&playershipimage, "SelectedAssets/MainSpaceShip.png");
    iResizeImage(&playershipimage, 90, 90);
    iLoadImage(&instructionpageimage, "SelectedAssets/instructionsFinal.png");
    iResizeImage(&instructionpageimage, screenwidth - 350, screenheight);
    iLoadImage(&creditspageimage, "SelectedAssets/creditsfinal.png");
    iResizeImage(&creditspageimage, screenwidth - 350, screenheight);
    iLoadImage(&regularenemyshipimage, "SelectedAssets/Ship3.png");
    iResizeImage(&regularenemyshipimage, 100, 100);
    iLoadImage(&finalbossshipimage, "SelectedAssets/FinalBoss.png");
    iResizeImage(&finalbossshipimage, 120, 120);
    iLoadImage(&playerbulletimage, "SelectedAssets/bullet.png");
    iLoadImage(&enemybulletimage, "SelectedAssets/Charge_1.png");
    iResizeImage(&enemybulletimage, 20, 20);
    iLoadImage(&fooditemimage, "SelectedAssets/food.png");
    iResizeImage(&fooditemimage, 75, 75);
    iLoadImage(&superpowerfoodimage, "SelectedAssets/superfood.png");
    iResizeImage(&superpowerfoodimage, 75, 75);
    char imagePath[100];
    for (int i = 0; i < 6; i++) {
        sprintf(imagePath, "SelectedAssets/sprites/%d.png", i + 1);
        // Ensure these image files exist at the specified path
        iLoadImage(&explosionSprites[i], imagePath);
        iResizeImage(&explosionSprites[i], 120, 120);
    }

    for(int i=0; i<MAX_EXPLOSIONS; i++){
        activeExplosions[i].isActive = false;
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadallgameresources();
    iInitializeSound();
    gameplaymusic = iPlaySound("SelectedAssets/playMuic.wav", true, 100);
    homescreenmusic = iPlaySound("SelectedAssets/Menu.wav", true, 100);
    iPauseSound(gameplaymusic);

    iSetTimer(50, moveplayermissiles);
    iSetTimer(500, fireenemybullets);
    iSetTimer(50, moveenemybullets);
    iSetTimer(50, moveenemies);
    iSetTimer(2000, spawnnewenemies);

    // As iDraw is the main drawing function called by GLUT, and it now manages
    // when to call rungamelogicanddisplay, there's no separate timer needed for rungamelogicanddisplay.

    srand(time(0));

    iInitialize(screenwidth, screenheight, "Space Shooter");
    return 0;
}
