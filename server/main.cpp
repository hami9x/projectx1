#include <iostream>
#include <enet/enet.h>
#include <chipmunk_private.h>
#include <SDLU.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "ChipmunkDebugDraw.h"

#include "global.h"
#include "player.h"
#include "proto/player.pb.h"
#include "proto/clientinfo.pb.h"

using namespace std;
using namespace xx;

SDL_Window * window;
SDL_Renderer * renderer;

bool initSDL() {
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
    window = SDL_CreateWindow( "Project X1: Mini aeroPlant", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    if( window == NULL )
    {
        printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return false;
    }

    //Create vsynced renderer for mWindow
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( renderer == NULL )
    {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return false;
    }

    //Initialize renderer color
    SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );

    return true;
}

int main(int argc, char* args[])
{
    if (enet_initialize () != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    initSDL();

    // cpVect is a 2D vector and cpv() is a shortcut for initializing them.
    cpVect gravity = cpv(0, 0);
    // Create an empty space.
    cpSpace *space = cpSpaceNew();
    cpSpaceSetGravity(space, gravity);
    cpSpaceSetDamping(space, 0.5);

    TmxMap m;
    TmxReturn error = tmxparser::parseFromFile("../map.tmx", &m);
    if (error != TmxReturn::kSuccess) {
        printf("Tmx parse error. Code %d.\n", error);
    }

    EntityCollection playerEnts = Entity::fromTmxGetAll("planes", "aircraft", &m, 0, NULL, space);
    EntityCollection clouds = Entity::fromTmxGetAll("clouds", "clouds", &m, 0, NULL, space);

    vector<Player> players(playerEnts.size());
    for (size_t i=0; i<playerEnts.size(); i++) {
        players[i] = Player(playerEnts[i]);
    }

    vector<bool> connected(playerEnts.size()+1, false);

    ENetAddress address;
    ENetHost * server;
    address.host = ENET_HOST_ANY;
    address.port = 5555;
    server = enet_host_create (&address /* the address to bind the server host to */,
                                 2      /* allow up to 32 clients and/or outgoing connections */,
                                  10      ,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);
    if (server == NULL)
    {
        fprintf (stderr,
                 "An error occurred while trying to create an ENet server host.\n");
        exit (EXIT_FAILURE);
    }

    vector<int> lastUpdated(players.size(), 0);
    ENetEvent event;
    int clientId = 1;

    cpFloat timeStep = 1.0/60.;
    cpFloat updateInterval = 10;
    enet_uint32 updatedTime = enet_time_get();

    ENetPeer * peers[3];
    SDL_Event e;
    ChipmunkDebugDrawInit();
    while (1)
    {
        while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    return 0;
                }
            }
        int size;
        void *buffer;
        enet_uint32 now = enet_time_get();
        if (now - updatedTime >= updateInterval) {
            //printf("Interval: %u\n", now-updatedTime);
            updatedTime = now;
            Update u;
            u.set_time(now);

            for (size_t i=0; i<players.size(); ++i) {
                if (!connected[i]) {
                    continue;
                }
                Player * player = &players[i];
                PlayerUpdate pu;
                pu.set_angle(cpBodyGetAngle(player->body()));
                //printf("%f :)\n", cpBodyGetAngle(player->body()));
                cpVect pos = cpBodyGetPosition(player->body());
                cpVect vel = cpBodyGetVelocity(player->body());
                pu.set_player(i+1);
                pu.set_posx(pos.x);
                pu.set_posy(pos.y);
                pu.set_velx(vel.x);
                pu.set_vely(vel.y);
                *u.add_players() = pu;
            }

            size = u.ByteSize();
            buffer = malloc(size);
            u.SerializeToArray(buffer, size);
            enet_host_broadcast(server, 4, enet_packet_create(buffer, size+1, 0));
            //printf("Broadcast Player %u , pos(%f,%f) , vel(%f,%f)\n",i+1,pu.posx(),pu.posy(),pu.velx(),pu.vely());
            free(buffer);
        }

        //Entity::renderAll(playerEnts, renderer);
        SDL_RenderPresent(renderer);
        SDLU_GL_RenderCacheState(renderer);
        glShadeModel(GL_SMOOTH);
        glClear(GL_COLOR_BUFFER_BIT);
        ChipmunkDebugDrawPushRenderer();
        PerformDebugDraw(space);
        ChipmunkDebugDrawFlushRenderer();
        ChipmunkDebugDrawPopRenderer();
        glShadeModel(GL_FLAT);      /* restore state */
        SDLU_GL_RenderRestoreState(renderer);
        cpSpaceStep(space, timeStep);

        if (enet_host_service (server, &event, 0) <= 0) {
            continue;
        }

        ClientInfo ci;
        PlayerUpdate pu;
        ENetPacket * packet;
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            printf ("A new client connected from %x:%u.\n",
                    event.peer -> address.host,
                    event.peer -> address.port);
            /* Store any relevant client information here. */
            event.peer->data = (void*)clientId;
            if (clientId == 1 || clientId == 2) {
                connected[clientId-1] = true;
                ci.set_player(clientId);
            } else {
                ci.set_player(0);
            }

            size = ci.ByteSize();
            buffer = malloc(size);
            ci.SerializeToArray(buffer, size);
            enet_peer_send(event.peer, 0, enet_packet_create(buffer, size+1, 0));
            free(buffer);
            peers[0] = event.peer;
            clientId++;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf ("A packet of length %u was received from %d on channel %u.\n",
                    event.packet -> dataLength,
                    (int)event.peer -> data,
                    event.channelID);
            if (event.channelID == 1) {
                int playerId = (int)event.peer->data;
                Player *p = &players[playerId-1];
                PlayerChange pc;
                pc.ParseFromArray(event.packet->data, event.packet->dataLength);
                if (lastUpdated[playerId] < pc.time() || pc.time() == 0) {
                    lastUpdated[playerId] = pc.time();
                    PlayerMove m = pc.move();
                    printf(":::::%f %f\n", m.mvectx(), m.mvecty());
                    p->setMove(cpv(m.mvectx(), m.mvecty()));
                    //p->setMove(cpvmult(p->vectorForward(), (cpFloat)m.forwards()));
                    p->fly();
                    cpBodySetAngle(p->body(), pc.angle());
                    printf("Receive Player at time %u: %d forwards, %d , pos(%f,%f) , vel(%f,%f), angle(%f)\n", pc.time(), 0, playerId,
                        cpBodyGetPosition(p->body()).x,
                        cpBodyGetPosition(p->body()).y,
                        cpBodyGetVelocity(p->body()).x,
                        cpBodyGetPosition(p->body()).y,
                        cpBodyGetAngle(p->body())
                    );
                }
<<<<<<< HEAD
                printf("Receive Player %d , pos(%f,%f) , vel(%f,%f)\n",playerId,
                       cpBodyGetPosition(p->body()).x,
                       cpBodyGetPosition(p->body()).y,
                       cpBodyGetVelocity(p->body()).x,
                       cpBodyGetVelocity(p->body()).y);
=======
>>>>>>> 83c74c513900f66fb36800528a5815648faf6b20
            }
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy (event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf ("%d disconnected.\n", (int)event.peer->data);
            /* Reset the peer's client information. */
            event.peer -> data = NULL;
        }
    }

    enet_host_destroy(server);
    return 0;
}
