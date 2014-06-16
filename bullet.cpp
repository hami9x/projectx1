#include "bullet.h"

#include "texture.h"
#include "global.h"
#include <string>
#include <SDL.h>
#include <chipmunk_private.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include "player.h"


namespace xx {

Bullet::Bullet() {
    mVelX=5;
    mVelY=5;
    mRange=0;
    exist=false;
    loadedImg=false;
}

Bullet::~Bullet() {
}

void Bullet::free() {
    img.free();
}

int n = 0;
void Bullet::destroy() {
    if (exist) {
        if (mBody == NULL) {
            return;
        }
        std::cout << mBody; printf(" :D %d\n", ++n);
        cpShape *shape = mBody->shapeList;
        cpSpace *space = cpBodyGetSpace( mBody );

        cpSpaceRemoveShape( space, shape );
        cpSpaceRemoveBody( space, mBody );
        cpShapeFree( shape );
        cpBodyFree( mBody );

        mBody = NULL;
        exist = false;

    }
    mRange = 0;
}

void Bullet::createBullet(SDL_Renderer *r, cpSpace *space, double range, cpDataPointer p) {

    Player *player = (Player*)p;
    //printf("in bullet %d\n", player);
    mVelX = player->velX();
    mVelY = player->velY();
    cpVect v = cpv(mVelX, mVelY);
    cpVect resV = cpvmult(cpvmult(v, 1./cpvlength(cpv(mVelX, mVelY))), BULLET_SPEED);
    mVelX = resV.x;
    mVelY = resV.y;
    mAngle = player->angle();
    mPosX = player->posX();
    mPosY = player->posY();
    //mX = player->sensor();
    mPlayer = player;

    if (!loadedImg) {
        loadedImg = true;
        img.loadFromFile(r,BULLET_IMG);
    }

    exist = true;
    mRange = range;
    mBody = cpBodyNew(10.f, cpMomentForBox(10.f, img.width(), img.height()));
    mBody->p.x = mPosX;
    mBody->p.y = mPosY;


    //printf("%d \n",mBody->userData);

    cpBodySetCenterOfGravity(mBody, cpv(img.width()/2, img.height()/2));
    cpShape *shape;

    shape = cpBoxShapeNew(mBody, img.width(), img.height(), 0);
    cpShapeSetCollisionType(shape, BULLET_TYPE);
    //cpShapeSetSensor(shape, true);
    cpSpaceAddShape(space, shape);
    cpSpaceAddBody(space, mBody);

    cpBodySetAngle(mBody,mAngle);
    cpBodySetUserData(mBody, this);
}

void Bullet::render(SDL_Renderer * r) {
    cpVect pos = cpBodyGetPosition(mBody);
    cpVect cog = cpBodyGetCenterOfGravity(mBody);
    cpShape *shape = mBody->shapeList;
    cpBody * b = shape->body;
    double t = sqrt( pow( pos.x - mPosX, 2 ) + pow( pos.y - mPosY, 2 ) );
    img.render(r, (int)pos.x - cog.x, (int)pos.y - cog.y, NULL ,(double)rad2deg(mAngle), NULL, SDL_FLIP_NONE);
    if( t > mRange) {
       destroy();
    }
}

void Bullet::moveBullet() {
    mBody->p.x += mVelX;
    mBody->p.y += mVelY;
    //cpBodyApplyForceAtWorldPoint(mBody, cpv(mVelX, mVelY), cpv(0, 0));
}

}

