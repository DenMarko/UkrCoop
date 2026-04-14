#ifndef _HEADER_NAV_LADDER_INCLUDE_
#define _HEADER_NAV_LADDER_INCLUDE_

#include "utlbuffer.h"
#include "ehandle.h"

enum Ladder_DirectionType
{
    LADDER_UP = 0,
    LADDER_DOWN,

    NUM_LADDER_DIRECTIONS
};

enum Nav_DirType
{
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,

	NUM_DIRECTIONS
};

//--------------------------------------------------------------------------------------------------------------
inline Nav_DirType AngleToDirection( float angle )
{
	while( angle < 0.0f )
		angle += 360.0f;

	while( angle > 360.0f )
		angle -= 360.0f;

	if (angle < 45 || angle > 315)
		return EAST;

	if (angle >= 45 && angle < 135)
		return SOUTH;

	if (angle >= 135 && angle < 225)
		return WEST;

	return NORTH;
}

class INavArea;
class IBasePlayer;
class IBaseEntity;

class INavLadder
{
public:
    //INavLadder();
    //~INavLadder() {}

	// void OnRoundRestart( void );			///< invoked when a game round restarts

	// void Save( CUtlBuffer &fileBuffer, unsigned int version ) const;
	// void Load( CUtlBuffer &fileBuffer, unsigned int version );

	unsigned int GetID( void ) const	{ return m_id; }		///< return this ladder's unique ID
	//static void CompressIDs( void );							///<re-orders ladder ID's so they are continuous


	Vector m_top;									///< world coords of the top of the ladder
	Vector m_bottom;								///< world coords of the top of the ladder
	float m_length;									///< the length of the ladder
	float m_width;

	Vector GetPosAtHeight( float height ) const;	///< Compute x,y coordinate of the ladder at a given height

	INavArea *m_topForwardArea;						///< the area at the top of the ladder
	INavArea *m_topLeftArea;
	INavArea *m_topRightArea;
	INavArea *m_topBehindArea;						///< area at top of ladder "behind" it - only useful for descending
	INavArea *m_bottomArea;							///< the area at the bottom of the ladder

	bool IsConnected( const INavArea *area, Ladder_DirectionType dir ) const;	///< returns true if given area is connected in given direction

	// void ConnectGeneratedLadder( float maxHeightAboveTopArea );		///< Connect a generated ladder to nav areas at the end of nav generation

	void ConnectTo( INavArea *area );				///< connect this ladder to given area
	void Disconnect( INavArea *area );				///< disconnect this ladder from given area

	void OnSplit( INavArea *original, INavArea *alpha, INavArea *beta );	///< when original is split into alpha and beta, update our connections
	void OnDestroyNotify( INavArea *dead );			///< invoked when given area is going away

	// void DrawLadder( void ) const;					///< Draws ladder and connections
	// void DrawConnectedAreas( void );				///< Draws connected areas

	// void UpdateDangling( void );					///< Checks if the ladder is dangling (bots cannot go up)

	bool IsInUse( const IBasePlayer *ignore = NULL ) const;	///< return true if someone is on this ladder (other than 'ignore')

	// void SetDir( Nav_DirType dir );
	Nav_DirType GetDir( void ) const;
	const Vector &GetNormal( void ) const;

	void Shift( const Vector &shift );							///< shift the nav ladder

	bool IsUsableByTeam( int teamNumber ) const;
	IBaseEntity *GetLadderEntity( void ) const;

private:
	void FindLadderEntity( void );

	CHandle<IBaseEntity> m_ladderEntity;

	Nav_DirType m_dir;								///< which way the ladder faces (ie: surface normal of climbable side)
	Vector m_normal;								///< surface normal of the ladder surface (or Vector-ized m_dir, if the traceline fails)

	enum LadderConnectionType						///< Ladder connection directions, to facilitate iterating over connections
	{
		LADDER_TOP_FORWARD = 0,
		LADDER_TOP_LEFT,
		LADDER_TOP_RIGHT,
		LADDER_TOP_BEHIND,
		LADDER_BOTTOM,

		NUM_LADDER_CONNECTIONS
	};

	INavArea ** GetConnection( LadderConnectionType dir );

	//static unsigned int m_nextID;					///< used to allocate unique IDs
	unsigned int m_id;								///< unique area ID
};
typedef CUtlVector< INavLadder * > NavLadderVector;

inline Nav_DirType INavLadder::GetDir(void) const
{
	return m_dir;
}

inline const Vector &INavLadder::GetNormal( void ) const
{
	return m_normal;
}

#endif