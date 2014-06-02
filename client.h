#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED
#include <enet/enet.h>

namespace xx {

class Client {
private:
    ENetHost *mClient;
    ENetEvent mEvent;
    ENetPeer *mHost;
    int mPlayerId;
    void * mBuffer;
    int requestPlayerId();
    int mBuffSize;

public:
    Client();
    ~Client();

    void connect(const char* dest, int timeout);
    ENetPacket* recv(int timeout);
    void clean();
    void send(int channel);
    void* buffer(int size);
    int playerId();
};

//ns end
}

#endif // CLIENT_H_INCLUDED
