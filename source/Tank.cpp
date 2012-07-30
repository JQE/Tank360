#include "Tank.h"
#include <zlx/Draw.h>

using namespace ZLX;


void cTank::draw() {
    if (Exploding > 0) {
        float eTop=33, eBottom=63, eRight=327, eLeft=297;
        switch (Exploding) {
            eTop = 33;
            eBottom = 63;
            case 8:
            case 7:
                eRight = 327;
                eLeft = 297;
                break;
            case 6:
            case 5:
                eRight = 63;
                eLeft = 33;
                break;
            case 4:
            case 3:
                eRight = 95;
                eLeft = 65;
                break;
            case 2:
            case 1:
                eRight = 128;
                eLeft = 98;
                break;               
        }
        Draw::DrawClipTexturedRect(x+15, y+15, (float)Map->width, (float)Map->height,eTop, eBottom, eRight, eLeft, Map);
        Exploding--;
        if (Exploding == 0 ) {
            sCount = rand()%4;
            spawn(sPoint[sCount].x, sPoint[sCount].y, sPoint[sCount].direction);
        }
    } else {
        Draw::DrawClipTexturedRect(x, y, (float)Map->width, (float)Map->height,top, bottom, right, left, Map);
    }
    if (shotTimer >0 ) shotTimer--;
    int loop;
    for (loop = 0; loop < maxBullets; loop++) {
        B1[loop].draw();
    }
}

void cTank::update() {
 
    move();
    checkBounds();
    for (int loop = 0; loop < maxBullets; loop++) {
        B1[loop].move();
        B1[loop].checkBounds();
        if (B1[loop].getShot() == 0 && loop < bCount) {
            bCount = loop;
        }
    }
}

void cTank::Finalize() {
    if (Impact) {
        x = oldx;
        y = oldy;
    }
    checkBounds();
    Impact = 0;
}

void cTank::Collision(cTank *P2) {
    
    BOOL boom = FALSE;
    int left = P2->getX()-8;
    int bottom = P2->getY();
    int right = left+39;
    int top = bottom+30;
    int right2 = x+39;
    int top2 = y+30;
    int bottom2 = y;
    int left2 = x-8;
    
    if ((left >= left2 && left <= right2) ||
       (right >= left2 && right <= right2)) {
        if ((bottom >= bottom2 && bottom <= top2) ||
        (top >= bottom2 && top <= top2)) {
            boom = TRUE;
        }
    }
    
    if ((left2 >= left && left2 <= right) ||
       (right2 >= left && right2 <= right)) {
        if ((bottom2 >= bottom && bottom2 <= top) ||
        (top2 >= bottom && top2 <= top)) {
            boom = TRUE;
        }
    } 
    
    if (boom) {       
        Impact = TRUE;
    }
    BOOL hit = FALSE;
    for (int loop = 0; loop < maxBullets; loop++) {
        if (B1[loop].Collision(P2->getX(), P2->getY())) {
            B1[loop].Shot = 0;
            score++;
            P2->Exploding = 12;
            loop = maxBullets;
            hit = TRUE;
        }
    }
    if (hit == TRUE) {
        for (int loop = 0; loop < maxBullets; loop++) {
            B1[loop].Shot = 0;
        }
    }    
}

BOOL cBullet::Collision(float nx, float ny ) {
    
    if (Shot == 1) {
        int left = nx;
        int bottom = ny;
        int right = left+40;
        int top = bottom+30;
        int right2 = x+10;
        int top2 = y+10;
        int bottom2 = y;
        int left2 = x;

        if ((left >= left2 && left <= right2) ||
           (right >= left2 && right <= right2)) {
            if ((bottom >= bottom2 && bottom <= top2) ||
            (top >= bottom2 && top <= top2)) {
                return TRUE;
            }
        }

        if ((left2 >= left && left2 <= right) ||
           (right2 >= left && right2 <= right)) {
            if ((bottom2 >= bottom && bottom2 <= top) ||
            (top2 >= bottom && top2 <= top)) {
                return TRUE;
            }
        }
    }
       
    return FALSE;
}

void cTank::checkBounds() {
    if (y < 40)
        y = 40;
    if (y > 600)
        y = 600;
    if (x > 1180)
        x = 1180;
    if (x < 50)
        x = 50;
}

void cBullet::checkBounds() {
    if ((y < 40) || (y > 650) || (x > 1180) || (x < 50)) {
        Shot = 0;
    }
}

void cTank::getImage( ) {
    switch(Direction) {
        case 0:
            break;
        case r:
            left = 530;
            bottom = 100;
            right = 560;
            top = 130;
            break;
        case l:
            left = 563;
            bottom = 100;
            right = 593;
            top = 130;
            break;
        case u:
            left = 530;
            bottom = 67;
            right = 560;
            top = 97;
            break;
        case d:
            left = 530;
            bottom = 33;
            right = 560;
            top = 63;
            break;
    }
}

void cTank::shoot() {
    if (shotTimer == 0 && bCount < maxBullets) {
        if (B1[bCount].getShot() == 0) {
            shotTimer = 10;
            B1[bCount].spawn(x, y, Direction);
            bCount++;
        }
    }
}

void cTank::move() {
    if (Movement) {
        switch(Direction) {
        case 0:
            break;
        case 1:
            oldx = x;
            x += 3;
            //if (x > 1180) x = 1180;
            getImage();
            break;
        case 2:
            oldx = x;
            x -= 3;
            //if (x < 50) x = 50;
            getImage();
            break;
        case 3:
            oldy = y;
            y += 3;
            //if (y > 670) y = 650;
            getImage();
            break;
        case 4:
            oldy = y;
            y -= 3;
            //if (y < 50) y = 40;
            getImage();
            break;
        }
     }
    Movement = FALSE;
}

void cTank::spawn(float nx, float ny, int ndirection) {
        
    x = nx;
    y=ny;
    oldx = nx;
    oldy = ny;
    Direction = ndirection;
        
    shotTimer = 0;
    Impact = 0;
    
    for (int loop = 0; loop < maxBullets; loop++) {
        B1[loop].init(Map);
    }
    getImage();
}

void cTank::setTexture(ZLXTexture *Tex) {
    Map = Tex;
    score = 0;
    Exploding = 0;
}



void cBullet::init(ZLXTexture *Tex) {
    Map = Tex;
    Shot = 0;
    x = 0;
    y = 0;
    direction = r;
    top = 0;
    bottom = 0;
    left = 0;
    right = 0;
    
}

void cBullet::spawn(float xN, float yN, int DirectionN) {
    if (Shot != 1) {
        Shot = 1;
        x = xN;
        y = yN;
        direction = DirectionN;
        switch(direction) {
            case r:
                x+=30;
            case l:
                y += 13;
                break;
            case u:
                y+=30;
            case d:
                x +=13;
                break;
        }
        getImage();
    }
}
    
void cBullet::move() {
    if (Shot == 1) {
        switch(direction) {
        case d:
            y -= 5;
            //if (y < 40) Shot = 0;
            break;
        case u:
            y +=5;
            //if (y > 650) Shot = 0;
            break;
        case r:
            x +=5;
            //if (x > 1180) Shot = 0;
            break;
        case l:
            x -=5;
            //if (x < 50) Shot = 0;
            break;
        }
    }
}
    
void cBullet::getImage() {
    top = 53;
    bottom = 43;
    switch(direction) {
        case u:
            right = 250;
            left = 240;
            break;
        case d:
            right = 152;
            left = 142;
            break;
        case r:
            right = 218;
            left = 208;
            break;
        case l:
            right = 185;
            left = 175;
            break;
    }
}
    
void cBullet::draw() {
    if (Shot == 1) {
        Draw::DrawClipTexturedRect(x, y, (float)Map->width, (float)Map->height, top, bottom, right, left, Map);
    }
}

