#include <thread>
#include "chipmunk_private.h"
#include "syncer.h"
#include "utils.h"

namespace xx {

Syncer::Syncer(Client *client, Player *player, Player *otherPlayer) {
    mClient = client;
    mPlayer = player;
    mOtherPlayer = otherPlayer;
    mLastRecvUpdate = 0;
    mUpdated = false;
}

void Syncer::start(cpVect & mvVect) {
    std::thread ut1(&Syncer::playerHostSync, *this);
    std::thread ut2(&Syncer::playerSendUpdate, *this, std::ref(mvVect));
}

void Syncer::playerHostSync() {
    ENetPacket * packet;
    while(true) {
        packet = mClient->recv(5);
        if (packet != NULL) {
            mUpdate.ParseFromArray(packet->data, packet->dataLength);
            mUpdated = true;
    //                printf ("A packet of length %u was received from %d on channel %u.\n",
    //                evt.packet->dataLength,
    //                (int)evt.peer->data,
    //                evt.channelID);
            mClient->clean();
            Sleep(20);
            continue;
        }

        return;
    }
}

void Syncer::playerSendUpdate(cpVect & mvVect) {
    utils::Timer timer(50);
    int size = 0;
    while(true) {
        if (timer.exceededReset()) {
            if (cpvlength(mvVect) == 0) {
                continue;
            }
            PlayerChange pc;
            cpBody * body=mPlayer->body();
            pc.set_time(utils::now());
            //printf("Send: %f\n",cpBodyGetAngle(p1.body()));
            pc.set_angle(cpBodyGetAngle(body));
            PlayerMove *m = pc.mutable_move();
            m->set_mvectx(mvVect.x);
            m->set_mvecty(mvVect.y);

            size = pc.ByteSize();
            pc.SerializeToArray(mClient->buffer(size), size);
            mClient->send(1);
            if (mvVect.x != 0 || mvVect.y != 0) {
                //printf("::%f %f\n", pc.move().mvectx(), pc.move().mvecty());
            }
            mvVect = cpvzero;
        }
        Sleep(20);
    }
}

//To be called in main for actual update of the players and objects
void Syncer::updateBodies(Physics * physics) {
    if (!mUpdated || mUpdate.time() <= mLastRecvUpdate) {
        return;
    }
    mUpdated = true;
    mLastRecvUpdate = mUpdate.time();
    google::protobuf::RepeatedPtrField<PlayerUpdate> pus = mUpdate.players();
    google::protobuf::RepeatedPtrField<PlayerUpdate>::iterator ii;
    for (ii = pus.begin(); ii != pus.end(); ++ii) {
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
        cpFloat timeoffs = utils::now()-mUpdate.time();
        cpFloat dist = cpvdist(svpos, pos);
        //printf("dist>> %f\n", dist);

        if (dist >= 100) {
            //cpBodySetAngle(p->body(), (pu.angle() + angle)/2.);
            cpBodySetPosition(p->body(), svpos);
            physics->step(timeoffs);
            cpBodySetAngle(p->body(), pu.angle());
            cpVect pos2 = cpBodyGetPosition(p->body());
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
