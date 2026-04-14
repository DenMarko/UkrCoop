#include "INavLadder.h"
#include "INavArea.h"

class IsLadderFreeFunctor
{
public:
	IsLadderFreeFunctor( const INavLadder *ladder, const IBasePlayer *ignore )
	{
		m_ladder = ladder;
		m_ignore = ignore;
	}

	bool operator() ( IBasePlayer *player )
	{
		if (player == m_ignore)
			return true;

		if (!player->IsOnLadder())
			return true;

		// player is on a ladder - is it this one?
		const Vector &feet = player->GetAbsOrigin();

		if (feet.z > m_ladder->m_top.z + HalfHumanHeight)
			return true;

		if (feet.z + HumanHeight < m_ladder->m_bottom.z - HalfHumanHeight)
			return true;

		Vector2D away( m_ladder->m_bottom.x - feet.x, m_ladder->m_bottom.y - feet.y );
		const float onLadderRange = 50.0f;
		return away.IsLengthGreaterThan( onLadderRange );
	}

	const INavLadder *m_ladder;
	const IBasePlayer *m_ignore;
};


/*INavLadder::INavLadder()
{
    m_topForwardArea = NULL;
    m_topRightArea = NULL;
    m_topLeftArea = NULL;
    m_topBehindArea = NULL;
    m_bottomArea = NULL;

    // set an ID for interactive editing - loads will overwrite this
    m_id = m_nextID++;
}*/

bool INavLadder::IsUsableByTeam(int teamNumber) const
{
	if ( m_ladderEntity.Get() == NULL )
		return true;

	int ladderTeamNumber = m_ladderEntity->GetTeamNumber();
	return ( teamNumber == ladderTeamNumber || ladderTeamNumber == TEAM_UNASSIGNED );
}

IBaseEntity *INavLadder::GetLadderEntity(void) const
{
	return m_ladderEntity;
}

void INavLadder::FindLadderEntity(void)
{
	m_ladderEntity = g_CallHelper->FindEntityByClassNameNearest("func_simpleladder", (m_top + m_bottom) * 0.5f, HalfHumanWidth);
}

Vector INavLadder::GetPosAtHeight(float height) const
{
	if ( height < m_bottom.z )
	{
		return m_bottom;
	}

	if ( height > m_top.z )
	{
		return m_top;
	}

	if ( m_top.z == m_bottom.z )
	{
		return m_top;
	}

	float percent = ( height - m_bottom.z ) / ( m_top.z - m_bottom.z );

	return m_top * percent + m_bottom * ( 1.0f - percent );
}

bool INavLadder::IsConnected(const INavArea *area, Ladder_DirectionType dir) const
{
	if ( dir == LADDER_DOWN )
	{
		return area == m_bottomArea;
	}
	else if ( dir == LADDER_UP )
	{
		return ( area == m_topForwardArea ||
			area == m_topLeftArea ||
			area == m_topRightArea ||
			area == m_topBehindArea );
	}
	else
	{
		return ( area == m_bottomArea ||
			area == m_topForwardArea ||
			area == m_topLeftArea ||
			area == m_topRightArea ||
			area == m_topBehindArea );
	}
}

void INavLadder::ConnectTo(INavArea *area)
{
	float center = (m_top.z + m_bottom.z) * 0.5f;

	if (area->GetCenter().z > center)
	{
		// connect to top
		Nav_DirType dir;

		Vector dirVector = area->GetCenter() - m_top;
		if ( fabs( dirVector.x ) > fabs( dirVector.y ) )
		{
			if ( dirVector.x > 0.0f ) // east
			{
				dir = EAST;
			}
			else // west
			{
				dir = WEST;
			}
		}
		else
		{
			if ( dirVector.y > 0.0f ) // south
			{
				dir = SOUTH;
			}
			else // north
			{
				dir = NORTH;
			}
		}

		if ( m_dir == dir )
		{
			m_topBehindArea = area;
		}
		else if ( OppositeDirection( m_dir ) == dir )
		{
			m_topForwardArea = area;
		}
		else if ( DirectionLeft( m_dir ) == dir )
		{
			m_topLeftArea = area;
		}
		else
		{
			m_topRightArea = area;
		}
	}
	else
	{
		// connect to bottom
		m_bottomArea = area;
	}
}

void INavLadder::Disconnect(INavArea *area)
{
	if ( m_topForwardArea == area )
	{
		m_topForwardArea = NULL;
	}
	else if ( m_topLeftArea == area )
	{
		m_topLeftArea = NULL;
	}
	else if ( m_topRightArea == area )
	{
		m_topRightArea = NULL;
	}
	else if ( m_topBehindArea == area )
	{
		m_topBehindArea = NULL;
	}
	else if ( m_bottomArea == area )
	{
		m_bottomArea = NULL;
	}
}

void INavLadder::OnSplit(INavArea *original, INavArea *alpha, INavArea *beta)
{
	for ( int con=0; con<NUM_LADDER_CONNECTIONS; ++con )
	{
		INavArea ** areaConnection = GetConnection( (LadderConnectionType)con );

		if ( areaConnection && *areaConnection == original )
		{
			float alphaDistance = alpha->GetDistanceSquaredToPoint( m_top );
			float betaDistance = beta->GetDistanceSquaredToPoint( m_top );

			if ( alphaDistance < betaDistance )
			{
				*areaConnection = alpha;
			}
			else
			{
				*areaConnection = beta;
			}
		}
	}
}

void INavLadder::OnDestroyNotify(INavArea *dead)
{
	Disconnect( dead );
}

INavArea ** INavLadder::GetConnection( LadderConnectionType dir )
{
	switch ( dir )
	{
	case LADDER_TOP_FORWARD:
		return &m_topForwardArea;
	case LADDER_TOP_LEFT:
		return &m_topLeftArea;
	case LADDER_TOP_RIGHT:
		return &m_topRightArea;
	case LADDER_TOP_BEHIND:
		return &m_topBehindArea;
	case LADDER_BOTTOM:
		return &m_bottomArea;
	}

	return NULL;
}

void INavLadder::Shift(const Vector &shift)
{
	m_top += shift;
	m_bottom += shift;
}

template<typename Func>
bool ForEachPlayers(Func &func)
{
	auto FNullEnt = [](const edict_t* pent)
	{
		return pent == nullptr || g_Sample.IndexOfEdict(pent) == 0;
	};

	for(int i = 1; i <= g_pGlobals->maxClients; ++i)
	{
		IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(i);
		if(!pPlayer)
			continue;

		if(FNullEnt(pPlayer->edict()))
			continue;

		if(!pPlayer->IsPlayer())
			continue;

		if(!pPlayer->IsConnected())
			continue;

		if(!func(pPlayer))
		{
			return false;
		}
	}
	return true;
}


bool INavLadder::IsInUse(const IBasePlayer *ignore) const
{
	IsLadderFreeFunctor isLadderFree(this, ignore);
    return !ForEachPlayers(isLadderFree);
}
