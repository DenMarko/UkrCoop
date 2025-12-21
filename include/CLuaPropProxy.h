#ifndef _INCLUDE_CLUA_PROP_PROXY_PROPER_H_
#define _INCLUDE_CLUA_PROP_PROXY_PROPER_H_
#include "extension.h"

const char* GetNameClass(CBaseEntity* pThisPtr);
float       GetPoseParameter(CBaseEntity* pThisPtr, int element);
void        SetPoseParameter(CBaseEntity* pThisPtr, float val, int element);
int         GetClientAimTarget(CBaseEntity *pThisPtr, bool only_player);

int         Prop_get_CollisionGroup(const CBaseEntity* pThisPtr);
int         Prop_get_MoveType(const CBaseEntity *pThisPtr);
Vector      Prop_get_vecOrigin(const CBaseEntity* pThisPtr);
int         Prop_get_spawnflags(const CBaseEntity *p);
int         Prop_get_fFlags(const CBaseEntity* pThisPtr);
QAngle*     Prop_get_angRotation(const CBaseEntity* pThisPtr);
float       Prop_get_flCycle(const CBaseEntity* pThisPtr);
int         Prop_get_nSequence(const CBaseEntity* pThisPtr);
int         Prop_get_nRenderFX(const CBaseEntity* pThisPtr);
float       Prop_get_flAnimTime(const CBaseEntity* pThisPtr);
float       Prop_get_flSimulationTime(const CBaseEntity *pThisPtr);
bool        Prop_get_mobRush(const CBaseEntity* pThisPtr);
CBaseEntity*Prop_get_hGroundEntity(const CBaseEntity* pThisPtr);
CBaseEntity*Prop_get_hOwnerEntity(const CBaseEntity* pThisPtr);
float       Prop_get_rage(const CBaseEntity* pThisPtr);
float       Prop_get_flPlaybackRate(const CBaseEntity* pThisPtr);
int         Prop_get_SolidFlags(const CBaseEntity* pThisPtr);
int         Prop_get_nRenderMode(const CBaseEntity* pThisPtr);
float       Prop_get_fGravity(const CBaseEntity* pThisPtr);
Vector*     Prop_get_vecBaseVelocity(const CBaseEntity* pThisPtr);
Vector*     Prop_get_vecVelocity(const CBaseEntity *pThisPtr);
int         Prop_get_EFlags(const CBaseEntity* pThisPtr);
string_t    Prop_get_target(CBaseEntity* pThisPtr);
Vector*     Prop_get_vecViewOffset(CBaseEntity *pThisPtr);
IPhysicsObject *Prop_get_PhysicsObject(const CBaseEntity *pThisPtr);
CBaseEntity*Prop_get_hEffectEntity(const CBaseEntity *pThisPtr);
int         Prop_get_fEffects(const CBaseEntity *pThisPtr);
CBaseEntity*Prop_get_customAbility(const CBaseEntity* pThisPtr);
int         Prop_get_isIncapacitated(const CBaseEntity *pThisPtr);
int         Prop_get_bLocked(const CBaseEntity* pThisPtr);
int         Prop_get_eDoorState(const CBaseEntity *pThisPtr);
int         Prop_get_iPrimaryAmmoType(CBaseEntity* pThisPtr);
int         Prop_get_iSecondaryAmmoType(CBaseEntity* pThisPtr);
CBaseEntity*Prop_get_hActiveWeapon(CBaseEntity* pThisPtr);
QAngle      *Prop_get_angAbsRotation(CBaseEntity *pThisPtr);
Vector      *Prop_get_vecAbsOrigin(CBaseEntity *pThisPtr);
char        Prop_get_takedamage(const CBaseEntity *pThisPtr);
unsigned char Prop_get_nWaterLevel(const CBaseEntity *pThisPtr);
int         Prop_get_zombieClass(const CBaseEntity *pThisPtr);
CBaseEntity *Prop_get_hMoveParent(const CBaseEntity *pThisPtr);
bool        Prop_get_bSequenceLoops(const CBaseEntity* pThisPtr);
int         Prop_get_MaxHealth(const CBaseEntity*pThis);
int         Prop_get_Health(const CBaseEntity* pThisPtr);

void        Prop_set_spawnflags(CBaseEntity *p, int flags);
void        Prop_set_fFlags(CBaseEntity* pThisPtr, int flag);
void        Prop_set_vecOrigin(CBaseEntity* pThisPtr, Vector* vec);
void        PropData_set_vecOrigin(CBaseEntity* pThisPtr, Vector* vec);
void        Prop_set_angRotation(CBaseEntity* pThisPtr, QAngle* ang);
void        PropData_set_angRotation(CBaseEntity* pThisPtr, QAngle* ang);
void        Prop_set_flCycle(CBaseEntity* pThisPtr, float val);
void        Prop_set_nSequence(CBaseEntity* pThisPtr, int val);
void        Prop_set_flAnimTime(CBaseEntity* pThisPtr, int val);
void        Prop_set_flSimulationTime(CBaseEntity *pThisPtr, float val);
void        Prop_set_nRenderFX(CBaseEntity* pThisPtr, int val);
void        Prop_set_mobRush(CBaseEntity* pThisPtr, bool val);
void        Prop_set_hGroundEntity(CBaseEntity* pThisPtr, CBaseEntity* val);
void        Prop_set_hOwnerEntity(CBaseEntity* pThisPtr, CBaseEntity* val);
void        Prop_set_rage(CBaseEntity* pThisPtr, float val);
void        Prop_set_flPlaybackRate(CBaseEntity* pThisPtr, float val);
void        Prop_set_CollisionGroup(CBaseEntity* pThisPtr, int val);
void        Prop_set_MoveType(CBaseEntity* pThisPtr, int val);
void        Prop_set_fGravity(CBaseEntity* pThisPtr, float g);
void        Prop_set_nRenderMode(CBaseEntity* pThisPtr, int nRenderMode);
void        Prop_set_SolidFlags(CBaseEntity* pThisPtr, int val);
void        Prop_set_vecBaseVelocity(CBaseEntity* pThisPtr, Vector &vecBaseVelocity);
void        Prop_set_vecVelocity(CBaseEntity *pThisPtr, Vector* vel);
void        Prop_set_EFlags(CBaseEntity* pThisPtr, int nEFlags);
void        Prop_set_hEffectEntity(CBaseEntity* pThisPtr, CBaseEntity* ent);
void        Prop_set_fEffects(CBaseEntity *pThisPtr, int val);
void        Prop_set_isIncapacitated(CBaseEntity *pThisPtr, int Val);
void        Prop_set_bLocked(CBaseEntity *pThisPtr, int val);
void        Prop_set_eDoorState(CBaseEntity* pThisPtr, int val);
void        Prop_set_iPrimaryAmmoType(CBaseEntity* pThisPtr, int val);
void        Prop_set_iSecondaryAmmoType(CBaseEntity* pThisPtr, int val);
void        Prop_set_hActiveWeapon(CBaseEntity* pThisPtr, CBaseEntity* val);
void        Prop_set_MaxHealth(CBaseEntity* pThisPtr, int val);
void        Prop_set_Health(CBaseEntity* pThisPtr, int val);

bool        IsIdentical(const Vector* pThis, const Vector* src);
bool        IsNotIdentical(const Vector* pThis, const Vector* src);
bool        IsIdentical(const QAngle* pThis, const QAngle* src);
bool        IsNotIdentical(const QAngle* pThis, const QAngle* src);

#endif