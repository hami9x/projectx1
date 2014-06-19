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

using namespace std;
namespace xx {

Bullet::Bullet():
    mVelX(5),
    mVelY(5),
    mRange(0),
    mExist(false),
    mBody(NULL),
    mLoadedImg(false),
    mPlayer(NULL),
    mInitialAngle(0),
    mNoRender(false)
{}

Bullet::~Bullet() {
}

void Bullet::free() {
    if (mLoadedImg) {
        img.free();
    }
}

void Bullet::destroy() {
    if (mExist) {
        if (mBody == NULL) {
            return;
        }
        cpShape *shape = mBody->shapeList;
        cpSpace *space = cpBodyGetSpace( mBody );

        cpSpaceRemoveShape( space, shape );
        cpSpaceRemoveBody( space, mBody );
        cpShapeFree( shape );
        cpBodyFree( mBody );

        mBody = NULL;
        mExist = false;

    }
    mRange = 0;
}

void Bullet::createBullet(SDL_Renderer *r, cpSpace *space, double range, Player * player, bool noRender) {
    mNoRender = noRender;
    //printf("in bullet %d\n", player);
    mVelX = player->velX();
    mVelY = player->velY();
    cpVect v = cpv(mVelX, mVelY);
    cpVect resV = cpvmult(cpvmult(v, 1./cpvlength(cpv(mVelX, mVelY))), BULLET_SPEED);
    mVelX = resV.x;
    mVelY = resV.y;
    mInitialAngle = player->angle();
    mPosX = player->posX();
    mPosY = player->posY();
    //mX = player->sensor();
    mPlayer = player;

    if (!mNoRender && !mLoadedImg) {
        mLoadedImg = true;
        img.loadFromFile(r, BULLET_IMG);
    }

    mExist = true;
    mRange = range;
    mBody = cpBodyNew(0.1f, INFINITY);

    mBody->p.x = mPosX;
    mBody->p.y = mPosY;


    //printf("%d \n",mBody->userData);

    cpBodySetCenterOfGravity(mBody, cpv(BULLET_WIDTH/2, BULLET_HEIGHT/2));
    cpShape *shape;

    shape = cpBoxShapeNew(mBody, BULLET_WIDTH, BULLET_HEIGHT, 0);
    cpShapeSetCollisionType(shape, BULLET_TYPE);
    //cpShapeSetSensor(shape, true);
    cpSpaceAddShape(space, shape);
    cpSpaceAddBody(space, mBody);

    cpBodySetAngle(mBody, mInitialAngle);
    cpBodySetUserData(mBody, this);

    cpBodySetVelocity(mBody, cpv(v.x, 100));
    cpBodyApplyImpulseAtLocalPoint(mBody, cpv(0, -60), cpv(0, -BULLET_HEIGHT/2));
    if (mInitialAngle < M_PI) {
        cpBodySetAngularVelocity(mBody, 1);
    } else {
        cpBodySetAngularVelocity(mBody, -1);
    }
}

void Bullet::render(SDL_Renderer * r) {
    cpVect pos = cpBodyGetPosition(mBody);
    cpVect cog = cpBodyGetCenterOfGravity(mBody);
    cpFloat angle = cpBodyGetAngle(mBody);
    //cpShape *shape = mBody->shapeList;
    //cpBody * b = shape->body;
    if (!mNoRender) {
        img.render(r, (int)pos.x - cog.x, (int)pos.y - cog.y, NULL ,(double)rad2deg(angle), NULL, SDL_FLIP_NONE);
    }
}

void Bullet::move() {
    //mBody->p.x += mVelX;
    //mBody->p.y += mVelY;
    //cpBodySetAngle(mBody, cpvtoangle(cpv(mVelX, mVelY)));
    if ((cpBodyGetAngle(mBody) > M_PI) xor (mInitialAngle > M_PI)) {
        cpBodySetAngle(mBody, M_PI);
    }
    cpVect v = cpBodyGetVelocity(mBody);
    if (abs(v.y) <= 10) {
        cpBodySetVelocity(mBody, cpv(v.x, 100));
    }

    if(cpvdist(cpBodyGetPosition(mBody), cpv(mPosX, mPosY)) > mRange) {
       destroy();
    }
}

}//Ns end

