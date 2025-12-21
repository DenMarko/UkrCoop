#include "IBaseEntity.h"
#include "IGameRules.h"
#include "CallMember.h"
#include "ConVar_l4d.h"
#include "IBaseAnimating.h"
#include "ITeam.h"
#include "CUserMessage.h"

// int CEventAction::s_iNextIDStamp = 0;

// const char *nexttoken(char *token, const char *str, char sep)
// {
// 	if ((str == NULL) || (*str == '\0'))
// 	{
// 		*token = '\0';
// 		return(NULL);
// 	}

// 	while ((*str != sep) && (*str != '\0'))
// 	{
// 		*token++ = *str++;
// 	}
// 	*token = '\0';

// 	if (*str == '\0')
// 	{
// 		return(str);
// 	}

// 	return(++str);
// }

// CEventAction::CEventAction( const char *ActionData )
// {
// 	m_pNext = NULL;
// 	m_iIDStamp = ++s_iNextIDStamp;

// 	m_flDelay = 0;
// 	m_iTarget = NULL_STRING;
// 	m_iParameter = NULL_STRING;
// 	m_iTargetInput = NULL_STRING;
// 	m_nTimesToFire = -1;

// 	if (ActionData == NULL)
// 		return;

// 	char szToken[256];
	
// 	char chDelim = 0x1b;
// 	if (!strchr(ActionData, 0x1b))
// 	{
// 		chDelim = ',';
// 	}

// 	const char *psz = nexttoken(szToken, ActionData, chDelim);
// 	if (szToken[0] != '\0')
// 	{
// 		m_iTarget = g_HL2->AllocPooledString(szToken);
// 	}

// 	psz = nexttoken(szToken, psz, chDelim);
// 	if (szToken[0] != '\0')
// 	{
// 		m_iTargetInput = g_HL2->AllocPooledString(szToken);
// 	}
// 	else
// 	{
// 		m_iTargetInput = g_HL2->AllocPooledString("Use");
// 	}

// 	psz = nexttoken(szToken, psz, chDelim);
// 	if (szToken[0] != '\0')
// 	{
// 		m_iParameter = g_HL2->AllocPooledString(szToken);
// 	}

// 	psz = nexttoken(szToken, psz, chDelim);
// 	if (szToken[0] != '\0')
// 	{
// 		m_flDelay = atof(szToken);
// 	}

// 	nexttoken(szToken, psz, chDelim);
// 	if (szToken[0] != '\0')
// 	{
// 		m_nTimesToFire = atoi(szToken);
// 		if (m_nTimesToFire == 0)
// 		{
// 			m_nTimesToFire = -1;
// 		}
// 	}
// }

// CEventAction::CEventAction( const CEventAction &p_EventAction )
// {
// 	m_pNext = NULL;
// 	m_iIDStamp = ++s_iNextIDStamp;

// 	m_flDelay = p_EventAction.m_flDelay;
// 	m_iTarget = p_EventAction.m_iTarget;
// 	m_iParameter = p_EventAction.m_iParameter;
// 	m_iTargetInput = p_EventAction.m_iTargetInput;
// 	m_nTimesToFire = p_EventAction.m_nTimesToFire;
// }

float CBaseEntityOutput::GetMaxDelay(void)
{
	float flMaxDelay = 0;
	CEventAction *ev = m_ActionList;

	while (ev != NULL)
	{
		if (ev->m_flDelay > flMaxDelay)
		{
			flMaxDelay = ev->m_flDelay;
		}
		ev = ev->m_pNext;
	}

	return(flMaxDelay);
}

CBaseEntityOutput::~CBaseEntityOutput()
{
	CEventAction *ev = m_ActionList;
	while (ev != NULL)
	{
		CEventAction *pNext = ev->m_pNext;	
		delete ev;
		ev = pNext;
	}
}

void CBaseEntityOutput::FireOutput(variant_t Value, IBaseEntity *pActivator, IBaseEntity *pCaller, float fDelay)
{
    static ConVarRef developer("developer");
	CEventAction *ev = m_ActionList;
	CEventAction *prev = NULL;
	
	while (ev != NULL)
	{
		if (ev->m_iParameter == NULL_STRING)
		{
			g_CallHelper->CEventQueueAdd( STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), Value, ev->m_flDelay + fDelay, pActivator, pCaller, ev->m_iIDStamp );
		}
		else
		{
			variant_t ValueOverride;
			ValueOverride.SetString( ev->m_iParameter );
			g_CallHelper->CEventQueueAdd( STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), ValueOverride, ev->m_flDelay, pActivator, pCaller, ev->m_iIDStamp );
		}

		if ( developer.GetBool() )
		{
			if ( ev->m_flDelay )
			{
				char szBuffer[256];
				Q_snprintf( szBuffer, sizeof(szBuffer), "(%0.2f) output: (%s,%s) -> (%s,%s,%.1f)(%s)\n", g_pGlobals->curtime, pCaller ? STRING(pCaller->m_iClassname) : "NULL", pCaller ? STRING(pCaller->GetEntityName()) : "NULL", STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), ev->m_flDelay, STRING(ev->m_iParameter) );
				DevMsg( 2, "%s", szBuffer );
				ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
			}
			else
			{
				char szBuffer[256];
				Q_snprintf( szBuffer, sizeof(szBuffer), "(%0.2f) output: (%s,%s) -> (%s,%s)(%s)\n", g_pGlobals->curtime, pCaller ? STRING(pCaller->m_iClassname) : "NULL", pCaller ? STRING(pCaller->GetEntityName()) : "NULL", STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), STRING(ev->m_iParameter) );
				DevMsg( 2, "%s", szBuffer );
				ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
			}
		}

		if ( pCaller && pCaller->m_debugOverlays & 16)
		{
			pCaller->DrawOutputOverlay(ev);
		}

		bool bRemove = false;
		if (ev->m_nTimesToFire != EVENT_FIRE_ALWAYS)
		{
			ev->m_nTimesToFire--;
			if (ev->m_nTimesToFire == 0)
			{
				char szBuffer[256];
				Q_snprintf( szBuffer, sizeof(szBuffer), "Removing from action list: (%s,%s) -> (%s,%s)\n", pCaller ? STRING(pCaller->m_iClassname) : "NULL", pCaller ? STRING(pCaller->GetEntityName()) : "NULL", STRING(ev->m_iTarget), STRING(ev->m_iTargetInput));
				DevMsg( 2, "%s", szBuffer );
				ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
				bRemove = true;
			}
		}

		if (!bRemove)
		{
			prev = ev;
			ev = ev->m_pNext;
		}
		else
		{
			if (prev != NULL)
			{
				prev->m_pNext = ev->m_pNext;
			}
			else
			{
				m_ActionList = ev->m_pNext;
			}

			CEventAction *next = ev->m_pNext;
			delete ev;
			ev = next;
		}
	}
}

void COutputEvent::FireOutput(IBaseEntity *pActivator, IBaseEntity *pCaller, float fDelay)
{
	variant_t Val;
	Val.Set( FIELD_VOID, NULL );
	CBaseEntityOutput::FireOutput(Val, pActivator, pCaller, fDelay);
}

// void CBaseEntityOutput::ParseEventAction( const char *EventData )
// {
// 	AddEventAction( new CEventAction( EventData ) );
// }

// void CBaseEntityOutput::AddEventAction( CEventAction *pEventAction )
// {
// 	pEventAction->m_pNext = m_ActionList;
// 	m_ActionList = pEventAction;
// }

void CBaseEntityOutput::RemoveEventAction( CEventAction *pEventAction )
{
	CEventAction *pAction = GetFirstAction();
	CEventAction *pPrevAction = NULL;
	while ( pAction )
	{
		if ( pAction == pEventAction )
		{
			if ( !pPrevAction )
			{
				m_ActionList = NULL;
			}
			else
			{
				pPrevAction->m_pNext = pAction->m_pNext;
			}
			return;
		}
		pAction = pAction->m_pNext;
	}
}

// int CBaseEntityOutput::Save( ISave &save )
// {
// 	// save that value out to disk, so we know how many to restore
// 	if ( !save.WriteFields( "Value", this, NULL, m_DataMap.dataDesc, m_DataMap.dataNumFields ) )
// 		return 0;

// 	for ( CEventAction *ev = m_ActionList; ev != NULL; ev = ev->m_pNext )
// 	{
// 		if ( !save.WriteFields( "EntityOutput", ev, NULL, ev->m_DataMap.dataDesc, ev->m_DataMap.dataNumFields ) )
// 			return 0;
// 	}

// 	return 1;
// }

// int CBaseEntityOutput::Restore( IRestore &restore, int elementCount )
// {
// 	// load the number of items saved
// 	if ( !restore.ReadFields( "Value", this, NULL, m_DataMap.dataDesc, m_DataMap.dataNumFields ) )
// 		return 0;

// 	m_ActionList = NULL;

// 	// read in all the fields
// 	CEventAction *lastEv = NULL;
// 	for ( int i = 0; i < elementCount; i++ )
// 	{
// 		CEventAction *ev = new CEventAction(NULL);

// 		if ( !restore.ReadFields( "EntityOutput", ev, NULL, ev->m_DataMap.dataDesc, ev->m_DataMap.dataNumFields ) )
// 			return 0;

// 		if ( lastEv )
// 		{
// 			lastEv->m_pNext = ev;
// 		}
// 		else
// 		{
// 			m_ActionList = ev;
// 		}
// 		ev->m_pNext = NULL;
// 		lastEv = ev;
// 	}

// 	return 1;
// }

const CEventAction *CBaseEntityOutput::GetActionForTarget( string_t iSearchTarget ) const
{
	for ( CEventAction *ev = m_ActionList; ev != NULL; ev = ev->m_pNext )
	{
		if ( ev->m_iTarget == iSearchTarget )
			return ev;
	}

	return NULL;
}

int CBaseEntityOutput::NumberOfElements( void )
{
	int count = 0;
	for ( CEventAction *ev = m_ActionList; ev != NULL; ev = ev->m_pNext )
	{
		count++;
	}
	return count;
}

void CBaseEntityOutput::DeleteAllElements( void ) 
{
	CEventAction *pNext = m_ActionList;
	m_ActionList = NULL;
	while (pNext)
	{
		CEventAction *strikeThis = pNext;
		pNext = pNext->m_pNext;
		delete strikeThis;
	}
}


enum InvalidatePhysicsBits_t
{
	POSITION_CHANGED	= 0x1,
	ANGLES_CHANGED		= 0x2,
	VELOCITY_CHANGED	= 0x4,
	ANIMATION_CHANGED	= 0x8,
};

int CheckEntityVelocity( Vector &v )
{
	float r = 2000.f * 2.f;
	if (
		v.x > -r && v.x < r &&
		v.y > -r && v.y < r &&
		v.z > -r && v.z < r)
	{
		return 1;
	}
	float speed = v.Length();
	if ( speed < 2000.0f * 2.0f * 100.0f )
	{
		v *= 2000.f * 2.f / speed;
		return 0;
	}

	return -1;
}

IBaseEntity::IBaseEntity(bool bServerOnly)
{
}

void IBaseEntity::PostClientMessagesSent( void )
{
	if ( IsEffectActive( EF_NOINTERP ) )
	{
		RemoveEffects( EF_NOINTERP );
	}
}

const char *IBaseEntity::GetDebugName() const
{
    if(m_iName.Get() != NULL_STRING)
        return m_iName.Get().ToCStr();
    else
        return GetClassname().ToCStr();
}

const Vector &IBaseEntity::GetAbsOrigin() const
{
    if(IsEFlagsSet(EFL_DIRTY_ABSTRANSFORM))
    {
        const_cast<IBaseEntity *>(this)->CalcAbsolutePosition();
    }

    return m_vecAbsOrigin;
}

void IBaseEntity::SetAbsOrigin(const Vector &absOrigin)
{
    CalcAbsolutePosition();
    if(m_vecAbsOrigin == absOrigin)
    {
        return;
    }

    InvalidatePhysicsRecursive(POSITION_CHANGED);
    RemoveEFlags(EFL_DIRTY_ABSTRANSFORM);

    m_vecAbsOrigin = absOrigin;
    MatrixSetColumn(absOrigin, 3, m_rgflCoordinateFrame);
    Vector vecNewOrigin;
    IBaseEntity *pMoveParent = GetMoveParent();
    if(!pMoveParent)
    {
        vecNewOrigin = absOrigin;
    }
    else
    {
        matrix3x4_t tempMat;
        matrix3x4_t &parentTransform = GetParentToWorldTransform(tempMat);
        VectorITransform(absOrigin, parentTransform, vecNewOrigin);
    }

    if(m_vecOrigin != vecNewOrigin)
    {
        m_vecOrigin = vecNewOrigin;
        SetSimulationTime(g_pGlobals->curtime);
    }
}

const QAngle &IBaseEntity::GetAbsAngles() const
{
    if(IsEFlagsSet(EFL_DIRTY_ABSTRANSFORM))
    {
        const_cast<IBaseEntity*>(this)->CalcAbsolutePosition();
    }

    return m_angAbsRotation;
}

void IBaseEntity::SetAbsAngles(const QAngle &absAngles)
{
    CalcAbsolutePosition();
    if(m_angAbsRotation == absAngles)
    {
        return;
    }

    InvalidatePhysicsRecursive(ANGLES_CHANGED);
    RemoveEFlags(EFL_DIRTY_ABSTRANSFORM);

    m_angAbsRotation = absAngles;
    AngleMatrix(absAngles, m_rgflCoordinateFrame);
    MatrixSetColumn(m_vecAbsOrigin, 3, m_rgflCoordinateFrame);

    QAngle angNewRotation;
    IBaseEntity *pMoveParent = GetMoveParent();
    if(!pMoveParent)
    {
        angNewRotation = absAngles;
    }
    else
    {
        if(m_angAbsRotation == pMoveParent->GetAbsAngles())
        {
            angNewRotation.Init();
        }
        else
        {
            matrix3x4_t worldToParent, localMatrix;
            MatrixInvert(pMoveParent->EntityToWorldTransform(), worldToParent);
            ConcatTransforms(worldToParent, m_rgflCoordinateFrame, localMatrix);
            MatrixAngles(localMatrix, angNewRotation);
        }
    }

    if(m_angRotation != angNewRotation)
    {
        m_angRotation = angNewRotation;
        SetSimulationTime(g_pGlobals->curtime);
    }
}

const Vector &IBaseEntity::GetAbsVelocity() const
{
    if(IsEFlagsSet(EFL_DIRTY_ABSVELOCITY))
    {
        const_cast<IBaseEntity*>(this)->CalcAbsoluteVelocity();
    }
    return m_vecAbsVelocity;
}

void IBaseEntity::SetAbsVelocity(const Vector &vecAbsVelocity)
{
    if(m_vecAbsVelocity == vecAbsVelocity)
    {
        return;
    }

    InvalidatePhysicsRecursive(VELOCITY_CHANGED);
    RemoveFlag(EFL_DIRTY_ABSVELOCITY);

    m_vecAbsVelocity = vecAbsVelocity;

    IBaseEntity *pMoveParent = GetMoveParent();
    if(!pMoveParent)
    {
        m_vecVelocity = vecAbsVelocity;
        return ;
    }

    Vector relVelocity;
    VectorSubtract(vecAbsVelocity, pMoveParent->GetAbsVelocity(), relVelocity);

    Vector vNew;
    VectorIRotate(relVelocity, pMoveParent->EntityToWorldTransform(), vNew);

    m_vecVelocity = vNew;
}

void IBaseEntity::CalcAbsolutePosition()
{
    if(!IsEFlagsSet(EFL_DIRTY_ABSTRANSFORM))
        return;

    RemoveEFlags(EFL_DIRTY_ABSTRANSFORM);

    IBaseEntity *pMoveParent = GetMoveParent();
    if(!pMoveParent)
    {
        m_vecAbsOrigin = m_vecOrigin;
        m_angAbsRotation = m_angRotation;
        if(HasDataObjectType(POSITIONWATCHER))
        {
            g_CallHelper->ReportPositionChanged((CBaseEntity *)this);
        }
        return;
    }

    matrix3x4_t tmpMatrix, scratchSpace;
    ConcatTransforms(GetParentToWorldTransform(scratchSpace), m_rgflCoordinateFrame, tmpMatrix);
    MatrixCopy(tmpMatrix, m_rgflCoordinateFrame);

    MatrixGetColumn(m_rgflCoordinateFrame, 3, m_vecAbsOrigin);
    if((m_angRotation == vec3_angle) && (m_iParentAttachment == 0))
    {
        VectorCopy(pMoveParent->GetAbsAngles(), m_angAbsRotation);
    }
    else
    {
        MatrixAngles(m_rgflCoordinateFrame, m_angAbsRotation);
    }
    if(HasDataObjectType(POSITIONWATCHER))
    {
        g_CallHelper->ReportPositionChanged((CBaseEntity *)this);
    }
}

void IBaseEntity::CalcAbsoluteVelocity()
{
    if(!IsEFlagsSet(EFL_DIRTY_ABSVELOCITY))
    {
        return;
    }

    RemoveEFlags(EFL_DIRTY_ABSVELOCITY);

    IBaseEntity *pMoveParent = GetMoveParent();
    if(!pMoveParent)
    {
        m_vecAbsVelocity = m_vecVelocity;
        return;
    }

    VectorRotate(m_vecVelocity, pMoveParent->EntityToWorldTransform(), m_vecAbsVelocity);
    m_vecAbsVelocity += pMoveParent->GetAbsVelocity();
}

matrix3x4_t &IBaseEntity::GetParentToWorldTransform(matrix3x4_t &tempMatrix)
{
    IBaseEntity *pMoveParent = GetMoveParent();
    if(!pMoveParent)
    {
        SetIdentityMatrix(tempMatrix);
        return tempMatrix;
    }

    if(m_iParentAttachment != 0)
    {
        IBaseAnimating *pAnim = (IBaseAnimating *)pMoveParent->GetBaseAnimating();
        if(pAnim && pAnim->GetAttachment(m_iParentAttachment, tempMatrix))
        {
            return tempMatrix;
        }
    }

    return pMoveParent->EntityToWorldTransform();
}

void IBaseEntity::DispatchTraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *tr)
{
    if(!PassesDamageFilter(info))
    {
        return;
    }
    TraceAttack(info, vecDir, tr);
}

void IBaseEntity::TraceAttackToTriggers(const CTakeDamageInfo &info, const Vector &vecStart, const Vector &vecEnd, const Vector &vecDir)
{
    Ray_t ray;
    ray.Init(vecStart, vecEnd);

    CTriggerTraceEnums trigerTraceEnum(&ray, info, vecDir, MASK_SHOT);
    g_pTarce->EnumerateEntities(ray, true, &trigerTraceEnum);
}

void IBaseEntity::TakeDamage(const CTakeDamageInfo &m_info)
{
    ITerrorGameRules *g_pGRules = g_HL2->GetGameRules();
    if(!g_pGRules)
    {
        return;
    }

    bool bHasPhysicsForceDamage = !g_pGRules->Damage_NoPhysicsForce(m_info.GetDamageType());
    if(bHasPhysicsForceDamage && m_info.GetDamageType() != DMG_GENERIC)
    {
        if(m_info.GetDamageForce() == vec3_origin || m_info.GetDamagePosition() == vec3_origin)
        {
            static int warningCount = 0;
            if(++warningCount < 10)
            {
                if(m_info.GetDamageForce() == vec3_origin)
                {
                    DevWarning( "CBaseEntity::TakeDamage:  with m_info.GetDamageForce() == vec3_origin\n" );
                }
                if ( m_info.GetDamagePosition() == vec3_origin )
                {
                    DevWarning( "CBaseEntity::TakeDamage:  with m_info.GetDamagePosition() == vec3_origin\n" );
                }
            }
        }
    }

    if(!PassesDamageFilter(m_info))
    {
        return;
    }

    if(!g_pGRules->AllowDamage((CBaseEntity *)this, m_info))
    {
        return;
    }

    if(g_CallHelper->IsPhysIsInCallback())
    {
        g_CallHelper->PhysCallBackDamage((CBaseEntity *)this, m_info);
    }
    else
    {
        CTakeDamageInfo info = m_info;
        IBaseEntity *pAttacker = (IBaseEntity *)info.GetAttacker();
        if(pAttacker)
        {
            info.ScaleDamage(pAttacker->GetAttackDamageScale((CBaseEntity *)this));
        }
        info.ScaleDamage(GetReceivedDamageScale((CBaseEntity *)pAttacker));
        OnTakeDamage(info);
    }
}

matrix3x4_t &IBaseEntity::EntityToWorldTransform()
{
    if(IsEFlagsSet(EFL_DIRTY_ABSTRANSFORM))
    {
        CalcAbsolutePosition();
    }
    return m_rgflCoordinateFrame;
}

const matrix3x4_t &IBaseEntity::EntityToWorldTransform() const
{
    if(IsEFlagsSet(EFL_DIRTY_ABSTRANSFORM))
    {
        const_cast<IBaseEntity*>(this)->CalcAbsolutePosition();
    }
    return m_rgflCoordinateFrame;
}

void IBaseEntity::PhysicsStepRecheckGround()
{
	uint32_t mask = PhysicsSolidMaskForEntity();
	Vector mins, maxs, point;
	trace_t tr;

	VectorAdd(this->GetAbsOrigin(), WorldAlignMins(), mins);
	VectorAdd(this->GetAbsOrigin(), WorldAlignMaxs(), maxs);

	point[2] = mins[2] - 1;
	for(int x = 0; x <= 1; x++)
	{
		for(int y = 0; y <= 1; y++)
		{
			point[0] = x ? maxs[0] : mins[0];
			point[1] = y ? maxs[1] : mins[1];

            ICollideable* pCollision = this->GetCollideable();
			if(pCollision && IsNPC())
			{
				util_TraceLineFilterEntity((CBaseEntity *)this, point, point, mask, COLLISION_GROUP_NONE, &tr);
			}
			else
			{
				Ray_t ray;
				ray.Init(point, point);
				util_TraceLine(ray, mask, this->GetNetworkable()->GetEntityHandle(), COLLISION_GROUP_NONE, &tr);
			}

			if(tr.startsolid)
			{
				SetGroundEntity((IBaseEntity *)tr.m_pEnt);
				return;
			}
		}
	}
}

inline bool IsPushableMoveType( int nMoveType )
{
	if ( nMoveType == MOVETYPE_PUSH || nMoveType == MOVETYPE_NONE || 
		nMoveType == MOVETYPE_VPHYSICS || nMoveType == MOVETYPE_NOCLIP )
		return false;
	return true;
}

void IBaseEntity::SetMoveType(MoveType_t val, MoveCollide_t moveCollide)
{
    if(m_MoveType == val)
    {
        m_MoveCollide = moveCollide;
        return;
    }

    if(m_MoveType == MOVETYPE_NOCLIP && val != m_MoveType)
    {
        RemoveEFlags(EFL_NOCLIP_ACTIVE);
    }

    if(IsPushableMoveType(val) != IsPushableMoveType(m_MoveType))
    {
        CollisionProp()->MarkPartitionHandleDirty();
    }

    m_MoveType = val;
    m_MoveCollide = moveCollide;

    CollisionRulesChanged();

    switch (m_MoveType)
    {
    case MOVETYPE_WALK:
        {
            SetSimulatedEveryTick(true);
            SetAnimatedEveryTick(true);
        }
        break;
    case MOVETYPE_STEP:
        {
            bool bMoveTypeStepSim = g_pConVar->GetConVarInt("sv_teststepsimulation") == 1 ? true : false;
            SetSimulatedEveryTick(bMoveTypeStepSim ? true : false);
            SetAnimatedEveryTick(false);
        }
        break;
    case MOVETYPE_FLY:
    case MOVETYPE_FLYGRAVITY:
        {
            UpdateWaterState();
        }
        break;
    default:
        {
            SetSimulatedEveryTick(true);
            SetAnimatedEveryTick(false);
        }
    }
    CheckStepSimulationChanged();
    CheckHasGamePhysicsSimulation();
}

void IBaseEntity::AddEFlags(int nEFlags)
{
    m_iEFlags |= nEFlags;
    if(nEFlags & (EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX))
    {
        DispatchUpdateTransmitState();
    }
}

void IBaseEntity::RemoveEFlags(int nEFlags)
{
    m_iEFlags &= ~nEFlags;
    if(nEFlags & (EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX))
    {
        DispatchUpdateTransmitState();
    }
}

void IBaseEntity::SetWaterType(int nType)
{
    m_nWaterType = 0;
    if(nType & CONTENTS_WATER)
        m_nWaterType |= 1;
    if(nType & CONTENTS_SLIME)
        m_nWaterType |= 2;
}

void IBaseEntity::SetRenderColor(byte r, byte g, byte b)
{
    m_clrRender.Init(r, g, b);
}

#include "IBasePlayer.h"
void IBaseEntity::SetGroundEntity(IBaseEntity *pGround)
{
    if(m_hGroundEntity.Get() == pGround)
    {
        return;
    }

    if(pGround && IsPlayer() && pGround->GetMoveType() == MOVETYPE_VPHYSICS)
    {
        IBasePlayer *pPlayer = (IBasePlayer*)this;
        if(pPlayer->IsPlayer())
        {
            IPhysicsObject *pPhysGroud = pGround->VPhysicsGetObject();
            if(pPhysGroud)
            {
                if(pPhysGroud->GetGameFlags() & 0x0004)
                {
                    pPlayer->ForceDropOfCarriedPhysObjects((CBaseEntity *)pGround);
                }
            }
        }
    }

    IBaseEntity *oldGround = m_hGroundEntity.Get();
    m_hGroundEntity = pGround;

    if(!oldGround && pGround)
    {
        pGround->AddEntityToGroundList(this);
    }
    else if(oldGround && !pGround)
    {
        PhysicsNotifyOtherOfGroundRemoval(this, oldGround);
    }
    else
    {
        PhysicsNotifyOtherOfGroundRemoval(this, oldGround);
        pGround->AddEntityToGroundList(this);
    }

    if(pGround)
    {
        AddFlag(FL_ONGROUND);
    }
    else
    {
        RemoveFlag(FL_ONGROUND);
    }
}

int IBaseEntity::DispatchUpdateTransmitState()
{
    edict_t *ed = edict();
    if(m_nTransmitStateOwnedCounter != 0)
    {
        return ed ? ed->m_fStateFlags : 0;
    }

    return UpdateTransmitState();
}

void IBaseEntity::CollisionRulesChanged()
{
    if(VPhysicsGetObject())
    {
        IPhysicsObject *pList[1024];
        int iCount = VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
        for(int i = 0; i < iCount; i++)
        {
            if(pList[i] != nullptr)
            {
                pList[i]->RecheckCollisionFilter();
            }
        }
    }
}

void IBaseEntity::SetSimulatedEveryTick(bool bSim)
{
    if(m_bSimulatedEveryTick != bSim)
    {
        m_bSimulatedEveryTick = bSim;
    } 
}

void IBaseEntity::SetAnimatedEveryTick(bool bAnim)
{
    if(m_bAnimatedEveryTick != bAnim)
    {
        m_bAnimatedEveryTick = bAnim;
    }
}

void IBaseEntity::UpdateWaterState()
{
    Vector point;
    CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.5f, 0.0f), &point);

    SetWaterLevel(0);
    SetWaterType(CONTENTS_EMPTY);
    int cont = g_pTarce->GetPointContents_WorldOnly( point, MASK_WATER);
    if((cont & MASK_WATER) == 0)
    {
        return;
    }
    SetWaterType(cont);
    SetWaterLevel(1);

    if(IsPointSized())
    {
        SetWaterLevel(3);
    }
    else
    {
        point[2] = WorldSpaceCenter().z;
        int midcont = g_pTarce->GetPointContents_WorldOnly( point, MASK_WATER);
        if(midcont & MASK_WATER)
        {
            SetWaterLevel(2);
            point[2] = EyePosition().z;
            int eyecont = g_pTarce->GetPointContents_WorldOnly( point, MASK_WATER);
            if(eyecont & MASK_WATER)
            {
                SetWaterLevel(3);
            }
        }
    }
}

void IBaseEntity::CheckStepSimulationChanged()
{
    bool bMoveTypeStepSim = g_pConVar->GetConVarInt("sv_teststepsimulation") == 1 ? true : false;
    if(bMoveTypeStepSim != IsSimulatedEveryTick())
    {
        SetSimulatedEveryTick(bMoveTypeStepSim);
    }

    bool hadobject = HasDataObjectType(STEPSIMULATION);
    if(bMoveTypeStepSim)
    {
        if(!hadobject)
        {
            CreateDataObject(STEPSIMULATION);
        }
    } else
    {
        if(hadobject)
        {
            DestroyDataObject(STEPSIMULATION);
        }
    }
}

void IBaseEntity::CheckHasGamePhysicsSimulation()
{
    bool isSimulating = WillSimulateGamePhysics();
    if(isSimulating != IsEFlagsSet(EFL_NO_GAME_PHYSICS_SIMULATION))
    {
        return;
    }

    if(isSimulating)
    {
        RemoveEFlags(EFL_NO_GAME_PHYSICS_SIMULATION);
    }
    else
    {
        AddEFlags(EFL_NO_GAME_PHYSICS_SIMULATION);
    }
    g_CallHelper->SimThink_EntityChanged((CBaseEntity *)this);
}

bool IBaseEntity::WillSimulateGamePhysics()
{
    if(!IsPlayer())
    {
        if(m_MoveType == MOVETYPE_NONE || m_MoveType == MOVETYPE_VPHYSICS)
        {
            return false;
        }
        if(m_MoveType == MOVETYPE_PUSH && GetMoveDoneTime() <= 0)
        {
            return false;
        }
    }
    return true;
}

void *IBaseEntity::CreateDataObject(int type)
{
    return g_CallHelper->CreateDataObjects((CBaseEntity *)this, type);
}

void IBaseEntity::DestroyDataObject(int type)
{
    g_CallHelper->DestroyDataObjects((CBaseEntity *)this, type);
}

void IBaseEntity::AddEffects(int nEffects)
{
    m_fEffects |= nEffects;
    if(nEffects & EF_NODRAW)
    {
        DispatchUpdateTransmitState();
    }
}

inline bool IsEntityPositionReasonable( const Vector &v )
{
	float r = (16384.0f);
	return
		v.x > -r && v.x < r &&
		v.y > -r && v.y < r &&
		v.z > -r && v.z < r;
}

inline bool IsEntityQAngleReasonable( const QAngle &q )
{
	float r = 360.0 * 1000.0f;
	return
		q.x > -r && q.x < r &&
		q.y > -r && q.y < r &&
		q.z > -r && q.z < r;
}

static double s_LastEntityReasonableEmitTime;
bool CheckEmitReasonablePhysicsSpew()
{
	double now = Plat_FloatTime();
	if ( now >= s_LastEntityReasonableEmitTime && now < s_LastEntityReasonableEmitTime + 5.0 )
	{
		return false;
	}

	s_LastEntityReasonableEmitTime = now;
	return true;
}

void IBaseEntity::SetLocalOrigin(const Vector &origin)
{
    if(!IsEntityPositionReasonable(origin))
    {
        if(CheckEmitReasonablePhysicsSpew())
        {
            Msg( "Bad SetLocalOrigin(%f,%f,%f) on %s\n", origin.x, origin.y, origin.z, GetDebugName() );
        }
        return;
    }

    if(m_vecOrigin != origin)
    {
        InvalidatePhysicsRecursive(POSITION_CHANGED);
        m_vecOrigin = origin;
        SetSimulationTime(g_pGlobals->curtime);
    }
}

void IBaseEntity::SetLocalAngles(const QAngle &angles)
{
    if(!IsEntityQAngleReasonable(angles))
    {
        if(CheckEmitReasonablePhysicsSpew())
        {
			Msg( "Bad SetLocalAngles(%f,%f,%f) on %s\n", angles.x, angles.y, angles.z, GetDebugName() );
        }
        return;
    }

    if(m_angRotation != angles)
    {
        InvalidatePhysicsRecursive(ANGLES_CHANGED);
        m_angRotation = angles;
        SetSimulationTime(g_pGlobals->curtime);
    }
}

void IBaseEntity::SetLocalVelocity(const Vector &inVecVelocity)
{
    Vector vecVelocity = inVecVelocity;
    switch (CheckEntityVelocity(vecVelocity))
    {
    case -1:
		Msg( "Discarding SetLocalVelocity(%f,%f,%f) on %s\n", vecVelocity.x, vecVelocity.y, vecVelocity.z, GetDebugName() );
        break;
    
    case 0:
        if ( CheckEmitReasonablePhysicsSpew() )
        {
            Msg( "Clamping SetLocalVelocity(%f,%f,%f) on %s\n", inVecVelocity.x, inVecVelocity.y, inVecVelocity.z, GetDebugName() );
        }
        break;
    }

    if(m_vecVelocity != vecVelocity)
    {
        InvalidatePhysicsRecursive(VELOCITY_CHANGED);
        m_vecVelocity = vecVelocity;
    }
}

bool IBaseEntity::IsCurrentlyTouching(void)
{
    if(HasDataObjectType(TOUCHLINK))
    {
        return true;
    }

    return false;
}

void IBaseEntity::InvalidatePhysicsRecursive(int nChangeFlags)
{
    int nDirtyFlags = 0;
    if(nChangeFlags & VELOCITY_CHANGED)
    {
        nDirtyFlags |= EFL_DIRTY_ABSVELOCITY;
    }

    if(nChangeFlags & POSITION_CHANGED)
    {
        nDirtyFlags |= EFL_DIRTY_ABSTRANSFORM;
        NetworkProp()->MarkPVSInformationDirty();
        CollisionProp()->MarkPartitionHandleDirty();
    }

    if(nChangeFlags & ANGLES_CHANGED)
    {
        nDirtyFlags |= EFL_DIRTY_ABSTRANSFORM;
        if(CollisionProp()->DoesRotationInvalidateSurroundingBox())
        {
            CollisionProp()->MarkPartitionHandleDirty();
        }
        nChangeFlags |= POSITION_CHANGED | VELOCITY_CHANGED;
    }
    AddEFlags(nDirtyFlags);

	bool bOnlyDueToAttachment = false;
	if ( nChangeFlags & ANIMATION_CHANGED )
	{
		if ( !( nChangeFlags & (POSITION_CHANGED | VELOCITY_CHANGED | ANGLES_CHANGED) ) )
		{
			bOnlyDueToAttachment = true;
		}

		nChangeFlags = POSITION_CHANGED | ANGLES_CHANGED | VELOCITY_CHANGED;
	}
	for (IBaseEntity *pChild = FirstMoveChild(); pChild; pChild = pChild->NextMovePeer())
	{
		if ( bOnlyDueToAttachment )
		{
			if ( pChild->GetParentAttachment() == 0 )
				continue;
		}
		pChild->InvalidatePhysicsRecursive( nChangeFlags );
	}
}

void IBaseEntity::SetCheckUntouch(bool check)
{
    if(check)
    {
        touchStamp++;
        if(!IsEFlagsSet(EFL_CHECK_UNTOUCH))
        {
            AddEFlags(EFL_CHECK_UNTOUCH);
            g_CallHelper->EntityTouchAdd((CBaseEntity *)this);
        }
    }
    else
    {
        RemoveEFlags(EFL_CHECK_UNTOUCH);
    }
}

void IBaseEntity::StopFollowingEntity()
{
    if(!IsFollowingEntity())
    {
        return;
    }

    SetParent(nullptr);
    RemoveEffects(EF_BONEMERGE);
    RemoveSolidFlags(FSOLID_NOT_SOLID);    
    SetMoveType(MOVETYPE_NONE);
    CollisionRulesChanged();
}

bool IBaseEntity::IsFollowingEntity()
{
    return IsEffectActive(EF_BONEMERGE) && (GetMoveType() == MOVETYPE_NONE) && GetMoveParent();
}

IBaseEntity *IBaseEntity::GetFollowedEntity()
{
    if(!IsFollowingEntity())
        return nullptr;
    return GetMoveParent();
}

void IBaseEntity::RemoveEffects(int nEffencts)
{
    m_fEffects &= ~nEffencts;
    if(nEffencts & EF_NODRAW)
    {
        NetworkProp()->MarkPVSInformationDirty();
        DispatchUpdateTransmitState();
    }
}

const char *IBaseEntity::TeamID(void) const
{
    if(GetTeam() == nullptr)
        return "";

    return GetTeam()->GetName();
}

IBaseEntity *IBaseEntity::GetRootMoveParent()
{
    IBaseEntity *pEntity = this;
    IBaseEntity *pParent = this->GetMoveParent();

    while (pParent)
    {
        pEntity = pParent;
        pParent = pEntity->GetMoveParent();
    }
    
    return pEntity;
}

bool IBaseEntity::IsInWorld(void) const
{
    if(!edict())
        return true;

    if(GetAbsOrigin().x >= MAX_COORD_INTEGER) return false;
    if(GetAbsOrigin().y >= MAX_COORD_INTEGER) return false;
    if(GetAbsOrigin().z >= MAX_COORD_INTEGER) return false;
    if(GetAbsOrigin().x <= MIN_COORD_INTEGER) return false;
    if(GetAbsOrigin().y <= MIN_COORD_INTEGER) return false;
    if(GetAbsOrigin().z <= MIN_COORD_INTEGER) return false;

    if(GetAbsVelocity().x >= 2000) return false;
    if(GetAbsVelocity().y >= 2000) return false;
    if(GetAbsVelocity().z >= 2000) return false;
    if(GetAbsVelocity().x <= -2000) return false;
    if(GetAbsVelocity().y <= -2000) return false;
    if(GetAbsVelocity().z <= -2000) return false;

    return true;
}

void IBaseEntity::EmitSound(const char *szSoundName, float soundTime, float *duration)
{
    CPASAttenuationFilter filter(this, szSoundName);

    EmitSound_t params;
    params.m_pSoundName = szSoundName;
    params.m_flSoundTime = soundTime;
    params.m_pflSoundDuration = duration;
    params.m_bWarnOnDirectWaveReference = true;

    g_CallHelper->CBaseEntity_EmitSound(filter, entindex(), params);
}

void IBaseEntity::EmitSound(const char *szSoundName, HSOUNDSCRIPTHANDLE &handle, float soundtime, float *duration)
{
    CPASAttenuationFilter filter(this, szSoundName, handle);
    EmitSound_t params;
    params.m_pSoundName = szSoundName;
    params.m_flSoundTime = soundtime;
    params.m_pflSoundDuration = duration;
    params.m_bWarnOnDirectWaveReference = true;

    g_CallHelper->CBaseEntity_EmitSound(filter, entindex(), params, handle);
}

void IBaseEntity::EmitSound(IRecipientFilter &filter, int iEntIndex, const char *szSoundName, const Vector *pOrigin, float soundtime, float *duration)
{
    if(!szSoundName)
        return;

    EmitSound_t params;
	params.m_pSoundName = szSoundName;
	params.m_flSoundTime = soundtime;
	params.m_pOrigin = pOrigin;
	params.m_pflSoundDuration = duration;
	params.m_bWarnOnDirectWaveReference = true;

    g_CallHelper->CBaseEntity_EmitSound(filter, iEntIndex, params, params.m_hSoundScriptHandle);
}

void IBaseEntity::EmitSound(IRecipientFilter &filter, int iEntIndex, const char *szSoundName, HSOUNDSCRIPTHANDLE &handle, const Vector *pOrigin, float soundtime, float *duration)
{
	EmitSound_t params;
	params.m_pSoundName = szSoundName;
	params.m_flSoundTime = soundtime;
	params.m_pOrigin = pOrigin;
	params.m_pflSoundDuration = duration;
	params.m_bWarnOnDirectWaveReference = true;

    g_CallHelper->CBaseEntity_EmitSound(filter, iEntIndex, params, handle);
}

bool IBaseEntity::BlocksLOS( void ) 
{ 
	return !IsEFlagSet(EFL_DONTBLOCKLOS); 
}

groundlink_t *IBaseEntity::AddEntityToGroundList(IBaseEntity *other)
{
    return (groundlink_t *)g_CallHelper->AddEntityToGroundList(this, other);
}

void IBaseEntity::PhysicsNotifyOtherOfGroundRemoval(IBaseEntity *ent, IBaseEntity *other)
{
	if ( !other )
		return;

	groundlink_t *root = ( groundlink_t * )g_CallHelper->GetDataObject((CBaseEntity *)other, GROUNDLINK );
	if ( root )
	{
		groundlink_t *link = root->nextLink;
		while ( link != root )
		{
			if ( link->entity == (CBaseEntity *)ent )
			{
				g_CallHelper->PhysicsRemoveGround( other, link );

				if (root->nextLink == root && 
                    root->prevLink == root )
				{
                    if((*((DWORD *)other + 872) & 1) != 0)
                        g_CallHelper->DestroyDataObjects((CBaseEntity *)other, GROUNDLINK );
				}
				return;
			}

			link = link->nextLink;
		}
	}
}

void IBaseEntity::FollowEntity(IBaseEntity *pEntity, bool bBoneMerge)
{
    if(pEntity)
    {
        SetParent((CBaseEntity *)pEntity);
        SetMoveType(MOVETYPE_NONE);
        if(bBoneMerge)
            AddEffects(EF_BONEMERGE);
        
        AddSolidFlags(FSOLID_NOT_SOLID);
        SetLocalOrigin(vec3_origin);
        SetLocalAngles(vec3_angle);
    }
    else
    {
        StopFollowingEntity();
    }
}

void IBaseEntity::NetworkStateChanged()
{
    NetworkProp()->NetworkStateChanged();
}

void IBaseEntity::NetworkStateChanged(unsigned short varOffset)
{
    NetworkProp()->NetworkStateChanged(varOffset);
}

void IBaseEntity::NetworkStateChanged(void *pVar)
{
    NetworkProp()->NetworkStateChanged((char*)pVar - (char*)this);
}

void IBaseEntity::SetParent(string_t newParent, IBaseEntity *pActivator, int iAttachment)
{
    IBaseEntity *pParent = g_CallHelper->FindEntityByName(nullptr, newParent.ToCStr(), NULL, pActivator);
    if(newParent != NULL_STRING && pParent == nullptr)
    {
        Msg("[ERROR] Entity %s(%s) has bad parent %s\n", GetClassname().ToCStr(), GetDebugName(), newParent.ToCStr());
    }
    else
    {
        if(g_CallHelper->FindEntityByName(pParent, newParent.ToCStr(), NULL, pActivator))
        {
            Msg("[ERROR] Entity %s(%s) is ambiguously parented to %s, because there is more than one entity by that name.\n", GetClassname().ToCStr(), GetDebugName(), newParent.ToCStr());
        }
        SetParent((CBaseEntity *)pParent, iAttachment);
    }
}

void IBaseEntity::SetSimulationTime(float st)
{
    m_flSimulationTime = st;
}

bool NamesMatchs( const char *pszQuery, string_t nameToMatch )
{
	if ( nameToMatch == NULL_STRING )
		return (!pszQuery || *pszQuery == 0 || *pszQuery == '*');

	const char *pszNameToMatch = STRING(nameToMatch);

	if ( pszNameToMatch == pszQuery )
		return true;

	while ( *pszNameToMatch && *pszQuery )
	{
		unsigned char cName = *pszNameToMatch;
		unsigned char cQuery = *pszQuery;
		if ( cName == cQuery )
			;
		else if ( cName - 'A' <= (unsigned char)'Z' - 'A' && cName - 'A' + 'a' == cQuery )
			;
		else if ( cName - 'a' <= (unsigned char)'z' - 'a' && cName - 'a' + 'A' == cQuery )
			;
		else
			break;
		++pszNameToMatch;
		++pszQuery;
	}

	if ( *pszQuery == 0 && *pszNameToMatch == 0 )
		return true;

	if ( *pszQuery == '*' )
		return true;

	return false;
}

bool IBaseEntity::ClassMatches(const char *strClassOrWildcard)
{
    string_t szName = GetClassname();
    if(IDENT_STRINGS(szName, strClassOrWildcard))
    {
        return true;
    }

    return NamesMatchs(strClassOrWildcard, szName);
}

void IBaseEntity::MoveDone( void )
{
    if (m_pfnMoveDone)
        (this->*m_pfnMoveDone)();
}

void IBaseEntity::Think( void )
{
    if (m_pfnThink)
        (this->*m_pfnThink)();
}

float CountdownTimers::Now( void ) const
{
	return g_pGlobals->curtime;
}

float IntervalTimers::Now(void) const
{
    return g_pGlobals->curtime;
}

void IBaseEntity::SetModelName(string_t name)
{
    m_ModelName = name;
    DispatchUpdateTransmitState();
}

void IBaseEntity::SetEffectEntity(IBaseEntity *pEffectEnt)
{
    if(m_hEffectEntity.Get() != pEffectEnt)
    {
        m_hEffectEntity = pEffectEnt;
    }
}

bool IBaseEntity::VPhysicsInitSetup()
{
    if(!edict() || IsMarkedForDeletion())
        return false;

    VPhysicsDestroyObject();
    return true;
}

IPhysicsObject *IBaseEntity::VPhysicsInitNormal( SolidType_t solidType, int nSolidFlags, bool createAsleep, solid_t *pSolid )
{
    if(!VPhysicsInitSetup())
        return nullptr;

    SetSolid(solidType);
    SetSolidFlags(nSolidFlags);

    if(solidType == SOLID_NONE)
    {
        return nullptr;
    }

    Vector vecOrig = GetAbsOrigin();
    QAngle vecAng = GetAbsAngles();

    IPhysicsObject* pPhysObject = g_CallHelper->PhysModelCreate(this, GetModelIndex(), &vecOrig, &vecAng, pSolid);
    if(pPhysObject)
    {
        VPhysicsSetObject(pPhysObject);
        SetMoveType(MOVETYPE_VPHYSICS);
        if(!createAsleep)
        {
            pPhysObject->Wake();
        }
    }

    return pPhysObject;
}

IPhysicsObject *IBaseEntity::VPhysicsInitShadow(bool allowPhysicsMovement, bool allowPhysicsRotation, solid_t *pSolid)
{
    if(!VPhysicsInitSetup())
        return nullptr;

    if(GetSolid() == SOLID_NONE)
        return nullptr;

    const Vector& origin = GetAbsOrigin();
    QAngle angles = GetAbsAngles();
    IPhysicsObject* pPhysicsObject = nullptr;

    if(GetSolid() == SOLID_BBOX)
    {
        float radius = 0.25f - 0.03125f;
        Vector mins = WorldAlignMins() + Vector(radius, radius, radius);
        Vector maxs = WorldAlignMaxs() - Vector(radius, radius, radius);
        pPhysicsObject = g_CallHelper->PhysModelCreateBox(this, mins, maxs, origin, false);
        angles = vec3_angle;
    }
    else if( GetSolid() == SOLID_OBB)
    {
        pPhysicsObject = g_CallHelper->PhysModelCreateOBB(this, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), origin, angles, false);
    }
    else
    {
        pPhysicsObject = g_CallHelper->PhysModelCreate(this, GetModelIndex(), &origin, &angles, pSolid);
    }

    if(!pPhysicsObject)
        return nullptr;

    VPhysicsSetObject(pPhysicsObject);
    pPhysicsObject->SetShadow(1e4, 1e4, allowPhysicsMovement, allowPhysicsRotation);
    pPhysicsObject->UpdateShadow(origin, angles, false, 0);
    return pPhysicsObject;
}

void IBaseEntity::VPhysicsSetObject( IPhysicsObject *pPhysics )
{
	if ( m_pPhysicsObject && pPhysics )
	{
		Warning( "Overwriting physics object for %s\n", GetClassname().ToCStr() );
	}
	m_pPhysicsObject = pPhysics;
	if ( pPhysics && !m_pPhysicsObject )
	{
		CollisionRulesChanged();
	}
}

inline bool AnyPlayersInHierarchy_R( IBaseEntity *pEnt )
{
	if ( pEnt->IsPlayer() )
		return true;

	for ( IBaseEntity *pCur = pEnt->FirstMoveChild(); pCur; pCur=pCur->NextMovePeer() )
	{
		if ( AnyPlayersInHierarchy_R( pCur ) )
			return true;
	}
	
	return false;	
}

void IBaseEntity::RecalcHasPlayerChildBit()
{
	if ( AnyPlayersInHierarchy_R( this ) )
		AddEFlags( EFL_HAS_PLAYER_CHILD );
	else
		RemoveEFlags( EFL_HAS_PLAYER_CHILD );
}

void IBaseEntity::SetCollisionGroup( int collisionGroup )
{
	if ( (int)m_CollisionGroup != collisionGroup )
	{
		m_CollisionGroup = collisionGroup;
		CollisionRulesChanged();
	}
}

void IBaseEntity::SetNextThink(float thinkTime, const char* szContext)
{
    int thinkTick = ( thinkTime == TICK_NEVER_THINK ) ? TICK_NEVER_THINK : ( (int)( 0.5f + (float)(thinkTime) / (g_pGlobals->interval_per_tick) ) );

    int iIndex = 0;
    if(!szContext)
    {
        m_nNextThinkTick = thinkTick;
        CheckHasThinkFunction(thinkTick == TICK_NEVER_THINK ? false : true);
        return;
    }
    else
    {
        iIndex = GetIndexForThinkContext(szContext);
        if(iIndex == NO_THINK_CONTEXT)
        {
            iIndex = RegisterThinkContext(szContext);
        }
    }

    m_aThinkFunctions[iIndex].m_nNextThinkTick = thinkTick;
    CheckHasThinkFunction(thinkTick == TICK_NEVER_THINK ? false : true);
}

void IBaseEntity::CheckHasThinkFunction( bool isThinking)
{
    if(IsEFlagSet(EFL_NO_THINK_FUNCTION) && isThinking)
    {
        RemoveEFlags(EFL_NO_THINK_FUNCTION);
    }
    else if(!isThinking && !IsEFlagSet(EFL_NO_THINK_FUNCTION) && !WillThink())
    {
        AddEFlags(EFL_NO_THINK_FUNCTION);
    }

    g_CallHelper->SimThink_EntityChanged((CBaseEntity *)this);
}

bool IBaseEntity::WillThink()
{
    if(m_nNextThinkTick > 0)
        return true;

    for(int i = 0; i < m_aThinkFunctions.Count(); i++)
    {
        if(m_aThinkFunctions[i].m_nNextThinkTick > 0)
            return true;
    }

    return false;
}

int	IBaseEntity::GetIndexForThinkContext( const char *pszContext )
{
	for ( int i = 0; i < m_aThinkFunctions.Size(); i++ )
	{
		if ( !Q_strncmp( STRING( m_aThinkFunctions[i].m_iszContext ), pszContext, MAX_CONTEXT_LENGTH ) )
			return i;
	}

	return NO_THINK_CONTEXT;
}

int IBaseEntity::RegisterThinkContext(const char* szContext)
{
    int iIndex = GetIndexForThinkContext(szContext);
    if(iIndex != NO_THINK_CONTEXT)
        return iIndex;

    thinkfunc_t sNewFunc;
    Q_memset(&sNewFunc, 0, sizeof(sNewFunc));
    sNewFunc.m_pfnThink = nullptr;
    sNewFunc.m_nNextThinkTick = 0;
    sNewFunc.m_iszContext = g_CallHelper->AllocPooledString(szContext);

    return m_aThinkFunctions.AddToTail(sNewFunc);
}

BASEPTR	IBaseEntity::ThinkSet( BASEPTR func, float flNextThinkTime, const char *szContext )
{
    if(!szContext)
    {
        m_pfnThink = func;
        return m_pfnThink;
    }

    int iIndex = GetIndexForThinkContext(szContext);
    if(iIndex == NO_THINK_CONTEXT)
    {
        iIndex = RegisterThinkContext(szContext);
    }

    m_aThinkFunctions[iIndex].m_pfnThink = func;

    if(flNextThinkTime != 0)
    {
        int thinkTick = ( flNextThinkTime == TICK_NEVER_THINK ) ? TICK_NEVER_THINK : ( (int)( 0.5f + (float)(flNextThinkTime) / (g_pGlobals->interval_per_tick) ) );
        m_aThinkFunctions[iIndex].m_nNextThinkTick = thinkTick;
        CheckHasThinkFunction(thinkTick == TICK_NEVER_THINK ? false : true);
    }
    return func;
}

int IBaseEntity::PrecacheModel(const char *name)
{
	if ( !name || !*name )
	{
		Msg( "Attempting to precache model, but model name is NULL\n");
		return -1;
	}

	// Warn on out of order precache
	// if ( !CBaseEntity::IsPrecacheAllowed() )
	{
		if ( !engine->IsModelPrecached( name ) )
		{
			Assert( !"CBaseEntity::PrecacheModel:  too late" );
			Warning( "Late precache of %s\n", name );
		}
	}

	int idx = engine->PrecacheModel( name, true );
	if ( idx != -1 )
	{
		// PrecacheModelComponents( idx );
	}

	return idx;
}

void IBaseEntity::DrawOutputOverlay(CEventAction *ev)
{
	char bigstring[1024];
	if ( ev->m_flDelay )
	{
		Q_snprintf( bigstring,sizeof(bigstring), "%3.1f  (%s) --> (%s),%.1f) \n", g_pGlobals->curtime, STRING(ev->m_iTargetInput), STRING(ev->m_iTarget), ev->m_flDelay);
	}
	else
	{
		Q_snprintf( bigstring,sizeof(bigstring), "%3.1f  (%s) --> (%s)\n", g_pGlobals->curtime,  STRING(ev->m_iTargetInput), STRING(ev->m_iTarget));
	}
	AddTimedOverlay(bigstring, 10.0);

	// Now print to the console
	if ( ev->m_flDelay )
	{
		DevMsg( 2, "output: (%s,%s) -> (%s,%s,%.1f)\n", STRING(m_iClassname), GetDebugName(), STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), ev->m_flDelay );
	}
	else
	{
		DevMsg( 2, "output: (%s,%s) -> (%s,%s)\n", STRING(m_iClassname), GetDebugName(), STRING(ev->m_iTarget), STRING(ev->m_iTargetInput) );
	}
}

void IBaseEntity::AddTimedOverlay(const char *msg, int endTime)
{
	TimedOverlay_t *pNewTO = new TimedOverlay_t;
	int len = strlen(msg);
	pNewTO->msg = new char[len + 1];
	Q_strncpy(pNewTO->msg,msg, len+1);
	pNewTO->msgEndTime = g_pGlobals->curtime + endTime;
	pNewTO->msgStartTime = g_pGlobals->curtime;
	pNewTO->pNextTimedOverlay = m_pTimedOverlay;
	m_pTimedOverlay = pNewTO;
}

IBaseEntity* CreateNoSpawn(const char* szName, const Vector &vecOrigion, const QAngle &vecAngles, IBaseEntity* pOwner)
{
    IBaseEntity *pEntity = (IBaseEntity*)servertools->CreateEntityByName(szName);
    if(!pEntity) return nullptr;

    pEntity->SetLocalOrigin(vecOrigion);
    pEntity->SetLocalAngles(vecAngles);
    pEntity->SetOwnerEntity((CBaseEntity*)pOwner);

    return pEntity;
}

IBaseEntity *CreateSpawn(const char* szName, const Vector &vecOrigion, const QAngle &vecAngles, IBaseEntity* pOwner)
{
    IBaseEntity* pEntity = CreateNoSpawn(szName, vecOrigion, vecAngles, pOwner);
    servertools->DispatchSpawn(pEntity);
    return pEntity;
}

void IBaseEntity::SetFadeDistance( float minFadeDist, float maxFadeDist )
{
	m_fadeMinDist = minFadeDist;
	m_fadeMaxDist = maxFadeDist;
}

void IBaseEntity::SetGlobalFadeScale( float flFadeScale )
{
	m_flFadeScale = flFadeScale;
}

float IBaseEntity::GetGlobalFadeScale() const
{
    return m_flFadeScale;
}
