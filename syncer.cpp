#include "syncer.h"

#include <thread>
#include <chrono>
#include "client.h"
#include "player.h"
#include "physics.h"

namespace xx {

Syncer::Syncer(Client *client, Player *player, Player *otherPlayer) {
    mClient = client;
    mPlayer = player;
    mOtherPlayer = otherPlayer;
    mLastRecvUpdate = 0;
}

Syncer::~Syncer() {
}

void Syncer::playerHostSync(bool *stopped, Update & update, bool & updated) {
    while(!(*stopped)) {
        ENetPacket *packet = mClient->recv(5000);
        if (packet != NULL && packet->data != NULL) {
            update.ParseFromArray(packet->data, packet->dataLength);
            updated = true;
//                    printf ("A packet of length %u was received from %d on channel %u.\n",
//                    evt.packet->dataLength,
//                    (int)evt.peer->data,
//                    evt.channelID);
            mClient->clean(packet);

            std::chrono::milliseconds dura(20);
            std::this_thread::sleep_for(dura);
            continue;
        }

        //return;
    }

    printf("playerHostSync stopped.\n");
}

void Syncer::playerSendUpdate(bool *stopped, cpVect * _mvVect) {
    utils::Timer timer(100);
    int size = 0;

    while(!(*stopped)) {
        if (timer.exceededReset()) {
            cpVect mvVect = *_mvVect;
            PlayerChange pc;
            if (mPlayer == NULL) {
                break;
            }
            cpBody * body=mPlayer->body();
            pc.set_time(utils::now());
            //printf("Send: %f\n",cpBodyGetAngle(body));
            pc.set_angle(cpBodyGetAngle(body));
            PlayerMove *m = pc.mutable_move();
            m->set_mvectx(mvVect.x);
            m->set_mvecty(mvVect.y);

            for(int i=0; i< mPlayer->firedNumber(); i++) {
                pc.add_firedangle(mPlayer->firedAngle(i));
            }
            pc.set_firednumber(mPlayer->firedNumber());
            mPlayer->updateReset();

            size = pc.ByteSize();
            pc.SerializeToArray(mClient->buffer(size), size);
            mClient->send(1);
            *_mvVect = cpvzero;
        }
        std::chrono::milliseconds dura(20);
        std::this_thread::sleep_for(dura);
    }
}

//To be called in main for actual update of the players and objects
void Syncer::updateBodies(Physics * physics, Update & update, bool updated) {
    if (!updated || update.time() <= mLastRecvUpdate) {
        return;
    }
    updated = true;
    mLastRecvUpdate = update.time();
    google::protobuf::RepeatedPtrField<PlayerUpdate> pus = update.players();
    google::protobuf::RepeatedPtrField<PlayerUpdate>::iterator ii;
    for (ii = pus.begin(); ii != pus.end(); ++ii) {
        //printf("DKDFKLLLLLLLLL\n");
        PlayerUpdate pu = *ii;
        cpVect svpos = cpv(pu.posx(), pu.posy());

        //Update for the other player
        if (mClient->playerId() != pu.player()) {
            cpBody * pbody = mOtherPlayer->body();
            cpBodySetVelocity(pbody, cpv(pu.velx(), pu.vely()));
            cpBodySetAngle(pbody, pu.angle());
            cpBodySetPosition(pbody, svpos);
            continue;
        }

        Player *p =  mPlayer;
        //printf("Rec player: %d\n", playerId);
        //printf("Offset time %u\n", enet_time_get()-u.time());
//                    printf("Player : %u , Pos(%f,%f) vs (%f, %f) , vel(%f,%f), angle(%f) vs (%f) \n",pu.player(),pu.posx(),pu.posy(), pos.x, pos.y, pu.velx(),pu.vely(), pu.angle(), cpBodyGetAngle(p->body()));
//                    cpBodySetAngle(p->body(), pu.angle());
        cpBodySetVelocity(p->body(), cpv(pu.velx(), pu.vely()));
        cpFloat angle = cpBodyGetAngle(p->body());
        cpBodySetAngle(p->body(), pu.angle());

        cpVect pos = cpBodyGetPosition(p->body());
        cpFloat timeoffs = utils::now()-update.time();
        cpFloat dist = cpvdist(svpos, pos);
        //printf("dist>> %f\n", dist);

        if (dist >= 100) {
            //cpBodySetAngle(p->body(), (pu.angle() + angle)/2.);
            cpBodySetPosition(p->body(), svpos);
            physics->step(timeoffs);
            cpBodySetAngle(p->body(), pu.angle());
            //printf(">>> %f <<<\n", cpvdist(pos, pos2));
            //printf("%f<< \n", timeStep*(timeoffs/ftavg));
        } else if (dist >= 20) {
            cpVect offs = cpvsub(svpos, pos);
            cpVect vel = cpBodyGetVelocity(p->body());
            cpBodySetVelocity(p->body(), cpvmult(offs, 1./5.));
            physics->spaceStep(1000);
            cpBodySetVelocity(p->body(), vel);
        }
        cpBodySetAngle(p->body(), angle);
    }
}

//ns end
}
