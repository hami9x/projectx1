#include "bullet.h"

#include "texture.h"
#include "global.h"
#include <string>
#include <SDL.h>
#include <chipmunk_private.h>
#include <stdio.h>

namespace xx {

Bullet::Bullet() {
    mVelX=5;
    mVelY=5;
    mRange=0;
    exist=false;
}

Bullet::~Bullet() {
    if (exist) {
        cpBodyFree(mBody);
    }
    mRange = 0;
}

void Bullet::createBullet(SDL_Renderer *r, cpSpace *space, double range) {
    printf("^^");
    img.loadFromFile(r,BULLET_IMG);
    exist = true;
    mRange = range;
    mBody = cpBodyNew(10.f, cpMomentForBox(10.f, img.width(), img.height()));
    mBody->p.x = mPosX;
    mBody->p.y = mPosY;
    cpShape *shape;

    shape = cpBoxShapeNew(mBody, img.width(), img.height(), img.height()/2);
    cpShapeSetCollisionType(shape, BULLET_TYPE);
    cpSpaceAddShape(space,shape);
    cpShapeCacheBB(shape);
    cpSpaceAddBody(space,mBody);

    printf("%f %f\n",mBody->p.x, mBody->p.y);
}

void Bullet::getPlayerVel( double pVelX, double pVelY ) {
    mVelX = pVelX;
    mVelY = pVelY;
}

void Bullet::render(SDL_Renderer * r) {
    cpVect pos = cpBodyGetPosition(mBody);
    img.render(r, (int)pos.x, (int)pos.y, NULL ,mAngle, NULL, SDL_FLIP_NONE);
}

void Bullet::getPlayerPos(cpFloat pX, cpFloat pY, cpFloat pAngle){
    mPosX = pX;
    mPosY = pY;
    mAngle = pAngle;
}

void Bullet::moveBullet() {
    mBody->p.x += mVelX;
    mBody->p.y += mVelY;
    cpBodyApplyForceAtLocalPoint(mBody, cpv(mVelX, mVelY), cpv(0, 0));
}

}
