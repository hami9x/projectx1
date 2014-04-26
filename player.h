#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include "global.h"
#include "entity.h"
#include "texture.h"
#include "bullet.h"
namespace xx {

class Player {
    public:

        //Maximum velocity
        constexpr static double PLAYER_VEL=10;
        constexpr static int PLAYER_RAD=5;
        constexpr static double PI=3.14159265;

        constexpr static int sWidth=SCREEN_WIDTH;
        constexpr static int sHeight=SCREEN_HEIGHT;

        //Initializes the variables
        Player(Entity *);

        //Deallocated Memory
        ~Player();

        //render
        void render(SDL_Renderer *r);

        //Event Handler
        void handleEvent(SDL_Event e, SDL_Renderer *r, cpSpace *space);

        //Move the aircraft
        void fly();


        //HP
        int hp;
        int maxhp;

        //DRAW HPBAR
        void drawHp(SDL_Renderer* mRenderer,int x,int y);

        //body
        cpBody *body()
        {
            return mEntity->body();
        }

    private:
        //Velocity of player
        double mVelX, mVelY;

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

        Bullet ammo[11];
};

}

#endif // PLAYER_H_INCLUDED
