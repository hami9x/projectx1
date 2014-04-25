/* Some of the code are originally written by LazyFoo Production. */
//Using SDL, SDL_image, SDL_ttf, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDLU.h>
#include "tmxparser.h"
#include "ChipmunkDebugDraw.h"

#include "global.h"
#include "text.h"
#include "texture.h"
#include "entity.h"
#include "player.h"

using namespace std;
using namespace tmxparser;
using namespace xx;

//Class to manage asset objects
class Assets {
    TTF_Font *mDefFont;

    public:

    TTF_Font *defFont() { return mDefFont; }

    Assets(): mDefFont(NULL)
    {
    }

    bool load()
    {
        //Open the font
        mDefFont = TTF_OpenFont( "NovaSquare.ttf", 28 );
        if( mDefFont == NULL )
        {
            printf( "Failed to load thefont! SDL_ttf Error: %s\n", TTF_GetError() );
            return false;
        }

        return true;
    }

    void teardown()
    {
        printf("Shut down assets.\n");
        TTF_CloseFont( mDefFont );
        mDefFont = NULL;
    }
};

//COLLISION HANDLER
static cpBool
beginFunc(cpArbiter *arb, cpSpace *space, cpDataPointer unused)
{
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    if( cpShapeGetCollisionType(a) == PLANE_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Plane hit Cloud\n");
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Bullet hit Cloud\n");

    return 0;
}

static void
separateFunc (cpArbiter *arb, cpSpace *space, void *unused)
{
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    if( cpShapeGetCollisionType(a) == PLANE_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Plane separate Cloud\n");
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Bullet separate Cloud\n");
}

void collision(int type_a, int type_b, cpSpace *space)
{
    //collision
    cpCollisionHandler * handler = cpSpaceAddCollisionHandler(space, type_a, type_b);
    handler->beginFunc = beginFunc;
    handler->separateFunc = separateFunc;
}

class Application {
    //The window we'll be rendering to
    SDL_Window* mWindow;

    //The window renderer
    SDL_Renderer* mRenderer;

    Assets assets;

    public:
    Application(): mWindow(NULL), mRenderer(NULL)
    {
    }

    ~Application()
    {
        teardown();
    }

    void start() {
        // cpVect is a 2D vector and cpv() is a shortcut for initializing them.
        cpVect gravity = cpv(0, 0);
        // Create an empty space.
        cpSpace *space = cpSpaceNew();
        cpSpaceSetGravity(space, gravity);
        cpSpaceSetDamping(space, 0.5);

        TmxMap m;
        TmxReturn error = tmxparser::parseFromFile("map.tmx", &m);
        if (error != TmxReturn::kSuccess) {
            printf("Tmx parse error. Code %d.\n", error);
        }

        Texture plImg;
        plImg.loadFromFile(mRenderer, "aircraft.png");
        EntityCollection players = Entity::fromTmxGetAll("planes", "aircraft", &m, 0, &plImg, true, space);
        Entity::addAll(players, space);
        Texture clImg;
        clImg.loadFromFile(mRenderer, "clouds.png");
        EntityCollection clouds = Entity::fromTmxGetAll("clouds", "clouds", &m, 0, &clImg, true, space);
        Entity::addAll(clouds, space);
        //Trap mouse to screen center
        SDL_WarpMouseInWindow(mWindow, SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        //set player1
        Player p1(players[0]);

        ChipmunkDebugDrawInit();
        SDL_RenderPresent(mRenderer);

        int n=0;

        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        cpFloat timeStep = 1.0/60.0;

        glClearColor(1, 1, 1, 1);

        //While application is running
        while( !quit )
        {
            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                else
                {
                    p1.handleEvent(e, mRenderer, space);
                }

            }

            //Move the aircraft
            p1.fly();

            //collision
            collision(PLANE_TYPE, CLOUD_TYPE, space);
            collision(BULLET_TYPE, CLOUD_TYPE, space);
            collision(PLANE_TYPE, BULLET_TYPE, space);

//            //Clear screen
//            SDL_SetRenderDrawColor( mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
//            SDL_RenderClear( mRenderer );

//            Render aircraft
            Entity::renderAll(players, mRenderer);
            Entity::renderAll(clouds, mRenderer);

            //Add HPbar
//            p1.hp=23; p1.maxhp=100;
//            p1.drawHp(mRenderer,0,0);
//            p1.drawHp(mRenderer,666,0);
            //Update screen
            cpSpaceStep(space, timeStep);

            SDL_RenderPresent(mRenderer);
            SDLU_GL_RenderCacheState(mRenderer);
            glShadeModel(GL_SMOOTH);
            glClear(GL_COLOR_BUFFER_BIT);
            ChipmunkDebugDrawPushRenderer();
            PerformDebugDraw(space);
            ChipmunkDebugDrawFlushRenderer();
            ChipmunkDebugDrawPopRenderer();
            glShadeModel(GL_FLAT);      /* restore state */
            SDLU_GL_RenderRestoreState(mRenderer);
        }
        ChipmunkDebugDrawCleanup();

        cpSpaceFree(space);
    }

    bool loadExtensions()
    {
        //Initialize PNG loading
        int imgFlags = IMG_INIT_PNG;
        if( !( IMG_Init( imgFlags ) & imgFlags ) )
        {
            printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
            return false;
        }

         //Initialize SDL_ttf
        if( TTF_Init() == -1 )
        {
            printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
            return false;
        }

        return true;
    }

    bool init()
    {
        //Load media
        if( !loadExtensions() )
        {
            printf( "Failed to load extensions!\n" );
            return false;
        }

        //Load media
        if( !assets.load() )
        {
            printf( "Failed to load assets!\n" );
            return false;
        }

        SDL_SetHintWithPriority("SDL_RENDER_DRIVER", "opengl", SDL_HINT_OVERRIDE);

        //Initialize SDL
        if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        {
            printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
            return false;
        }

        //Set texture filtering to linear
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }

        //Create mWindow
        mWindow = SDL_CreateWindow( "Project X1: Mini aeroPlant", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
        if( mWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            return false;
        }

        //Create vsynced renderer for mWindow
        mRenderer = SDL_CreateRenderer( mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
        if( mRenderer == NULL )
        {
            printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
            return false;
        }

        //Initialize renderer color
        SDL_SetRenderDrawColor( mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

        return true;
    }

    void teardown()
    {
        printf("shut down app.\n");
        SDL_DestroyRenderer( mRenderer );
        SDL_DestroyWindow( mWindow );
        mWindow = NULL;
        mRenderer = NULL;

        assets.teardown();
    	//Quit SDL subsystems
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
};


int main( int argc, char* args[] )
{
    Application app;

	//Start up SDL and create window
	if( !app.init() )
	{
		printf( "Failed to initialize!\n" );
		return 1;
	}

    //Hide cursor
    SDL_ShowCursor(0);
    app.start();

	return 0;
}
