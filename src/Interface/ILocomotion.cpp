#include "ILocomotion.h"
#include "INextBot.h"
#include "INavArea.h"
#include "../CTrace.h"

class NextBotTraversableTraceFilter : public CTraceFilterSimples
{
public:
	NextBotTraversableTraceFilter( INextBot *bot, ILocomotion::TraverseWhenType when = ILocomotion::EVENTUALLY ) : 
    CTraceFilterSimples( reinterpret_cast<IBaseEntity*>(bot->GetEntity()), COLLISION_GROUP_NONE )
	{
		m_bot = bot;
		m_when = when;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		IBaseEntity *entity = GetEntityFromEntityHandle( pServerEntity );

		if ( m_bot->IsSelf( (CBaseCombatCharacter*)entity ) )
		{
			return false;
		}

		if ( CTraceFilterSimples::ShouldHitEntity( pServerEntity, contentsMask ) )
		{
			return !m_bot->GetLocomotionInterface()->IsEntityTraversable( (CBaseEntity *)entity, m_when );
		}

		return false;
	}

private:
    bool IsStaticProp( CBaseHandle handle )
    {
        return (handle.GetSerialNumber() == (0x40000000 >> NUM_ENT_ENTRY_BITS));
    }

    IBaseEntity *GetEntityFromEntityHandle( IHandleEntity *pHandleEntity )
    {
        IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
        IBaseEntity *pEnt = reinterpret_cast<IBaseEntity*>(pUnk->GetBaseEntity());
        if (IsStaticProp(pEnt->GetRefEHandle()))
            return nullptr;

        return pEnt;
    }


	INextBot *m_bot;
	ILocomotion::TraverseWhenType m_when;
};

void ILocomotion::ClearStuckStatus(const char *reason)
{
    if ( IsStuck() )
	{
		m_isStuck = false;
		GetBot()->OnUnStuck();
	}

	m_stuckPos = GetFeet();
	m_stuckTimer.Start();
}

bool ILocomotion::IsAttemptingToMove(void) const
{
    return m_moveRequestTimer.HasStarted() && m_moveRequestTimer.GetElapsedTime() < 0.25f;
}

void ILocomotion::AdjustPosture(const Vector &moveGoal)
{
	IBody *body = GetBot()->GetBodyInterface();
	if ( !body->IsActualPosture( IBody::STAND ) && !body->IsActualPosture( IBody::CROUCH ) )
		return;

	const Vector &mins = body->GetHullMins() + Vector( 0, 0, GetStepHeight() );

	const float halfSize = body->GetHullWidth()/2.0f;
	Vector standMaxs( halfSize, halfSize, body->GetStandHullHeight() );

	trace_t trace;
	NextBotTraversableTraceFilter filter( GetBot(), ILocomotion::IMMEDIATELY );

	const Vector &groundNormal = GetGroundNormal();
	const Vector &feet = GetFeet();
	Vector moveDir = moveGoal - feet;
	float moveLength = moveDir.NormalizeInPlace();
	Vector left( -moveDir.y, moveDir.x, 0.0f );
	Vector goal = feet + moveLength * CrossProduct( left, groundNormal ).Normalized();

	util_TraceHull( feet, goal, mins, standMaxs, body->GetSolidMask(), &filter, &trace );

	if ( trace.fraction >= 1.0f && !trace.startsolid )
	{
		if ( body->IsActualPosture( IBody::CROUCH ) )
		{
			body->SetDesiredPosture( IBody::STAND );
		}
		return;
	}

	if ( body->IsActualPosture( IBody::CROUCH ) )
		return;

	Vector crouchMaxs( halfSize, halfSize, body->GetCrouchHullHeight() );

	util_TraceHull( feet, goal, mins, crouchMaxs, body->GetSolidMask(), &filter, &trace );

	if ( trace.fraction >= 1.0f && !trace.startsolid )
	{
		body->SetDesiredPosture( IBody::CROUCH );
	}
}

void ILocomotion::Reset( void )
{
    INextBotComponent::Reset();

    m_motionVector = Vector(1.f, 0.f, 0.f);
    m_speed = 0.f;
    m_groundMotionVector = m_motionVector;
    m_groundSpeed = m_speed;

    m_moveRequestTimer.Invalidate();

    m_isStuck = false;
    m_stuckTimer.Invalidate();
    m_stuckPos = vec3_origin;
}

void ILocomotion::Update( void )
{
	const Vector &vel = GetVelocity();
	m_speed = vel.Length();
	m_groundSpeed = vel.AsVector2D().Length();

	const float velocityThreshold = 10.0f;
	if ( m_speed > velocityThreshold )
	{
		m_motionVector = vel / m_speed;
	}

	if ( m_groundSpeed > velocityThreshold )
	{
		m_groundMotionVector.x = vel.x / m_groundSpeed;
		m_groundMotionVector.y = vel.y / m_groundSpeed;
		m_groundMotionVector.z = 0.0f;
	}
}

void ILocomotion::Approach(const Vector &goalPos, float goalWeight)
{
    m_moveRequestTimer.Start();
}

void ILocomotion::DriveTo(const Vector &pos)
{
    m_moveRequestTimer.Start();
}

bool ILocomotion::IsPotentiallyTraversable(const Vector &from, const Vector &to, TraverseWhenType when, float *fraction) const
{
	if ( ( to.z - from.z ) > GetMaxJumpHeight() + 0.1f )
	{
		Vector along = to - from;
		along.NormalizeInPlace();
		if ( along.z > GetTraversableSlopeLimit() )
		{
			if ( fraction )
			{
				*fraction = 0.0f;
			}
			return false;
		}
	}

	trace_t result;
	NextBotTraversableTraceFilter filter( GetBot(), when );

	const float probeSize = 0.25f * GetBot()->GetBodyInterface()->GetHullWidth();
	const float probeZ = GetStepHeight();

	Vector hullMin( -probeSize, -probeSize, probeZ );
	Vector hullMax( probeSize, probeSize, GetBot()->GetBodyInterface()->GetCrouchHullHeight() );
	util_TraceHull( from, to, hullMin, hullMax, GetBot()->GetBodyInterface()->GetSolidMask(), &filter, &result );

	if ( fraction )
	{
		*fraction = result.fraction;
	}

	return ( result.fraction >= 1.0f ) && ( !result.startsolid );
}

bool ILocomotion::HasPotentialGap(const Vector &from, const Vector &tos, float *fraction) const
{
	float traversableFraction;
	IsPotentiallyTraversable( from, tos, IMMEDIATELY, &traversableFraction );

	Vector to = from + (( tos - from ) * traversableFraction);

	Vector forward = to - from;
	float length = forward.NormalizeInPlace();

	IBody *body = GetBot()->GetBodyInterface();

	float step = body->GetHullWidth()/2.0f;

	Vector pos = from;
	Vector delta = step * forward;
	for( float t = 0.0f; t < (length + step); t += step )
	{
		if ( IsGap( pos, forward ) )
		{
			if ( fraction )
			{
				*fraction = ( t - step ) / ( length + step );
			}
			
			return true;
		}

		pos += delta;		
	}

	if ( fraction )
	{
		*fraction = 1.0f;
	}

	return false;
}

bool ILocomotion::IsGap(const Vector &pos, const Vector &forward) const
{
	IBody *body = GetBot()->GetBodyInterface();
	const float halfWidth = 1.0f;
	const float hullHeight = 1.0f;

	unsigned int mask = ( body ) ? body->GetSolidMask() : MASK_PLAYERSOLID;

	trace_t ground;

	NextBotTraceFilterIgnoreActors filter( reinterpret_cast<IBaseEntity*>(GetBot()->GetEntity()), COLLISION_GROUP_NONE );

	util_TraceHull( pos + Vector( 0, 0, GetStepHeight() ),
					pos + Vector( 0, 0, -GetMaxJumpHeight() ), 
					Vector( -halfWidth, -halfWidth, 0 ), Vector( halfWidth, halfWidth, hullHeight ), 
					mask, &filter, &ground );

	return ( ground.fraction >= 1.0f && !ground.startsolid );
}

bool ILocomotion::IsEntityTraversable(CBaseEntity *obstacle, TraverseWhenType when) const
{
	if ( reinterpret_cast<IBaseEntity*>(obstacle)->IsWorld() )
		return false;

	if ( reinterpret_cast<IBaseEntity*>(obstacle)->ClassMatches("prop_door*" ) || 
    reinterpret_cast<IBaseEntity*>(obstacle)->ClassMatches("func_door*" ) )
	{
        int m_eDoorState = access_member<int>(obstacle, 1664);
		if ( m_eDoorState == 2 )
		{
			return false;
		}

		return true;
	}

	if ( reinterpret_cast<IBaseEntity*>(obstacle)->ClassMatches( "func_brush" ) )
	{		
		switch ( *((DWORD *)obstacle + 223) )
		{
			case 2:
				return false;
			case 1:
				return true;
			case 0:
				return true;
		}
	}

	if ( when == IMMEDIATELY )
	{
		return false;
	}

	return GetBot()->IsAbleToBreak( obstacle );
}

bool ILocomotion::IsAreaTraversable(const CNavArea *baseArea) const
{
	return true;
}

const Vector &ILocomotion::GetFeet(void) const
{
    return vec3_origin;
}
