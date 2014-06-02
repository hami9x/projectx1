#ifndef SYNCER_H_INCLUDED
#define SYNCER_H_INCLUDED
#include "client.h"
#include "utils.h"
#include "player.h"
#include "proto/player.pb.h"
#include "physics.h"

namespace xx {
    class Syncer {
        Client *mClient;
        Player *mPlayer;
        Player *mOtherPlayer;

        uint32 mLastRecvUpdate;
        Update mUpdate;
        bool mUpdated;
        bool mStopped;
        void playerHostSync();
        void playerSendUpdate(cpVect & mvVect);
    public:
        Syncer(Client *, Player *, Player *);
        void start(cpVect & mvVect);
        void stop();
        void updateBodies(Physics * physics);
    };
}

#endif // SYNCER_H_INCLUDED
