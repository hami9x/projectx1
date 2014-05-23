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
#include "ops.h"
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
                    10,
                    0,
                    0);
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
        host = enet_host_connect (client, &address, 10, 0);
        if (host == NULL)
        {
           fprintf (stderr,
                    "No available peers for initiating an ENet connection.\n");
           exit (EXIT_FAILURE);
        }

        if (enet_host_service (client, &event, 5000) > 0 &&
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
        enet_host_service (client, &event, 5000);
        if (event.type == ENET_EVENT_TYPE_RECEIVE && event.channelID == 0 && event.packet->data != NULL) {
            ClientInfo ci;
            ci.ParseFromArray(event.packet->data, event.packet->dataLength);
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
        EntityCollection players = Entity::fromTmxGetAll("planes", "aircraft", &m, 0, &plImg, space);
        Texture clImg;
        clImg.loadFromFile(mRenderer, "clouds.png");
        EntityCollection clouds = Entity::fromTmxGetAll("clouds", "clouds", &m, 0, &clImg, space);
        //Trap mouse to screen center
//        SDL_WarpMouseInWindow(mWindow, SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
//        SDL_SetRelativeMouseMode(SDL_TRUE);
        //set player1
        Player p1(players[playerId-1]);
        //Assume we have 2 player only, this could be change later.
        Player p2(players[2-playerId]);
        //
        explosionPrepare(mRenderer);
        //Debug Draw
        ChipmunkDebugDrawInit();
        SDL_RenderPresent(mRenderer);

        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        cpFloat timeStep = 1.0/60.0;
        cpFloat updateInterval = 100;
        cpFloat updateTime = 0;
        cpFloat fireTime = 0;

        glClearColor(1, 1, 1, 1);

        ENetEvent evt;
        void *buffer = NULL;
        int size = 0;
        cpVect mvVect = cpvzero;
        enet_uint32 lastUpdate = 0;
        enet_uint32 lastRecvUpdate = 0;
        srand(time(NULL));
        //collision
        setupCollisions(space, &p1);

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
                    p1.handleEvent(e, mRenderer, space, mvVect);
                }
            }
            p1.rightPressCheck(mvVect);

            //Firing
            fireTime += timeStep;
            enet_uint32 now = enet_time_get();
            if (now-lastUpdate >= updateInterval) {
                PlayerChange pc;
                lastUpdate = now;
                pc.set_time(enet_time_get());
                //printf("Send: %f\n",cpBodyGetAngle(p1.body()));
                pc.set_angle(cpBodyGetAngle(p1.body()));
                PlayerMove *m = pc.mutable_move();
                m->set_mvectx(mvVect.x);
                m->set_mvecty(mvVect.y);

                size = pc.ByteSize();
                buffer = malloc(size);
                pc.SerializeToArray(buffer, size);
                if (mvVect.x != 0 || mvVect.y != 0) {
                    //printf("::%f %f\n", pc.move().mvectx(), pc.move().mvecty());
                }
                ENetPacket * packet = enet_packet_create(buffer, size+1, 0);
                enet_peer_send(host, 1, packet);
                enet_host_service(client, &evt, 0);
                mvVect = cpvzero;
                free(buffer);
            }
            p1.handleFire(mRenderer, space, fireTime);
            enet_host_service(client, &evt, 0);

            if (evt.type == ENET_EVENT_TYPE_RECEIVE && evt.packet->data != NULL)
            {
//                printf ("A packet of length %u was received from %d on channel %u.\n",
//                evt.packet->dataLength,
//                (int)evt.peer->data,
//                evt.channelID);
                Update u;
                google::protobuf::RepeatedPtrField<PlayerUpdate>::iterator ii;
                u.ParseFromArray(evt.packet->data, evt.packet->dataLength);
                google::protobuf::RepeatedPtrField<PlayerUpdate> pus = u.players();

                if (u.time() <= lastRecvUpdate) {
                    continue;
                }
                lastRecvUpdate = u.time();
                for (ii = pus.begin(); ii != pus.end(); ++ii) {
                    PlayerUpdate pu = *ii;
                    Player *p = (pu.player() == playerId) ? &p1 : &p2;
                    //printf("Rec player: %d\n", playerId);
                    cpVect pos = cpBodyGetPosition(p->body());
                    //printf("Offset time %u\n", enet_time_get()-u.time());
//                    printf("Player : %u , Pos(%f,%f) vs (%f, %f) , vel(%f,%f), angle(%f) vs (%f) \n",pu.player(),pu.posx(),pu.posy(), pos.x, pos.y, pu.velx(),pu.vely(), pu.angle(), cpBodyGetAngle(p->body()));
//                    cpBodySetAngle(p->body(), pu.angle());
                    cpBodySetVelocity(p->body(), cpv(pu.velx(), pu.vely()));
                    cpFloat angle = cpBodyGetAngle(p->body());
                    cpBodySetAngle(p->body(), pu.angle());
                    cpVect svpos = cpv(pu.posx(), pu.posy());
                    cpFloat timeoffs = cpFloat(enet_time_get()-u.time())/1000.;
                    cpFloat dist = cpvdist(svpos, pos);
                    if (dist >= 100) {
                        cpBodySetPosition(p->body(), svpos);
                        cpSpaceStep(space, timeoffs);
                    } else if (dist >= 5) {
                        cpVect offs = cpvsub(svpos, pos);
                        cpVect vel = cpBodyGetVelocity(p->body());
                        cpBodySetVelocity(p->body(), cpvmult(offs, 1./cpvlength(offs)));
                        cpSpaceStep(space, timeoffs/3.);
                        cpBodySetVelocity(p->body(), vel);
                    }
                    if (pu.player() == playerId ) {
                        cpBodySetAngle(p->body(), angle);
                    }
                }
                enet_packet_destroy (evt.packet);
            }
            //Move the aircraft
            p1.fly();
            p2.fly();

//            //Clear screen
//            SDL_SetRenderDrawColor( mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
//            SDL_RenderClear( mRenderer );

//            Render aircraft
            Entity::renderAll(players, mRenderer);

            p1.render(mRenderer);

            //TODO: split explosion into class, this explosionCheck from nowhere is...
            explosionCheckRender(mRenderer);
//          Entity::renderAll(clouds, mRenderer);

            Entity::renderAll(clouds, mRenderer);


            cpSpaceStep(space, timeStep);
            drawPlayerHp(p1, mRenderer,0,0,assets.defFont());
            drawPlayerHp(p1, mRenderer,666,0,assets.defFont());
            //aacircleColor(mRenderer, 100, 50, 300, 0x80a8f2);
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

            Sleep(5);
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
    //SDL_ShowCursor(0);
    return app.start();
}
