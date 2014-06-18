/* Some of the code are originally written by LazyFoo Production. */
//Using SDL, SDL_image, SDL_ttf, standard IO, math, and strings
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <thread>
#include <SDLU.h>
#include <enet/enet.h>
#include "tmxparser.h"
#include "ChipmunkDebugDraw.h"
#include "global.h"
#include "utils.h"
#include "text.h"
#include "texture.h"
#include "entity.h"
#include "player.h"
#include "bullet.h"
#include "client.h"
#include "syncer.h"
#include "skill.h"
#include "proto/player.pb.h"
#include "proto/clientinfo.pb.h"
#include "animation.h"

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
        mDefFont = TTF_OpenFont( "NovaSquare.ttf", 20 );
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

void drawPlayerHp(Player & p, SDL_Renderer* mRenderer,int x,int y,TTF_Font *mFont){
    char num[100]="HP: ",temp[100]="";
    SDL_Rect fillRect = { x, y, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 20 };
    SDL_SetRenderDrawColor( mRenderer, 0x99,0x33,0x66, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { x+4, y+4, SCREEN_WIDTH / 3 - 8, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF,0xDD,0xFF, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { x+4, y+4, (SCREEN_WIDTH/3 - 8)*(double)(p.hp)/p.maxhp, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF, 0x99, 0xCC, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );
    sprintf(temp,"%d",p.hp);
    strcat(num,temp);
    strcat(num,"/");
    sprintf(temp,"%d",p.maxhp);
    strcat(num,temp);
    Text hptxt(num,mFont, {94,19,83});
    hptxt.render(mRenderer,x+10,y+35,200);
}
void loadIcon(std::string path,int x , int y, SDL_Renderer *renderer){
    Texture icon;
    icon.loadFromFile(renderer,path.c_str());
    icon.render(renderer,x,y);
}
void drawCoolDown(int x, int y, SDL_Renderer * renderer, Skillmanager* sManager , int skillNum){
    if (sManager->cdCheck(skillNum) < 1) {
        Texture cooldownBox;
        cooldownBox.loadFromFile(renderer,"cooldown.png");
        cooldownBox.setBlendMode(SDL_BLENDMODE_BLEND);
        cooldownBox.setAlpha(100);
        SDL_Rect cdBox = {0,0,30-30*sManager->cdCheck(skillNum),30};
        cooldownBox.render(renderer,x,y,&cdBox);
    }
}
void drawFPSInfo(SDL_Renderer * renderer, int fps, int x, int y, TTF_Font *font) {
    char tx[50]; sprintf(tx, "FPS: %d", fps);
    Text fpsTxt(tx, font, {94,19,83});
    fpsTxt.render(renderer, x, y, 300);
}

void enet_init() {
    if (enet_initialize () != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        exit(EXIT_FAILURE);
    }
    atexit(enet_deinitialize);
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

    int start() {
        //! Network setup
        enet_init();
        Client client;
        client.setOfflineMode();
        client.connect("localhost", 1000);
        int playerId = client.playerId();

        //! Physics setup
        Physics physics(17);
        physics.setupCollisions();

        //! Load objects
        TmxMap m;
        TmxReturn error = tmxparser::parseFromFile("map.tmx", &m);
        if (error != TmxReturn::kSuccess) {
            printf("Tmx parse error. Code %d.\n", error);
        }
        Skillmanager sManager;
        Texture plImg;
        plImg.loadFromFile(mRenderer, "aircraft.png");
        EntityCollection players = Entity::fromTmxGetAll("planes", "aircraft", &m, 0, &plImg, physics.space());
        Texture clImg;
        clImg.loadFromFile(mRenderer, "clouds.png");
        EntityCollection clouds = Entity::fromTmxGetAll("clouds", "clouds", &m, 0, &clImg, physics.space());
        //Trap mouse to screen center
//        SDL_WarpMouseInWindow(mWindow, SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
//        SDL_SetRelativeMouseMode(SDL_TRUE);
        //set player1
        Player p1(players[playerId-1]);
        //Assume we have 2 player only, this could be change later.
        Player p2(players[2-playerId]);

        ExplosionCreate(mRenderer);

        cpVect mvVect = cpvzero;

        //! Client-server syncer
        Syncer syncer(&client, &p1, &p2);
        //syncer.start(&mvVect);
        bool stopped = false;
        Update update;
        bool updated = false;
        std::thread ut1(&Syncer::playerHostSync, syncer, &stopped, std::ref(update), std::ref(updated));
        std::thread ut2(&Syncer::playerSendUpdate, syncer, &stopped, &mvVect);

        //!
        //
        //explosionPrepare(mRenderer);
        //Debug Draw
        ChipmunkDebugDrawInit();
        //SDL_RenderPresent(mRenderer);

        glClearColor(1, 1, 1, 1);

        //!!!WARNING: Main loop area
        //!! Khong phan su mien vao :-s
        bool quit = false;
        SDL_Event e;
        utils::Timer fTimer(0), fpsTimer(300);
        int fps;
        utils::Timer fireTimer(1000);

        //While application is running
        while( !quit )
        {
            uint32 ftime = fTimer.elapsed();
            if (ftime > 0 && ftime < 17) {
                Sleep(17-ftime);
                ftime=17;
            }
            //! Physics integration
            physics.step(ftime);
            //!
            fTimer.reset();

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
                    p1.handleEvent(e, mRenderer, physics.space(), &sManager, mvVect);
                }
            }
            p1.rightPressCheck(mvVect);

            //Firing
            p1.handleFire(mRenderer, physics.space(), fireTimer, cpBodyGetAngle(p1.body()));

            //Move the aircraft
            p1.updateState();
            p1.renderBullets(mRenderer);

            syncer.updateBodies(&physics, update, updated);

            //!!! Rendering Area
            //!!  Khong phan su mien vao :v
            //Clear screen
            //SDL_SetRenderDrawColor( mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
            //SDL_RenderClear( mRenderer );

            Entity::renderAll(players, mRenderer);

            //TODO: split explosion into class, this explosionCheck from nowhere is...
            //explosionCheckRender(mRenderer);
            Entity::renderAll(clouds, mRenderer);

            drawPlayerHp(p1, mRenderer,0,0,assets.defFont());
            drawPlayerHp(p1, mRenderer,666,0,assets.defFont());

            if (ftime && fpsTimer.exceededReset()) {
                fps = 1000/ftime;
            }
            drawFPSInfo(mRenderer, fps, 20, 20, assets.defFont());
            loadIcon("push.jpg", 5 , 65 , mRenderer);
            drawCoolDown(5 ,65 ,mRenderer , &sManager ,1 );
            //Demo draw from SDL2_gfx
            //filledPieRGBA(mRenderer, 200, 200, 200, 0, 70, 100, 200, 100, 255);

            SDL_RenderPresent(mRenderer);

            SDLU_GL_RenderCacheState(mRenderer);
            glClear(GL_COLOR_BUFFER_BIT);
            ChipmunkDebugDrawPushRenderer();
            PerformDebugDraw(physics.space());
            ChipmunkDebugDrawFlushRenderer();
            ChipmunkDebugDrawPopRenderer();
            SDLU_GL_RenderRestoreState(mRenderer);
            ExplosionRender(mRenderer);
            //!!! End Rendering Area
        }

        stopped = true;
        Sleep(300);
        ut1.join();
        ut2.join();

        ChipmunkDebugDrawCleanup();
        p1.free();
        p2.free();

        Entity::freeAll(players, physics.space());
        Entity::freeAll(clouds, physics.space());
        physics.free();

        return 0;
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
    //SDL_ShowCursor(0);
    return app.start();
}
