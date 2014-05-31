#ifndef SKILL_H_INCLUDED
#define SKILL_H_INCLUDED
#include <enet/enet.h>

namespace xx {
class Skill
{
    public:
        Skill();

        ~Skill();

        //Checking cooldown
        float coolDownCheck(int skillNum);

        enet_uint32 lastUsedPush;

    protected:
    private:

};
}


#endif // SKILL_H
