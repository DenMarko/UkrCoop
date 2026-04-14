#ifndef _INCLUDE_BASE_MULTIPLAYER_PLAYER_H_
#define _INCLUDE_BASE_MULTIPLAYER_PLAYER_H_
#include "IBasePlayer.h"
#include "../sdk/ai_speechconcept.h"

enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY=0,
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY,
	PLAYERANIMEVENT_THROW_GRENADE,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_RELOAD,
	PLAYERANIMEVENT_RELOAD_START,
	PLAYERANIMEVENT_RELOAD_LOOP,
	PLAYERANIMEVENT_RELOAD_END,
	
	PLAYERANIMEVENT_COUNT
};

class CWeaponCSBase;
class CCSPlayer;
class CMultiplayer_Expresser;

enum SpeechPriorityType
{
	SPEECH_PRIORITY_LOW,
	SPEECH_PRIORITY_NORMAL,
	SPEECH_PRIORITY_MANUAL,
	SPEECH_PRIORITY_UNINTERRUPTABLE,
};

class IFlexCycler : public IBaseFlex
{
public:
	~IFlexCycler() {
	}

	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Spawn( void ) = 0;
	virtual int				ObjectCaps( void ) = 0;
	virtual void			Think( void ) = 0;
	virtual int				OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual void			ProcessSceneEvents( void ) = 0;
	virtual bool			IsAlive( void ) = 0;

	float m_flextime;					// 1668
	LocalFlexController_t m_flexnum;	// 1672
	float m_flextarget[64];				// 1676
	float m_blinktime;					// 1932
	float m_looktime;					// 1936
	Vector m_lookTarget;				// 1940
	float m_speaktime;					// 1952
	int	m_istalking;					// 1956
	int	m_phoneme;						// 1960

	string_t m_iszSentence;				// 1964
	int m_sentence;						// 1968
};

class IFlexExpresserShim : public IFlexCycler
{
public:
	~IFlexExpresserShim() { }
	inline CAI_Expresser *GetExpresser( void ) { return m_pExpresser; }
	inline const CAI_Expresser *GetMultiplayerExpresser( void ) const { return m_pExpresser; }

protected:
	CAI_Expresser *m_pExpresser;
};

class CAI_ExpresserSink
{
public:
	virtual void			OnSpokeConcept( CAI_Concept concepts, AI_Response *response ) = 0;
	virtual void 			OnStartSpeaking() = 0;
	virtual bool 			UseSemaphore() = 0;
};

template <class BASE_CLASS>
class IExpresserHost : public BASE_CLASS, protected CAI_ExpresserSink
{
public:
	virtual ~IExpresserHost() {}
protected:
	virtual IResponseSystem 	*GetResponseSystem() = 0;
	virtual void				DispatchResponse( const char *conceptName ) = 0;
	virtual void				ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
public:
	virtual bool				CanSpeak() = 0;
	virtual void				NoteSpeaking( float duration, float delay ) = 0;
	virtual bool 				Speak( CAI_Concept *concepts, const char *modifiers = nullptr, char *pszOutResponseChosen = nullptr, size_t bufsize = 0, IRecipientFilter *filter = nullptr ) = 0;
	virtual bool				Speak( CAI_Concept *concepts, AI_CriteriaSet *, char *pszOutResponseChosen = nullptr, unsigned int bufsize = 0, IRecipientFilter *filter = nullptr) = 0;
	virtual void				PostSpeakDispatchResponse( CAI_Concept concepts, AI_Response *response ) = 0;
};

class IBaseMultiplayerPlayer : public IExpresserHost<IBasePlayer>
{
public:
	virtual ~IBaseMultiplayerPlayer() { }

	virtual void				Spawn( void ) = 0;
	virtual void				Precache( void ) = 0;
	virtual void				PostConstructor( const char *szClassname ) = 0;
	virtual IResponseSystem 	*GetResponseSystem() = 0;
	virtual void				ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
	virtual bool				ClientCommand( const CCommand &args ) = 0;
	virtual bool				CanHearAndReadChatFrom( CBasePlayer *pPlayer ) = 0;
	virtual bool				CanSpeak( void ) = 0;
	virtual CAI_Expresser 		*GetExpresser() = 0;
	virtual bool				SpeakIfAllowed( CAI_Concept, SpeechPriorityType, char const*, char *, unsigned int, IRecipientFilter *) = 0;
	virtual bool				SpeakConceptIfAllowed( int iConcept, const char *modifiers = nullptr, char *pszOutResponseChosen = nullptr, size_t bufsize = 0, IRecipientFilter *filter = nullptr ) = 0;
	virtual bool				CanSpeakVoiceCommand( void ) = 0;
	virtual bool				ShouldShowVoiceSubtitleToEnemy( void ) = 0;
	virtual void				NoteSpokeVoiceCommand( const char *pszScenePlayed ) = 0;
	virtual CMultiplayer_Expresser *GetMultiplayerExpresser() = 0;
	virtual int					CalculateTeamBalanceScore( void ) = 0;
protected:
	virtual CAI_Expresser 		*CreateExpresser( void ) = 0;
private:
	int 						m_iIgnoreGlobalChat;							// 4548
	int							m_iCurrentConcept;								// 4552
	CMultiplayer_Expresser		*m_pExpresser;									// 4556
	float 						m_flConnectionTime;								// 4560
	float 						m_flLastForcedChangeTeamTime;					// 4564
	int 						m_iBalanceScore;								// 4568
	KeyValues					*m_pAchievementKV;								// 4572
};

class IFlexExpresser : public IExpresserHost<IFlexExpresserShim>
{
public:
	~IFlexExpresser() {}

	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Think( void ) = 0;
	virtual IResponseSystem *GetResponseSystem() = 0;
	virtual int				OnTakeDamage( const CTakeDamageInfo &info ) = 0;
};

#endif