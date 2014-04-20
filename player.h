#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include "texture.h"

namespace xx {

class Player
{
    public:

        //Maximum velocity
        constexpr static double PLAYER_VEL=3;
        constexpr static int PLAYER_RAD=3;
        constexpr static double PI=3.14159265;

        //Initializes the variables
        Player();

        //Deallocated Memory
        ~Player();

        //Load image
        void getAvatar(SDL_Renderer *r,char s[]);

        //render
        void render(SDL_Renderer *r);

        //Event Handler
        void handleEvent(SDL_Event e);

        //Move the aircraft
        void fly();

        void getScreenSize(int a, int b);

    private:
        //Player position
        int mPosX, mPosY;

        //Screen height and width
        int sHeight,sWidth;

        //Velocity of player
        double mVelX, mVelY;

        //Angle use for rotation
        double rdegrees;

        //Angle use for calculate velocity
        double mdegrees;

        //Aircraft texture
        Texture aircraft;

        //Rotation
        void left();
        void right();

        //check if right button press or not
        bool press;
};

}

#endif // PLAYER_H_INCLUDED
