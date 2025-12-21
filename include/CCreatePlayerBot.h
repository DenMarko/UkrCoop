#ifndef _INCLUDE_CREATE_PLAYER_BOT_HEADER_
#define _INCLUDE_CREATE_PLAYER_BOT_HEADER_

#include "extension.h"

class ITerrorPlayer;
class IWitch;
class IInfected;
class INavArea;

class CBotPlayerCreate
{
public:
    CBotPlayerCreate()
    {}

    ~CBotPlayerCreate()
    {
    }

    ITerrorPlayer   *SpawnTank(     const Vector& vecOrigion,   const QAngle& vecAngle);
    ITerrorPlayer   *SpawnTank(     const INavArea* pNav,       const QAngle& vecAngle);

    ITerrorPlayer   *SpawnBoomer(   const Vector& vecOrigion,   const QAngle& vecAngle);
    ITerrorPlayer   *SpawnBoomer(   const INavArea* pNav,       const QAngle& vecAngle);

    ITerrorPlayer   *SpawnSmoker(   const Vector& vecOrigion,   const QAngle& vecAngle);
    ITerrorPlayer   *SpawnSmoker(   const INavArea* pNav,       const QAngle& vecAngle);

    ITerrorPlayer   *SpawnHunter(   const Vector& vecOrigion,   const QAngle& vecAngle);
    ITerrorPlayer   *SpawnHunter(   const INavArea* pNav,       const QAngle& vecAngle);

    IWitch          *SpawnWitch(    const Vector& vecOrigion,   const QAngle& vecAngle);
    IWitch          *SpawnWitch(    const INavArea* pNav,       const QAngle& vecAngle);

    IInfected       *SpawnCommon(   const Vector& vecOrigion,   const QAngle& vecAngle);
    IInfected       *SpawnCommon(   const INavArea* pNav,       const QAngle& vecAngle);

    ITerrorPlayer   *SpawnSurvivor(  );

private:
    ITerrorPlayer *NextBotCreatePlayerBot(const char* szClassName, const char* szName);
    void Event_SpawnZombie( int nType, int nVal );
    ITerrorPlayer *EdictToTerrorPlayer(edict_t* pEdict);

private:
};

extern CBotPlayerCreate* g_pBotCreator;

#endif //_INCLUDE_CREATE_PLAYER_BOT_HEADER_