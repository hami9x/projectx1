#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

#include <string>
#include <SDL.h>
#include "texture.h"
#include <chipmunk_private.h>

static const std::string BULLET_IMG = "bullet.png";
static const int BULLET_SPEED = 3;
//Explosion animation
static const int EXPLOTION_ANIMATION_FRAME = 5;
static SDL_Rect gSpriteClips[ EXPLOTION_ANIMATION_FRAME ];

namespace xx {

class Bullet {
    public:
        void getPlayerVel(double pVelX, double pVelY );
        void getPlayerPos(cpFloat pPosX, cpFloat pPosY, cpFloat pAngle, cpFloat pX);
        bool checkExist() { return exist; }
        bool checkExpl() { return Hit; }
        //Initializes the variables
        Bullet();

        //Deallocate memory
        ~Bullet();

        //Free
        void free();

        //Create bullet
        void createBullet( SDL_Renderer *r, cpSpace *space, cpDataPointer n, double range );

        //Move the bullet
        void moveBullet();

        //return body
        cpBody *body() { return mBody; }
        //render
        void render(SDL_Renderer *r);
        //explosion render
        void eRender(SDL_Renderer *r);
        //Set up explosion
        void explosion(cpFloat x, cpFloat y);
    private:
        //Velocity of bullet
        double mVelX,mVelY;

        //Bullet texture
        Texture img;
        //Explosion sprite
        Texture explSprite;
        cpFloat ePosX, ePosY;

        //Bullet properties
        double mRange;//maximum range
        double fRange;//flying range
        bool exist;
        cpBody *mBody;
        cpFloat mPosX, mPosY,mX;
        cpFloat mAngle;
        cpBool Hit;
        int frame;

        //SDL_Renderer
        SDL_Renderer *mR;
};
}
#endif // BULLET_H_INCLUDED
