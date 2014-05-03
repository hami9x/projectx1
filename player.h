#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include "global.h"
#include "entity.h"
#include "texture.h"
#include "bullet.h"
#include "proto/player.pb.h"

const cpFloat PLAYER_RAD = 0.05f;
namespace xx {

class Player {
    public:

        //Maximum velocity
        constexpr static double PLAYER_VEL=10;

        constexpr static int sWidth=SCREEN_WIDTH;
        constexpr static int sHeight=SCREEN_HEIGHT;

        //Default constructor
        Player() {}

        //Initializes the variables
        Player(Entity *);

        //Deallocated Memory
        ~Player();

        //render
        void render(SDL_Renderer *r);

        //Event Handler
        void handleEvent(SDL_Event e, SDL_Renderer *r, cpSpace *space, PlayerChange *pc);

        void handleFire(SDL_Renderer *r, cpSpace *space, cpFloat &time);

        //Move the aircraft
        void fly();


        //HP
        int hp=10000;
        int maxhp=10000;

        //body
        cpBody *body() { return mEntity->body(); }

        //free bullet
        void freeBullet(Bullet a);

        //Hurt
        void hurt(int dam);

        void setMove(cpVect vel) { mVel = vel; }

        cpVect vectorForward();

        //in Cloud
        int inCloud=0;

        //Getter
        double velX() { return sin( cpBodyGetAngle( mEntity->body() ) )*PLAYER_VEL; }
        double velY() { return -cos( cpBodyGetAngle( mEntity->body() ) )*PLAYER_VEL; }
        cpFloat posX() { return mEntity->body()->p.x; }
        cpFloat posY() { return mEntity->body()->p.y; }
        cpFloat angle() { return mAngle; }
        cpFloat sensor() { return mEntity->sprite().height()/2; }

    private:
        //Velocity of player
        cpVect mVel;

        //Entity
        Entity *mEntity;

        //Aircraft texture
        Texture aircraft;

        //Rotation
        void rotLeft();
        void rotRight();
        cpFloat mAngle;

        //check if button press or not
        bool Rpressed,Lpressed;

        //Maximum bullet
        int maxAmmo;
        Bullet ammo[10];
};

}

#endif // PLAYER_H_INCLUDED
