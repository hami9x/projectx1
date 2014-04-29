#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include <SDL_ttf.h>
#include "global.h"
#include "entity.h"
#include "texture.h"
#include "bullet.h"

const cpFloat PLAYER_RAD = 0.05f;
namespace xx {

class Player {
    public:

        //Maximum velocity
        constexpr static double PLAYER_VEL=10;

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
        int hp=10000;
        int maxhp=10000;

        //DRAW HPBAR
        void drawHp(SDL_Renderer* mRenderer,int x,int y,TTF_Font *mFont);

        //body
        cpBody *body()
        {
            return mEntity->body();
        }
        //Hurt
        void hurt(int dam);

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
