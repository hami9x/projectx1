#include <iostream>
#include <enet/enet.h>
#include <chipmunk_private.h>
#include <SDL.h>
#include <ctime>

#include "global.h"
#include "player.h"
#include "proto/player.pb.h"
#include "proto/clientinfo.pb.h"

using namespace std;
using namespace xx;

#define CLOCKS_PER_MSEC CLOCKS_PER_SEC/1000

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

    EntityCollection playerEnts = Entity::fromTmxGetAll("planes", "aircraft", &m, 0, NULL, space, 5, PLANE_TYPE);
    EntityCollection clouds = Entity::fromTmxGetAll("clouds", "clouds", &m, 0, NULL, space, 1000, CLOUD_TYPE);

    vector<Player> players(playerEnts.size());
    for (size_t i=0; i<playerEnts.size(); i++) {
        players[i] = Player(playerEnts[i]);
    }

    ENetAddress address;
    ENetHost * server;
    address.host = ENET_HOST_ANY;
    address.port = 5555;
    server = enet_host_create (&address /* the address to bind the server host to */,
                                 2      /* allow up to 32 clients and/or outgoing connections */,
                                  2      /* allow up to 2 channels to be used, 0 and 1 */,
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

    std::clock_t start;
    double duration;

    start = std::clock();
    while (1)
    {
        duration = (std::clock() - start) / (double)CLOCKS_PER_MSEC;
        if (duration > 100) {
            start = std::clock();
            for (size_t i=0; i<players.size(); ++i) {
                Player * player = &players[i];
                PlayerUpdate pu;
                pu.set_angle(cpBodyGetAngle(player->body()));
                cpVect pos = cpBodyGetPosition(player->body());
                cpVect vel = cpBodyGetVelocity(player->body());
                pu.set_posx(pos.x);
                pu.set_posy(pos.y);
                pu.set_velx(vel.x);
                pu.set_vely(vel.y);
                string pus = pu.SerializeAsString();
                enet_host_broadcast(server, 3, enet_packet_create(pus.c_str(), pus.size()+1, 0));
            }
        }

        if (enet_host_service (server, &event, 10000) <= 0) {
            continue;
        }

        ClientInfo ci;
        string infstr;
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            printf ("A new client connected from %x:%u.\n",
                    event.peer -> address.host,
                    event.peer -> address.port);
            /* Store any relevant client information here. */
            event.peer->data = (void*)clientId;
            ci.set_player(clientId == 1 || clientId == 2 ? clientId : 0);
            infstr = string(ci.SerializeAsString());
            enet_peer_send(event.peer, 0, enet_packet_create(infstr.c_str(), infstr.size()+1, 0));
            clientId++;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf ("A packet of length %u was received from %d on channel %u.\n",
                    event.packet -> dataLength,
                    (int)event.peer -> data,
                    event.channelID);
            if (event.channelID == 1) {
                int playerId = (int)event.peer->data;
                Player *p = &players[playerId];
                PlayerChange pc;
                pc.ParseFromString(std::string((char*)event.packet->data));
                if (lastUpdated[playerId] < pc.time()) {
                    lastUpdated[playerId] = pc.time();
                    PlayerMove m = pc.move();
                    cpBodySetAngle(p->body(), m.angle());
                    p->setMove(cpvmult(p->vectorForward(), (cpFloat)m.forwards()));
                    p->fly();
                }
            }
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy (event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf ("%d disconnected.\n", (char*)event.peer->data);
            /* Reset the peer's client information. */
            event.peer -> data = NULL;
        }
    }

    enet_host_destroy(server);
    return 0;
}
