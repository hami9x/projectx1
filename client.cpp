#include "client.h"
#include "proto/clientinfo.pb.h"

using namespace std;

namespace xx {

Client::Client() {
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
    mPlayerId = -1;
    mBuffer = 0;
    mBuffSize = 0;
}

Client::~Client() {
    enet_peer_reset(mHost);
    enet_host_destroy(mClient);
}

void Client::connect(const char * dest, int timeout) {
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
        exit(EXIT_FAILURE);
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
    if (mPlayerId == -1) {
        printf("Not connected.");
        exit(EXIT_FAILURE);
    }
    return mPlayerId;
}

void Client::clean() {
    if (mBuffer != NULL) free(mBuffer);
    if (mEvent.packet != NULL) enet_packet_destroy(mEvent.packet);
    mBuffSize = 0;
}

ENetPacket* Client::recv(int timeout) {
    enet_host_service(mClient, &mEvent, timeout);
    if (mEvent.type == ENET_EVENT_TYPE_RECEIVE && mEvent.packet->data != NULL) {
        return mEvent.packet;
    }

    return NULL;
}

void Client::send(int channel) {
    if (!mBuffer || mBuffSize == 0) {
        printf("Buffer empty, please put data into the buffer first.\n");
        exit(EXIT_FAILURE);
    }
    ENetPacket * packet = enet_packet_create(mBuffer, mBuffSize, 0);
    enet_peer_send(mHost, channel, packet);
    enet_host_service(mClient, NULL, 0);
    clean();
}

void* Client::buffer(int size) {
    mBuffer = malloc(size);
    mBuffSize = size;
    return mBuffer;
}

//ns end
}
