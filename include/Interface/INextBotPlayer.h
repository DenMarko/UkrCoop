#ifndef _HEADER_NEXT_BOT_PLAYER_INCLUDE_
#define _HEADER_NEXT_BOT_PLAYER_INCLUDE_
#include "ITerrorPlayer.h"
#include "INextBot.h"

class INextBotPlayerInput
{
public:
	virtual void PressFireButton( float duration = -1.0f ) = 0;
	virtual void ReleaseFireButton( void ) = 0;

	virtual void PressMeleeButton( float duration = -1.0f ) = 0;
	virtual void ReleaseMeleeButton( void ) = 0;

	virtual void PressUseButton( float duration = -1.0f ) = 0;
	virtual void ReleaseUseButton( void ) = 0;

	virtual void PressReloadButton( float duration = -1.0f ) = 0;
	virtual void ReleaseReloadButton( void ) = 0;
	
	virtual void PressForwardButton( float duration = -1.0f ) = 0;
	virtual void ReleaseForwardButton( void ) = 0;

	virtual void PressBackwardButton( float duration = -1.0f ) = 0;
	virtual void ReleaseBackwardButton( void ) = 0;

	virtual void PressLeftButton( float duration = -1.0f ) = 0;
	virtual void ReleaseLeftButton( void ) = 0;

	virtual void PressRightButton( float duration = -1.0f ) = 0;
	virtual void ReleaseRightButton( void ) = 0;

	virtual void PressJumpButton( float duration = -1.0f ) = 0;
	virtual void ReleaseJumpButton( void ) = 0;

	virtual void PressCrouchButton( float duration = -1.0f ) = 0;
	virtual void ReleaseCrouchButton( void ) = 0;

	virtual void PressWalkButton( float duration = -1.0f ) = 0;
	virtual void ReleaseWalkButton( void ) = 0;

	virtual void SetButtonScale( float forward, float right ) = 0;
};


template<typename PlayerType>
class INextBotPlayer : public PlayerType, public INextBot, public INextBotPlayerInput
{
public:
	virtual ~INextBotPlayer() {};

	virtual void 				Spawn( void ) = 0;
	virtual void 				Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual INextBot*			MyNextBotPointer( void ) = 0;
	virtual bool 				IsNetClient( void ) const = 0;                 					// Bots should return FALSE for this, they can't receive NET messages
	virtual void 				Touch( CBaseEntity *other ) = 0;
	virtual void 				PhysicsSimulate( void ) = 0;
	virtual void 				HandleAnimEvent( animevent_t *event ) = 0;
	virtual void 				Weapon_Equip( CBaseCombatWeapon *weapon ) = 0;						// for OnPickUp
	virtual	void 				Weapon_Drop( CBaseCombatWeapon *weapon, const Vector *target, const Vector *velocity ) = 0;	// for OnDrop
	virtual int 				OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual int 				OnTakeDamage_Dying( const CTakeDamageInfo &info ) = 0;
	virtual void 				OnNavAreaChanged( CNavArea *enteredArea, CNavArea *leftArea ) = 0;	// invoked (by UpdateLastKnownArea) when we enter a new nav area (or it is reset to NULL)
	virtual bool 				IsFakeClient( void ) const = 0;
	virtual bool 				IsAbleToAutoCenterOnLadders(void) const = 0;
	virtual void 				OnMainActivityComplete( Activity newActivity, Activity oldActivity ) = 0;
	virtual void 				OnMainActivityInterrupted( Activity newActivity, Activity oldActivity ) = 0;
	virtual bool 				IsBot( void ) const = 0;
	virtual bool 				IsRemovedOnReset( void ) const = 0;				                // remove this bot when the NextBot manager calls Reset
	virtual void 				PressFireButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseFireButton( void ) = 0;
	virtual void 				PressMeleeButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseMeleeButton( void ) = 0;
	virtual void 				PressUseButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseUseButton( void ) = 0;
	virtual void 				PressReloadButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseReloadButton( void ) = 0;
	virtual void 				PressForwardButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseForwardButton( void ) = 0;
	virtual void 				PressBackwardButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseBackwardButton( void ) = 0;
	virtual void 				PressLeftButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseLeftButton( void ) = 0;
	virtual void 				PressRightButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseRightButton( void ) = 0;
	virtual void 				PressJumpButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseJumpButton( void ) = 0;
	virtual void 				PressCrouchButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseCrouchButton( void ) = 0;
	virtual void 				PressWalkButton( float duration = -1.0f ) = 0;
	virtual void 				ReleaseWalkButton( void ) = 0;
	virtual void 				SetButtonScale( float forward, float right ) = 0;

public:
	// begin INextBot ------------------------------------------------------------------------------------------------------------------
	virtual void 				Update( void ) = 0;												// (EXTEND) update internal state

};

#endif