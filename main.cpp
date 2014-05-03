/* Some of the code are originally written by LazyFoo Production. */
//Using SDL, SDL_image, SDL_ttf, standard IO, math, and strings
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDLU.h>
#include <enet/enet.h>
#include "tmxparser.h"
#include "ChipmunkDebugDraw.h"
#include "global.h"
#include "text.h"
#include "texture.h"
#include "entity.h"
#include "player.h"
#include "bullet.h"
#include "proto/player.pb.h"
#include "proto/clientinfo.pb.h"

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

//Post-step: Ammo free
static void
ammoFree( cpSpace *space, cpShape *shape, void *unused)
{
    cpBody *Body = cpShapeGetBody(shape);
    Bullet *ammo = (Bullet*)cpBodyGetUserData(Body);
    cpVect pos = cpBodyGetPosition(Body);
    printf("body->p: %f %f \n", Body->p.x, Body->p.y);
    printf("Posion: %f %f \n",pos.x,pos.y);
    ammo->explosion(Body->p.x - 59, Body->p.y - 59);
    ammo->free();
}
//COLLISION HANDLER
int inCloud=0;
static cpBool
beginFunc(cpArbiter *arb, cpSpace *space, cpDataPointer unused)
{
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    if( cpShapeGetCollisionType(a) == PLANE_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        inCloud+=1;
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Bullet hit Cloud\n");
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == PLANE_TYPE )
    {
        cpBool check = cpShapeGetSensor(a);
        if(!check){
            printf("Hit plane\n");

            cpSpaceAddPostStepCallback( space, (cpPostStepFunc)ammoFree, a, NULL);
        }
    }

    return 0;
}

static void
separateFunc (cpArbiter *arb, cpSpace *space, void *unused)
{
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    if( cpShapeGetCollisionType(a) == PLANE_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
       inCloud-=1;

    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Bullet separate Cloud\n");
}

void collision(int type_a, int type_b, cpSpace *space,Player *pl)
{
    //collision
    cpCollisionHandler * handler = cpSpaceAddCollisionHandler(space, type_a, type_b);
    handler->beginFunc = beginFunc;
    handler->separateFunc = separateFunc;
    if (inCloud!=0) (*pl).hurt(inCloud);
}

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
        if (enet_initialize () != 0)
        {
            fprintf (stderr, "An error occurred while initializing ENet.\n");
            exit(EXIT_FAILURE);
        }
        atexit (enet_deinitialize);

        ENetHost * client;
        client = enet_host_create(NULL /* create a client host */,
                    1 /* only allow 1 outgoing connection */,
                    2 /* allow up 2 channels to be used, 0 and 1 */,
                    0 /* 56K modem with 56 Kbps downstream bandwidth */,
                    0 /* 56K modem with 14 Kbps upstream bandwidth */);
        if (client == NULL)
        {
            fprintf(stderr,
                     "An error occurred while trying to create an ENet client host.\n");
            exit(EXIT_FAILURE);
        }

        ENetAddress address;
        ENetEvent event;
        ENetPeer *host;
        enet_address_set_host (&address, "localhost");
        address.port = 5555;
        /* Initiate the connection, allocating the two channels 0 and 1. */
        host = enet_host_connect (client, &address, 2, 0);
        if (host == NULL)
        {
           fprintf (stderr,
                    "No available peers for initiating an ENet connection.\n");
           exit (EXIT_FAILURE);
        }

        if (enet_host_service (client, & event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            printf ("Connection to localhost:5555 succeeded.\n");
        }
        else
        {
            /* Either the 5 seconds are up or a disconnect event was */
            /* received. Reset the peer in the event the 5 seconds   */
            /* had run out without any significant event.            */
            enet_peer_reset (host);
            printf ("Connection to localhost:5555 failed.\n");
            return 1;
        }

        int playerId = -1;
        enet_host_service (client, & event, 5000);
        if (event.type == ENET_EVENT_TYPE_RECEIVE && event.channelID == 0 && event.packet->data != NULL) {
            string message((char*)event.packet->data);
            ClientInfo ci;
            ci.ParseFromString(message);
            playerId = ci.player();
            enet_packet_destroy (event.packet);
        } else {
            printf("No information received from host.\n");
            return 1;
        }

        if (playerId == 0) {
            printf("Unhandled.");
            return 1;
        }

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
        EntityCollection players = Entity::fromTmxGetAll("planes", "aircraft", &m, 0, &plImg, space,5, PLANE_TYPE);
        Texture clImg;
        clImg.loadFromFile(mRenderer, "clouds.png");
        EntityCollection clouds = Entity::fromTmxGetAll("clouds", "clouds", &m, 0, &clImg, space,1000, CLOUD_TYPE);
        //Trap mouse to screen center
        SDL_WarpMouseInWindow(mWindow, SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        //set player1
        Player p1(players[playerId-1]);

        ChipmunkDebugDrawInit();
        SDL_RenderPresent(mRenderer);

        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        cpFloat timeStep = 1.0/60.0;
        cpFloat updateInterval = 1.0/20.0;
        cpFloat updateTime = 0;
        cpFloat fireTime = 0;

        glClearColor(1, 1, 1, 1);

        PlayerChange pc;

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
                    p1.handleEvent(e, mRenderer, space, &pc);
                }
            }
            //Firing
            fireTime += timeStep;
            updateTime += timeStep;
            if (updateTime >= updateInterval) {
                updateTime -= updateInterval;
                pc.set_time(now());
                string message = pc.SerializeAsString();
                ENetPacket * packet = enet_packet_create(message.c_str(), message.size()+1, 0);
                enet_peer_send(host, 1, packet);
                pc.Clear();
            }
            p1.handleFire(mRenderer, space, fireTime);

            enet_host_service (client, &event, 0);

            if (event.type == ENET_EVENT_TYPE_RECEIVE) {

            }

            //Move the aircraft
            p1.fly();

            //collision

            collision(PLANE_TYPE, CLOUD_TYPE, space,&p1);
            collision(BULLET_TYPE, CLOUD_TYPE, space,&p1);
            collision(BULLET_TYPE, PLANE_TYPE, space,&p1);

//            //Clear screen
//            SDL_SetRenderDrawColor( mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
//            SDL_RenderClear( mRenderer );

//            Render aircraft
            Entity::renderAll(players, mRenderer);

            p1.render(mRenderer);
//            Entity::renderAll(clouds, mRenderer);

            Entity::renderAll(clouds, mRenderer);


            cpSpaceStep(space, timeStep);
            drawPlayerHp(p1, mRenderer,0,0,assets.defFont());
            drawPlayerHp(p1, mRenderer,666,0,assets.defFont());
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
        Entity::freeAll(players, space);
        Entity::freeAll(clouds, space);

        cpSpaceFree(space);
        enet_host_destroy(client);
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
    return app.start();
}
