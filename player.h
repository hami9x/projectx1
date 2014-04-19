#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include "texture.h"

class XxPlayer
{
    public:

        //Maximum velocity
        static const double PLAYER_VEL=3;
        static const int PLAYER_RAD=3;
        static const double PI=3.14159265;

        //Initializes the variables
        XxPlayer();

        //Deallocated Memory
        ~XxPlayer();

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
        XxTexture aircraft;

        //Rotation
        void left();
        void right();

        //check if right button press or not
        bool press;
};

#endif // PLAYER_H_INCLUDED
