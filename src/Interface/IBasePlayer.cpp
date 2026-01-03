#include "IBasePlayer.h"
#include "IBaseCombatWeapon.h"

void IBasePlayer::EyeVectors(Vector *pForward, Vector *pRight, Vector *pUp)
{
    if(GetVehicle() != nullptr)
    {
        CacheVehicleView();
        AngleVectors(m_vecVehicleViewAngles, pForward, pRight, pUp);
    }
    else
    {
        AngleVectors(EyeAngles(), pForward, pRight, pUp);
    }
}

void IBasePlayer::CacheVehicleView(void)
{
    if(m_nVehicleViewSavedFrame == g_pGlobals->framecount)
        return;

    IServerVehicle *pVehicle = GetVehicle();
    if(pVehicle)
    {
        int nRole = pVehicle->GetPassengerRole((CBaseCombatCharacter*)this);

        pVehicle->GetVehicleViewPosition(nRole, &m_vecVehicleViewOrigin, &m_vecVehicleViewAngles, &m_flVehicleViewFOV);
        m_nVehicleViewSavedFrame = g_pGlobals->framecount;
    }
}

bool fogparams_t::operator !=( const fogparams_t& other ) const
{
	if ( this->enable != other.enable ||
		this->blend != other.blend ||
		!VectorsAreEqual(this->dirPrimary, other.dirPrimary, 0.01f ) || 
		this->colorPrimary != other.colorPrimary ||
		this->colorSecondary != other.colorSecondary ||
		this->start != other.start ||
		this->end != other.end ||
		this->farz != other.farz ||
		this->maxdensity != other.maxdensity ||
		this->colorPrimaryLerpTo != other.colorPrimaryLerpTo ||
		this->colorSecondaryLerpTo != other.colorSecondaryLerpTo ||
		this->startLerpTo != other.startLerpTo ||
		this->endLerpTo != other.endLerpTo ||
		this->maxdensityLerpTo != other.maxdensityLerpTo ||
		this->lerptime != other.lerptime ||
		this->duration != other.duration ||
		this->HDRColorScale != other.HDRColorScale)
		return true;

	return false;
}

CPlayerLocalData::CPlayerLocalData()
{
	m_audio.soundscapeIndex = 0;
	m_audio.localBits = 0;
	m_audio.entIndex = 0;
	m_pOldSkyCamera = NULL;
	m_bDrawViewmodel = true;
}

void CPlayerLocalData::UpdateAreaBits( IBasePlayer *pl, unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES] )
{
	Vector origin = pl->EyePosition();

	unsigned char tempBits[32] = { 0 };
	COMPILE_TIME_ASSERT( sizeof( tempBits ) >= sizeof( ((CPlayerLocalData*)0)->m_chAreaBits ) );

	int i;
	int area = engine->GetArea( origin );
	engine->GetAreaBits( area, tempBits, sizeof( tempBits ) );
	for ( i=0; i < MAX_AREA_STATE_BYTES; i++ )
	{
		if ( tempBits[i] != m_chAreaBits[ i ] )
		{
			m_chAreaBits.Set(i, tempBits[i]);
		}
	}

	for ( i=0; i < MAX_AREA_PORTAL_STATE_BYTES; i++ )
	{
		if ( chAreaPortalBits[i] != m_chAreaPortalBits[i] )
			m_chAreaPortalBits.Set(i, chAreaPortalBits[i]);
	}
}

const char *IBasePlayer::GetNetworkIDString()
{
	Q_strncpy( m_szNetworkIDString, engine->GetPlayerNetworkIDString( edict() ), sizeof(m_szNetworkIDString) );
	return m_szNetworkIDString; 
}

bool IBasePlayer::IsDead() const
{
	return m_lifeState == LIFE_DEAD;
}

const char *CPlayerInfo::GetName()
{ 
	return m_pParent->GetPlayerName(); 
}

int	CPlayerInfo::GetUserID() 
{ 
	return engine->GetPlayerUserId( m_pParent->edict() ); 
}

const char *CPlayerInfo::GetNetworkIDString() 
{ 
	return m_pParent->GetNetworkIDString(); 
}

int	CPlayerInfo::GetTeamIndex() 
{ 
	return m_pParent->GetTeamNumber(); 
}  

void CPlayerInfo::ChangeTeam( int iTeamNum ) 
{ 
	m_pParent->ChangeTeam(iTeamNum); 
}

int	CPlayerInfo::GetFragCount() 
{ 
	return m_pParent->FragCount(); 
}

int	CPlayerInfo::GetDeathCount() 
{ 
	return m_pParent->DeathCount(); 
}

bool CPlayerInfo::IsConnected() 
{ 
	return m_pParent->IsConnected(); 
}

int	CPlayerInfo::GetArmorValue() 
{ 
	return m_pParent->ArmorValue(); 
}

bool CPlayerInfo::IsHLTV() 
{ 
	return m_pParent->IsHLTV(); 
}

bool CPlayerInfo::IsPlayer() 
{ 
	return m_pParent->IsPlayer(); 
}

bool CPlayerInfo::IsFakeClient() 
{ 
	return m_pParent->IsFakeClient(); 
}

bool CPlayerInfo::IsDead() 
{ 
	return m_pParent->IsDead(); 
}

bool CPlayerInfo::IsInAVehicle() 
{ 
	return m_pParent->IsInAVehicle(); 
}

bool CPlayerInfo::IsObserver() 
{ 
	return m_pParent->IsObserver(); 
}

const Vector CPlayerInfo::GetAbsOrigin() 
{ 
	return m_pParent->GetAbsOrigin(); 
}

const QAngle CPlayerInfo::GetAbsAngles() 
{ 
	return m_pParent->GetAbsAngles(); 
}

const Vector CPlayerInfo::GetPlayerMins() 
{ 
	return m_pParent->GetPlayerMins(); 
}

const Vector CPlayerInfo::GetPlayerMaxs() 
{ 
	return m_pParent->GetPlayerMaxs(); 
}

const char *CPlayerInfo::GetWeaponName() 
{ 
	IBaseCombatWeapon *weap = m_pParent->GetActiveWeapon();
	if ( !weap )
	{
		return NULL;
	}
	return weap->GetName();
}

const char *CPlayerInfo::GetModelName() 
{ 
	return m_pParent->GetModelName().ToCStr(); 
}

const int CPlayerInfo::GetHealth() 
{ 
	return m_pParent->GetHealth(); 
}

const int CPlayerInfo::GetMaxHealth() 
{ 
	return m_pParent->GetMaxHealth(); 
}

void CPlayerInfo::SetAbsOrigin( Vector & vec ) 
{ 
	if ( m_pParent->IsBot() )
	{
		m_pParent->SetAbsOrigin(vec); 
	}
}

void CPlayerInfo::SetAbsAngles( QAngle & ang ) 
{ 
	if ( m_pParent->IsBot() )
	{
		m_pParent->SetAbsAngles(ang); 
	}
}

void CPlayerInfo::RemoveAllItems( bool removeSuit ) 
{ 
	if ( m_pParent->IsBot() )
	{
		m_pParent->RemoveAllItems(removeSuit); 
	}
}

void CPlayerInfo::SetActiveWeapon( const char *WeaponName ) 
{ 
	if ( m_pParent->IsBot() )
	{
		CBaseCombatWeapon *weap = m_pParent->Weapon_Create( WeaponName );
		if ( weap )
		{
			m_pParent->Weapon_Equip(weap); 
			m_pParent->Weapon_Switch(weap); 
		}
	}
}

void CPlayerInfo::SetLocalOrigin( const Vector& origin ) 
{ 
	if ( m_pParent->IsBot() )
	{
		m_pParent->SetLocalOrigin(origin); 
	}
}

const Vector CPlayerInfo::GetLocalOrigin( void ) 
{ 
	if ( m_pParent->IsBot() )
	{
		Vector origin = m_pParent->GetLocalOrigin();
		return origin; 
	}
	else
	{
		return Vector( 0, 0, 0 );
	}
}

void CPlayerInfo::SetLocalAngles( const QAngle& angles ) 
{ 
	if ( m_pParent->IsBot() )
	{
		m_pParent->SetLocalAngles( angles ); 
	}
}

const QAngle CPlayerInfo::GetLocalAngles( void ) 
{ 
	if ( m_pParent->IsBot() )
	{
		return m_pParent->GetLocalAngles(); 
	}
	else
	{
		return QAngle();
	}
}

void CPlayerInfo::PostClientMessagesSent( void ) 
{ 
	if ( m_pParent->IsBot() )
	{
		m_pParent->PostClientMessagesSent(); 
	}
}

bool CPlayerInfo::IsEFlagSet( int nEFlagMask ) 
{ 
	if ( m_pParent->IsBot() )
	{
		return m_pParent->IsEFlagSet(nEFlagMask); 
	}
	return false;
}

void CPlayerInfo::RunPlayerMove( CBotCmd *ucmd ) 
{ 
	if ( m_pParent->IsBot() )
	{
		CUserCmd cmd;
		cmd.buttons = ucmd->buttons;
		cmd.command_number = ucmd->command_number;
		cmd.forwardmove = ucmd->forwardmove;
		cmd.hasbeenpredicted = ucmd->hasbeenpredicted;
		cmd.impulse = ucmd->impulse;
		cmd.mousedx = ucmd->mousedx;
		cmd.mousedy = ucmd->mousedy;
		cmd.random_seed = ucmd->random_seed;
		cmd.sidemove = ucmd->sidemove;
		cmd.tick_count = ucmd->tick_count;
		cmd.upmove = ucmd->upmove;
		cmd.viewangles = ucmd->viewangles;
		cmd.weaponselect = ucmd->weaponselect;
		cmd.weaponsubtype = ucmd->weaponsubtype;

		// Store off the globals.. they're gonna get whacked
		float flOldFrametime = g_pGlobals->frametime;
		float flOldCurtime = g_pGlobals->curtime;

		m_pParent->SetTimeBase( g_pGlobals->curtime );

		g_CallHelper->MoveHelperServerv()->SetHost( m_pParent );
		m_pParent->PlayerRunCommand( &cmd, g_CallHelper->MoveHelperServerv() );

		// save off the last good usercmd
		m_pParent->SetLastUserCommand( cmd );

		// Clear out any fixangle that has been set
		m_pParent->pl.fixangle = FIXANGLE_NONE;

		// Restore the globals..
		g_pGlobals->frametime = flOldFrametime;
		g_pGlobals->curtime = flOldCurtime;
		g_CallHelper->MoveHelperServerv()->SetHost( NULL );
	}
}

void CPlayerInfo::SetLastUserCommand( const CBotCmd &ucmd ) 
{ 
	if ( m_pParent->IsBot() )
	{
		CUserCmd cmd;
		cmd.buttons = ucmd.buttons;
		cmd.command_number = ucmd.command_number;
		cmd.forwardmove = ucmd.forwardmove;
		cmd.hasbeenpredicted = ucmd.hasbeenpredicted;
		cmd.impulse = ucmd.impulse;
		cmd.mousedx = ucmd.mousedx;
		cmd.mousedy = ucmd.mousedy;
		cmd.random_seed = ucmd.random_seed;
		cmd.sidemove = ucmd.sidemove;
		cmd.tick_count = ucmd.tick_count;
		cmd.upmove = ucmd.upmove;
		cmd.viewangles = ucmd.viewangles;
		cmd.weaponselect = ucmd.weaponselect;
		cmd.weaponsubtype = ucmd.weaponsubtype;

		m_pParent->SetLastUserCommand(cmd); 
	}
}


CBotCmd CPlayerInfo::GetLastUserCommand()
{
	CBotCmd cmd;
	const CUserCmd *ucmd = m_pParent->GetLastUserCommand();
	if ( ucmd )
	{
		cmd.buttons = ucmd->buttons;
		cmd.command_number = ucmd->command_number;
		cmd.forwardmove = ucmd->forwardmove;
		cmd.hasbeenpredicted = ucmd->hasbeenpredicted;
		cmd.impulse = ucmd->impulse;
		cmd.mousedx = ucmd->mousedx;
		cmd.mousedy = ucmd->mousedy;
		cmd.random_seed = ucmd->random_seed;
		cmd.sidemove = ucmd->sidemove;
		cmd.tick_count = ucmd->tick_count;
		cmd.upmove = ucmd->upmove;
		cmd.viewangles = ucmd->viewangles;
		cmd.weaponselect = ucmd->weaponselect;
		cmd.weaponsubtype = ucmd->weaponsubtype;
	}
	return cmd;
}

bool IBasePlayer::ClearUseEntity(void)
{
	if(m_hUseEntity != NULL)
	{
		m_hUseEntity->Use((CBaseEntity*)this, (CBaseEntity*)this, USE_OFF, 0);
		m_hUseEntity = NULL;
		return true;
	}

    return false;
}

void IBasePlayer::SnapEyeAngles(const QAngle &viewAngles)
{
	pl.v_angle = viewAngles;
	pl.fixangle = FIXANGLE_ABSOLUTE;
}

void IBasePlayer::Weapon_DropSlot(int weaponSlot)
{
	IBaseCombatWeapon *pWeapon;

	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		pWeapon = GetWeapon(i);
		if(pWeapon != nullptr)
		{
			if(pWeapon->GetSlot() == weaponSlot )
			{
				Weapon_Drop((CBaseCombatWeapon*)pWeapon);
			}
		}
	}
}
