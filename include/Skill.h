#ifndef SKILL_H
#define SKILL_H
#include "player.h"


class Skill
{
    public:
        Skill();
        virtual ~Skill();
        enet_uint32 lastUsedPush = 0;
        void push(Player *p);
        float coolDownCheck(enet_uint32 timer, int skillNum);
    protected:
    private:
};

#endif // SKILL_H
