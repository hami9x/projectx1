#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

#include <string>
#include <SDL.h>
#include "texture.h"
#include <chipmunk_private.h>


static const std::string BULLET_IMG = "bullet.png";
static const int BULLET_SPEED = 6;


namespace xx {

class Player;

class Bullet {
    public:
        bool checkExist() { return exist; }
        //Initializes the variables
        Bullet();

        //Deallocate memory
        ~Bullet();

        //Free
        void free();

        //Create bullet
        void createBullet( SDL_Renderer *r, cpSpace *space, double range, cpDataPointer p );

        //Move the bullet
        void moveBullet();

        void destroy();

        //return body
        cpBody * body() { return mBody; }
        Player * player() { return mPlayer; }

        //render
        void render(SDL_Renderer *r);
    private:
        //Velocity of bullet
        double mVelX,mVelY;

        //Bullet texture
        Texture img;

        //Bullet properties
        double mRange;//maximum range
        double fRange;//flying range
        bool exist;
        cpBody *mBody;
        cpFloat mPosX, mPosY,mX;
        cpFloat mAngle;
        Player *mPlayer;
        bool loadedImg;
};
}
#endif // BULLET_H_INCLUDED
