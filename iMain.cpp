#include "iGraphics.h"
#include <math.h>
#include <iostream>
#include "iSound.h"
#
bool homepage = true, startPage = false, creditPage = false, instructionPage = false; // page con
double screen_height = 750, screen_width = 1850;
double px = 10, py = screen_height / 2, enemy_x = screen_width + 10, enemy_y = screen_height / 2, finalBoss_x=screen_width+10, finalBoss_y=screen_height/2; // Space ship position
double misile_speed = 50, enemy_speed=6, final_boss_speed=10;
Image planeImg, bulletImg, backgroundImg, instructionImg, creditsImg, enemy_shipsImg, finalBoss_shipImg; // image
int HomeMusic, PlayMusic;// Sound Storing Variable
int score = 0;
char score_string[20], my_health_string[20];

int booss_coming = -1, enemy_coming = 1;
int boss_health=200, enemy_health=100, my_health=100;

int difficulty=0;


struct Bullet
{
    double x, y;
};

struct EnemyBullet
{
    double x, y;
};

struct finaBossBullet{
    double x, y;
};

EnemyBullet enemyBullets[50];
int enemyBulletCount = 0;
double enemyBulletSpeed = 20;

void enemyShoot()
{
    if (startPage && enemyBulletCount < 50)
    {
        enemyBullets[enemyBulletCount].x = enemy_x;
        enemyBullets[enemyBulletCount].y = enemy_y + 30;
        enemyBulletCount++;
    }
}

void enemyBulletMovement()
{
    for (int i = 0; i < enemyBulletCount; i++)
    {
        enemyBullets[i].x -= enemyBulletSpeed;
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


Bullet bullets[50];
int bulletCount = 0;

void missileMovement()
{
    for (int i = 0; i < bulletCount; i++)
    {
        bullets[i].x += misile_speed;
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

    if (startPage && bulletCount < 50)
    {
        bullets[bulletCount].x = px + 90;
        bullets[bulletCount].y = py + 33;
        bulletCount++;
    }
}


finaBossBullet finalBossBullets[50];
int finalBossBulletCount = 0;
double finalBossBulletSpeed = 20;




void enemy_movement()
{
    if (startPage == false)
    {
        return;
    }
    
    if(enemy_coming==1){
        if(enemy_x>screen_width-450){
            enemy_x-=enemy_speed;
        }else{
            enemy_y+=enemy_speed;
            if(enemy_y>screen_height-100 || enemy_y<0){
                enemy_speed=-enemy_speed;
            }
        }
    }
    if(booss_coming==1){
        if(finalBoss_x>screen_width-500){
            finalBoss_x-=final_boss_speed;
        }else{
            finalBoss_y+=final_boss_speed;
            if(finalBoss_y>screen_height-150 || finalBoss_y<0){
                final_boss_speed=-final_boss_speed;
            }
        }
    }
}

void startGame()
{
    if (my_health == 0)
    {
        // close the game
    }

    iShowLoadedImage(0, 0, &backgroundImg);
    iWrapImage(&backgroundImg, -10);
    iShowLoadedImage(px, py, &planeImg);
    iShowLoadedImage(finalBoss_x, finalBoss_y, &finalBoss_shipImg);
    iShowLoadedImage(enemy_x, enemy_y, &enemy_shipsImg);
    for (int i = 0; i < bulletCount; i++)
    {
        iShowLoadedImage(bullets[i].x, bullets[i].y, &bulletImg);
    }
    for (int i = 0; i < enemyBulletCount; i++)
    {
        iShowLoadedImage(enemyBullets[i].x, enemyBullets[i].y, &bulletImg);
    }

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
    iShowImage(screen_width / 2 - 250, screen_height - 300, "SelectedAssets/start.png"); // play button
    iShowImage(screen_width / 2 - 250, screen_height - 370, "SelectedAssets/start.png"); // instruction button
    iShowImage(screen_width / 2 - 250, screen_height - 440, "SelectedAssets/start.png"); // credit button
    iShowImage(screen_width / 2 - 250, screen_height - 510, "SelectedAssets/Quit.png");  // quit button
    iSetColor(240, 240, 240);
    iTextAdvanced(480, 550, "SPACE SHOOTER", 0.5, 7.5); // space shooter text
}

void showScores()
{
}

void showInstructions()
{
    iShowLoadedImage(0, 0, &instructionImg);
}

void showCredits()
{
    iShowLoadedImage(0, 0, &creditsImg);
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
        startGame();
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
    // px=mx;
    // py=my;
}

void iMouseDrag(int mx, int my)
{
    // place your codes here
}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        cout << mx << " " << my << endl;

        if (homepage == true)
        {
            if (675 < mx && mx < 803 && 451 < my && my < 493)
            {
                homepage = false;
                startPage = true;
                px = 10;
                py = screen_height / 2;
                soundWorks();
            }
            else if (675 < mx && mx < 803 && 380 < my && my < 423)
            {
                homepage = false;
                creditPage = true;
            }
            else if (675 < mx && mx < 803 && 310 < my && my < 350)
            {
                homepage = false;
                instructionPage = true;
            }
            else if (675 < mx && mx < 803 && 240 < my && my < 280)
            {
                exit(0);
            }
        }
        if (instructionPage == true && 49 < mx && mx < 176 && 627 < my && my < 725)
        {
            instructionPage = false;
            homepage = true;
        }
        if (creditPage == true && 46 < mx && mx < 152 && 595 < my && my < 682)
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
        iPauseTimer(0);
    }
    else if (key == 'r')
    {
        iResumeTimer(0);
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
        if (creditPage == true)
        {
            creditPage = false;
            homepage = true;
        }
        if (startPage == true)
        {
            startPage = false;
            homepage = true;
            soundWorks();
        }
    }
    else if (key == ' ')
    {
        shootBullet();
    }else if(key=='t'){
        booss_coming=-booss_coming;
        enemy_coming=-enemy_coming;
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
    if (key == GLUT_KEY_RIGHT && px <= screen_width - 440)
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
    if (key == GLUT_KEY_DOWN && py >= -10)
    {
        py -= 15;
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
    iSetTimer(50, missileMovement);
    iSetTimer(1000, enemyShoot);
    iSetTimer(50, enemyBulletMovement);
    iSetTimer(50, enemy_movement);
    iInitialize(screen_width, screen_height, "Space Shooter");
    return 0;
}