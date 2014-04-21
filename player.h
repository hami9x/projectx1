#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include "global.h"
#include "entity.h"
#include "texture.h"

namespace xx {

class Player {
    public:

        //Maximum velocity
        constexpr static double PLAYER_VEL=20;
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
        void handleEvent(SDL_Event e);

        //Move the aircraft
        void fly();

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

        //check if right button press or not
        bool pressed;
};

}

#endif // PLAYER_H_INCLUDED
