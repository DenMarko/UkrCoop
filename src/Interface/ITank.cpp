#include "ITank.h"

void ITank::EnterStasis(void)
{
    if(access_member<bool>(this, 11973))
    {
        return;
    }

    if(edict())
    {
        access_member<MoveType_t>(this, 11976) = GetMoveType();
        SetSolidFlags(GetSolidFlags() | FSOLID_TRIGGER);
        SetMoveType(MOVETYPE_NONE);

        if(VPhysicsGetObject())
        {
            VPhysicsGetObject()->EnableCollisions(false);
            VPhysicsGetObject()->Sleep();
        }
        AddEffects(EF_NODRAW);
        m_takedamage = 0;
        access_member<bool>(this, 11973) = true;
    }

    return;
}

void ITank::LeaveStasis(void)
{
    if(!access_member<bool>(this, 11973))
    {
        return;
    }

    SetSolidFlags(GetSolidFlags() & 0xFFFB);
    SetMoveType(access_member<MoveType_t>(this, 2994*4));

    if(VPhysicsGetObject())
    {
        VPhysicsGetObject()->EnableCollisions(true);
        VPhysicsGetObject()->Wake();
    }

    RemoveEffects(0xFFFFFFDF);
    m_takedamage = 2;
    access_member<bool>(this, 11973) = false;
}