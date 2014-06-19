#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include "global.h"
#include "entity.h"
#include "utils.h"
#include "bullet.h"
#include "skill.h"
#include "proto/player.pb.h"

const cpFloat PLAYER_RAD = 0.05f;
namespace xx {

class Player {
    public:

        //Maximum velocity
        constexpr static double PLAYER_VEL=30;

        constexpr static int sWidth=SCREEN_WIDTH;
        constexpr static int sHeight=SCREEN_HEIGHT;

        //Default constructor
        Player():
            mEntity(NULL),
            Rpressed(false), Lpressed(false),
            mFiredNumber(0)
        {}

        //Initializes the variables
        Player(Entity *);

        //Deallocated Memory
        ~Player();

        //render
        void render(SDL_Renderer *r);

        //Event Handler
        void handleEvent(SDL_Event e, SDL_Renderer *r, cpSpace *space, Skillmanager *sManager, cpVect & moveVect);

        void handleFire(SDL_Renderer *r, cpSpace *space, utils::Timer & fireTimer, cpFloat fireAngle, bool isclient);

        void rightPressCheck(cpVect & moveVect);

        void setInCloud(int);

        //Move the aircraft
        void updateState();
        void renderBullets(SDL_Renderer *r);

        //body
        inline cpBody *body() { if (mEntity != NULL) return mEntity->body(); else return NULL; }

        //Hurt
        void hurt(int dam);

        //free bullets
        void free();

        //void setMove(cpVect vel) { mVel = vel; }

        cpVect vectorForward();

        //in Cloud
        int mInCloud=0;

        //Getter
        inline double velX() { return sin( cpBodyGetAngle( mEntity->body() ) )*PLAYER_VEL; }
        inline double velY() { return -cos( cpBodyGetAngle( mEntity->body() ) )*PLAYER_VEL; }
        inline cpFloat posX() { return mEntity->body()->p.x; }
        inline cpFloat posY() { return mEntity->body()->p.y; }
        inline cpFloat angle() { return cpBodyGetAngle(mEntity->body()); }
        //cpFloat sensor() { return mEntity->sprite().height()/2; }
        inline int firedNumber() { return mFiredNumber; }
        inline cpFloat firedAngle(int i) { return mFiredAngle[i]; }
        inline void setMove(cpVect v) { mVel = v; }
        inline void updateReset() { mFiredNumber = 0;}
        inline int hp() { return mHp; }
        inline void setHp(int hp) { mHp = hp; }
        inline int maxHp() { return mMaxHp; }

        static const int mMaxAmmo;
        static const int MAX_FIREANGLES;

    private:
        //Velocity of player
        cpVect mVel;

        //Entity
        Entity *mEntity;

        //Rotation
        void rotLeft();
        void rotRight();

        //check if button press or not
        bool Rpressed, Lpressed;

        int mFiredNumber;

        //Maximum bullet
        Bullet mAmmo[10];

        //Memory fired ammo
        cpFloat mFiredAngle[30];

        cpVect mVectp;

        //HP
        int mHp;
        int mMaxHp;

        utils::Timer mHpLossTimer;

};

}

#endif // PLAYER_H_INCLUDED
