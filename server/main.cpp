#include <iostream>
#include <enet/enet.h>
#include <chipmunk_private.h>
#include <SDL.h>

#include "global.h"
#include "player.h"
#include "proto/player.pb.h"
#include "proto/clientinfo.pb.h"

using namespace std;
using namespace xx;

int main(int argc, char* args[])
{
    if (enet_initialize () != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

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

    cpFloat updateInterval = 1.0/10.0;
    cpFloat updateTime = 0;
    cpFloat timeStep = 1.0/20.;

    ENetPeer * peers[3];
    while (1)
    {
        cpSpaceStep(space, timeStep);
        updateTime += timeStep;
        int size;
        void *buffer;
        if (updateTime >= updateInterval) {
            updateTime -= updateInterval;
            Update u;
            u.set_time(enet_time_get());

            for (size_t i=0; i<players.size(); ++i) {
                if (!connected[i]) {
                    continue;
                }
                Player * player = &players[i];
                PlayerUpdate pu;
                pu.set_angle(cpBodyGetAngle(player->body()));
                printf("%f :)\n", cpBodyGetAngle(player->body()));
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

        if (enet_host_service (server, &event, 200) <= 0) {
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
                pc.ParseFromString(string((char*)event.packet->data));
                if (lastUpdated[playerId] < pc.time() || pc.time() == 0) {
                    lastUpdated[playerId] = pc.time();
                    PlayerMove m = pc.move();
                    cpBodySetAngle(p->body(), m.angle());
                    p->setMove(cpvmult(p->vectorForward(), (cpFloat)m.forwards()));
                    p->fly();
                    printf("Receive Player at time %u: %d , pos(%f,%f) , vel(%f,%f), angle(%f)\n", pc.time(), playerId,
                        cpBodyGetPosition(p->body()).x,
                        cpBodyGetPosition(p->body()).y,
                        cpBodyGetVelocity(p->body()).x,
                        cpBodyGetPosition(p->body()).y,
                        cpBodyGetAngle(p->body())
                    );
                }
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
