#include "INextBotChasePath.h"
#include "Interface/IBaseCombatCharacter.h"

inline bool CloseEnough( const Vector &a, const Vector &b, float epsilon = EQUAL_EPSILON )
{
	return fabs( a.x - b.x ) <= epsilon &&
		fabs( a.y - b.y ) <= epsilon &&
		fabs( a.z - b.z ) <= epsilon;
}

Vector IChasePath::PredictSubjectPosition(INextBot *bot, IBaseEntity *subject) const
{
	ILocomotion *mover = bot->GetLocomotionInterface();

	const Vector &subjectPos = subject->GetAbsOrigin();

	Vector to = subjectPos - bot->GetPosition();
	to.z = 0.0f;
	float flRangeSq = to.LengthSqr();

	// don't lead if subject is very far away
	float flLeadRadiusSq = GetLeadRadius();
	flLeadRadiusSq *= flLeadRadiusSq;
	if ( flRangeSq > flLeadRadiusSq )
		return subjectPos;

	// Normalize in place
	float range = sqrt( flRangeSq );
	to /= ( range + 0.0001f );	// avoid divide by zero

	// estimate time to reach subject, assuming maximum speed
	float leadTime = 0.5f + ( range / ( mover->GetRunSpeed() + 0.0001f ) );
	
	// estimate amount to lead the subject	
	Vector lead = leadTime * subject->GetAbsVelocity();
	lead.z = 0.0f;

	if ( DotProduct( to, lead ) < 0.0f )
	{
		// the subject is moving towards us - only pay attention 
		// to his perpendicular velocity for leading
		Vector2D to2D = to.AsVector2D();
		to2D.NormalizeInPlace();

		Vector2D perp( -to2D.y, to2D.x );

		float enemyGroundSpeed = lead.x * perp.x + lead.y * perp.y;

		lead.x = enemyGroundSpeed * perp.x;
		lead.y = enemyGroundSpeed * perp.y;
	}

	// compute our desired destination
	Vector pathTarget = subjectPos + lead;

	// validate this destination

	// don't lead through walls
	if ( lead.LengthSqr() > 36.0f )
	{
		float fraction;
		if ( !mover->IsPotentiallyTraversable( subjectPos, pathTarget, ILocomotion::IMMEDIATELY, &fraction ) )
		{
			// tried to lead through an unwalkable area - clip to walkable space
			pathTarget = subjectPos + fraction * ( pathTarget - subjectPos );
		}
	}

    INavMesh *pMesh = g_HL2->GetTheNavMesh();
    if(pMesh)
    {
        // don't lead over cliffs
        INavArea *leadArea = NULL;
		IBaseCombatCharacter *pBCC = (IBaseCombatCharacter*)subject->MyCombatCharacterPointer();
		if ( pBCC && CloseEnough( pathTarget, subjectPos, 3.0 ) )
		{
			pathTarget = subjectPos;
			leadArea = (INavArea*)pBCC->GetLastKnownArea(); // can return null?
		}
		else
		{
			struct CacheEntry_t
			{
				CacheEntry_t() : pArea(NULL) {}
				Vector target;
				INavArea *pArea;
			};

			static int iServer;
			static CacheEntry_t cache[4];
			static int iNext;
			unsigned int i;

			bool bFound = false;
			if ( iServer != g_pGlobals->serverCount )
			{
				for ( i = 0; i < ARRAYSIZE(cache); i++ )
				{
					cache[i].pArea = NULL;
				}
				iServer = g_pGlobals->serverCount;
			}
			else
			{
				for ( i = 0; i < ARRAYSIZE(cache); i++ )
				{
					if ( cache[i].pArea && CloseEnough( cache[i].target, pathTarget, 2.0 ) )
					{
						pathTarget = cache[i].target;
						leadArea = cache[i].pArea;
						bFound = true;
						break;
					}
				}
			}

			if ( !bFound )
			{
				leadArea = pMesh->GetNearestNavArea( pathTarget );
				if ( leadArea )
				{
					cache[iNext].target = pathTarget;
					cache[iNext].pArea = leadArea;
					iNext = ( iNext + 1 ) % ARRAYSIZE( cache );
				}
			}
		}
        if ( !leadArea || leadArea->GetZ( pathTarget.x, pathTarget.y ) < pathTarget.z - mover->GetMaxJumpHeight() )
        {
            // would fall off a cliff
            return subjectPos;		
        }
    }
    else
    {
        DevMsg("[IChasePath::PredictSubjectPosition] INavMesh is null\n");
    }
	/** This needs more thought - it is preventing bots from using dropdowns
	float fraction = 0.f;
	if ( mover->HasPotentialGap( subjectPos, pathTarget, &fraction ) )
	{
		// tried to lead over a cliff - clip to safe region
		pathTarget = subjectPos + fraction * ( pathTarget - subjectPos );
	}
	*/
	return pathTarget;
}

void DirectChasePath::NotifyVictim( INextBot *me, IBaseEntity *victim )
{
	IBaseCombatCharacter *pBCCVictim = victim ? (IBaseCombatCharacter*)victim->MyCombatCharacterPointer() : nullptr;
	if ( !pBCCVictim )
		return;
	
	pBCCVictim->OnPursuedBy( me );
}