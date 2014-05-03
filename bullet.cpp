#include "bullet.h"

#include "texture.h"
#include "global.h"
#include <string>
#include <SDL.h>
#include <chipmunk_private.h>
#include <stdio.h>
#include <math.h>

namespace xx {

Bullet::Bullet() {
    mVelX=5;
    mVelY=5;
    mRange=0;
    exist=false;
    frame=0;
    Hit=false;
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

void Bullet::createBullet(SDL_Renderer *r, cpSpace *space, cpDataPointer n, double range) {

    mR=r;
    img.loadFromFile(r,BULLET_IMG);
    exist = true;
    mRange = range;
    mBody = cpBodyNew(10.f, cpMomentForBox(10.f, img.width(), img.height()));
    mBody->p.x = mPosX;
    mBody->p.y = mPosY;

    printf("%d \n",mBody->userData);

    cpBodySetCenterOfGravity(mBody, cpv(img.width()/2, img.height()/2));
    cpShape *shape;

    shape = cpBoxShapeNew(mBody, img.width(), img.height(), 0);
    cpShapeSetCollisionType(shape, BULLET_TYPE);
    cpShapeSetSensor(shape, true);
    cpSpaceAddShape(space, shape);
    cpSpaceAddBody(space, mBody);

    cpBodySetAngle(mBody,mAngle);
    cpBodySetUserData(mBody, n);
}

void Bullet::getPlayerVel( double pVelX, double pVelY ) {
    mVelX = pVelX;
    mVelY = pVelY;
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

void Bullet::getPlayerPos(cpFloat pPosX, cpFloat pPosY, cpFloat pAngle, cpFloat pX){
    mPosX = pPosX;
    mPosY = pPosY;
    mX = pX;
    mAngle = pAngle;
}

void Bullet::moveBullet() {
    mBody->p.x += mVelX;
    mBody->p.y += mVelY;
    cpBodyApplyForceAtLocalPoint(mBody, cpv(mVelX, mVelY), cpv(0, 0));
}

void Bullet::explosion(cpFloat x, cpFloat y) {

    if ( explSprite.loadFromFile(mR, "explosion.png") )
    {
        //Set sprite clips
        gSpriteClips[ 0 ].x =   0;
        gSpriteClips[ 0 ].y =   0;
        gSpriteClips[ 0 ].w =  118;
        gSpriteClips[ 0 ].h = 118;

        gSpriteClips[ 1 ].x =   118;
        gSpriteClips[ 1 ].y =   0;
        gSpriteClips[ 1 ].w =  118;
        gSpriteClips[ 1 ].h = 118;

        gSpriteClips[ 2 ].x =   236;
        gSpriteClips[ 2 ].y =   0;
        gSpriteClips[ 2 ].w =  118;
        gSpriteClips[ 2 ].h = 118;

        gSpriteClips[ 3 ].x =   354;
        gSpriteClips[ 3 ].y =   0;
        gSpriteClips[ 3 ].w =  118;
        gSpriteClips[ 3 ].h = 118;

        gSpriteClips[ 4 ].x =   472;
        gSpriteClips[ 4 ].y =   0;
        gSpriteClips[ 4 ].w =  118;
        gSpriteClips[ 4 ].h = 118;

        Hit=true;
        ePosX = x;
        ePosY = y;
        frame = 0;
    }
}
void Bullet::eRender(SDL_Renderer *r) {
        //Render current frame
        SDL_Rect* currentClip = &gSpriteClips[ frame/10 ];
        explSprite.render(r, (int)ePosX, (int)ePosY, currentClip, 0, NULL, SDL_FLIP_NONE);
        frame++;
        if( frame > 100 )
        {
            explSprite.free();
            Hit = false;
        }
}
}
