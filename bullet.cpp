#include "bullet.h"

#include "texture.h"
#include "global.h"
#include <string>
#include <SDL.h>
#include <chipmunk_private.h>
#include <stdio.h>
#include <math.h>
#include "player.h"

namespace xx {

Bullet::Bullet() {
    mVelX=5;
    mVelY=5;
    mRange=0;
    exist=false;
}

Bullet::~Bullet() {
    free();
}

void Bullet::free() {
    if (exist) {
        cpShape *shape = mBody->shapeList;
        cpSpace *space = cpBodyGetSpace( mBody );

        img.free();
        cpSpaceRemoveShape( space, shape );
        cpSpaceRemoveBody( space, mBody );
        cpShapeFree( shape );
        cpBodyFree( mBody );

        exist = false;

    }
    mRange = 0;
}

void Bullet::createBullet(SDL_Renderer *r, cpSpace *space, double range, cpDataPointer p) {

    Player *player = (Player*)p;
    //printf("in bullet %d\n", player);
    mVelX = player->velX();
    mVelY = player->velY();
    mAngle = player->angle();
    mPosX = player->posX();
    mPosY = player->posY();
    mX = player->sensor();

    img.loadFromFile(r,BULLET_IMG);
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
    cpShapeSetSensor(shape, true);
    cpSpaceAddShape(space, shape);
    cpSpaceAddBody(space, mBody);

    cpBodySetAngle(mBody,mAngle);
    cpBodySetUserData(mBody, this);
}

void Bullet::render(SDL_Renderer * r) {
    cpVect pos = cpBodyGetPosition(mBody);
    cpVect cog = cpBodyGetCenterOfGravity(mBody);
    cpShape *shape = mBody->shapeList;
    double t = sqrt( pow( pos.x - mPosX, 2 ) + pow( pos.y - mPosY, 2 ) );
    if( t > mRange)
       free();
    if( t > mX )
        cpShapeSetSensor(shape, false);
    img.render(r, (int)pos.x - cog.x, (int)pos.y - cog.y, NULL ,(double)rad2deg(mAngle), NULL, SDL_FLIP_NONE);
}

void Bullet::moveBullet() {
    mBody->p.x += mVelX;
    mBody->p.y += mVelY;
    cpBodyApplyForceAtLocalPoint(mBody, cpv(mVelX, mVelY), cpv(0, 0));
}

}

