#include "client.h"
#include "proto/clientinfo.pb.h"

using namespace std;

namespace xx {

Client::Client():
    mClient(NULL),
    mHost(NULL),
    mPlayerId(-1),
    mBuffer(NULL),
    mBuffSize(0),
    mOfflineMode(false)
 {
    mClient = enet_host_create(NULL /* create a client host */,
                1 /* only allow 1 outgoing connection */,
                10,
                0,
                0);
    if (mClient == NULL)
    {
        fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }
}

Client::~Client() {
    if (mClient) enet_host_destroy(mClient);
}

void Client::connect(const char * dest, int timeout) {
    if (mOfflineMode) {
        return;
    }
    ENetAddress address;
    ENetEvent event;
    enet_address_set_host(&address, dest);
    address.port = 5555;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    mHost = enet_host_connect(mClient, &address, 10, 0);
    if (mHost == NULL)
    {
       fprintf (stderr, "No available peers for initiating an ENet connection.\n");
       exit(EXIT_FAILURE);
    }

    if (enet_host_service (mClient, &event, timeout) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        printf ("Connection to %s:5555 succeeded.\n", dest);
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (mHost);
        printf ("Connection to %s:5555 failed.\n", dest);
        exit(EXIT_FAILURE);
    }

    mPlayerId = requestPlayerId();
}

int Client::requestPlayerId() {
    int playerId = -1;
    ENetEvent event;
    enet_host_service (mClient, &event, 5000);
    if (event.type == ENET_EVENT_TYPE_RECEIVE && event.channelID == 0 && event.packet->data != NULL) {
        ClientInfo ci;
        ci.ParseFromArray(event.packet->data, event.packet->dataLength);
        enet_packet_destroy(event.packet);
        return ci.player();
    } else {
        printf("No information received from host.\n");
        exit(EXIT_FAILURE);
    }

    if (playerId == 0) {
        printf("Unhandled.");
        exit(EXIT_FAILURE);
    }
}

int Client::playerId() {
    if (mOfflineMode) {
        return 1;
    }
    if (mPlayerId == -1) {
        printf("Not connected.");
        exit(EXIT_FAILURE);
    }
    return mPlayerId;
}

void Client::clean(ENetPacket *packet) {
    if (packet->data != NULL) enet_packet_destroy(packet);
    mBuffSize = 0;
}

ENetPacket* Client::recv(int timeout) {
    ENetEvent event;
    if (mOfflineMode) {
        return NULL;
    }
    enet_host_service(mClient, &event, timeout);
    if (event.type == ENET_EVENT_TYPE_RECEIVE && event.packet->data != NULL) {
        return event.packet;
    }

    return NULL;
}

void Client::send(int channel) {
    if (mOfflineMode) {
        return;
    }

    if (!mBuffer || mBuffSize == 0) {
        printf("Buffer empty, please put data into the buffer first.\n");
        exit(EXIT_FAILURE);
    }

    ENetPacket * packet = enet_packet_create(mBuffer, mBuffSize, 0);
    enet_peer_send(mHost, channel, packet);
    enet_host_flush(mClient);
    free(mBuffer);
}

void* Client::buffer(int size) {
    if (mBuffer != NULL) {
        free(mBuffer);
    }
    mBuffer = malloc(size);
    mBuffSize = size;
    return mBuffer;
}

void Client::setOfflineMode() {
    mOfflineMode = true;
}

//ns end
}
