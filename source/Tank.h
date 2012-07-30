/* 
 * File:   Tank.h
 * Author: jqe
 *
 * Created on August 4, 2011, 10:27 AM
 */

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <zlx/Draw.h>


#ifndef TANK_H
#define	TANK_H

#define n 0
#define r 1
#define l 2
#define u 3
#define d 4

struct spawnPoint {
    float x, y;
    int direction;
};

class cBullet {
public:
        
    int Shot;
    float x;
    float y;
    float bottom;
    float top;
    float right;
    float left;
    int direction;
    
    ZLXTexture *Map;
    

    void spawn(float xN, float yN, int DirectionN);
    void move();
    void getImage();
    void draw();
    void init(ZLXTexture *Map);
    void checkBounds();
    BOOL Collision(float x, float y);
    
    int getShot() { return Shot; }
    float getX() { return x; }
    float getY() { return y; }
    
};

#define maxBullets 4

class cTank {
public:
    int tankNum;
    float x;
    float y;
    float oldx;
    float oldy;
    spawnPoint sPoint[4];
    int sCount;
    
    cBullet B1[maxBullets];
    int bCount;
        
    int Direction;
    BOOL Movement;
    int shotTimer;
    BOOL Impact;
    int score;
    int Exploding;
    
    ZLXTexture *Map;
    
    //std::vector<Bullet> Blist;

    void spawn(float x, float y, int direction);
    void move();
    void getImage();
    void draw();
    void shoot();
    void update();
    void Collision(cTank *P2);
    void setDirection(int Dir) { Direction = Dir; Movement = TRUE; }
    void checkBounds();
    void Finalize();
    
    void setX(float sX) { x = sX; }
    void setY(float sY) { y = sY; }
    float getX() { return x; }
    float getY() { return y; }
    int getDirection() { return Direction; }
    int getTankNum() { return tankNum; }
    
    void setTexture(ZLXTexture *Tex);
    
    
    float bottom;
    float top;
    float right;
    float left;
    
};


#endif	/* TANK_H */

