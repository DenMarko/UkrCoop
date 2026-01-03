#include "CAmmoPack.h"
#include "CAmmoDef.h"
#include "Interface/IProps.h"
#include "Interface/ITerrorPlayer.h"
#include "Interface/IInfected.h"
#include "detours.h"

extern int PrecacheModels(const char *szName, bool isPreload = true);
extern bool Disolved(CBaseEntity* pThisPtr, const char* pMaterialName, bool bNPCOnly, int nDissolveType, Vector vDissolveOrigin, int nMagnitude);

ConVar      ukr_ammobox_use_distance("   ukr_ammobox_use_distance",  "700",  FCVAR_CHEAT, "");
ConVar      ukr_add_ammo_ammobox("       ukr_add_ammo_ammobox",      "80",   FCVAR_CHEAT, "");

#define MODEL_SHOTGUN_BOX "models/items/boxbuckshot.mdl"
#define MODEL_PISTOL_BOX "models/items/357ammobox.mdl"

class CAmmoBox
{
public:
    CAmmoBox();
    virtual ~CAmmoBox();

    virtual bool Spawn(const Vector &vecOrigion, bool IsShotgun = false);
    virtual void OnUse(IPhysicsProp* pThis, IBaseEntity* pActivator, IBaseEntity* pCaller, USE_TYPE nType, float value);

    inline IBaseEntity *GetEntity() { return m_pEntity; }
    inline bool operator == (CBaseEntity *other)
    {
        return (this->GetEntity() == (IBaseEntity*)other); 
    }
    inline bool operator != (CBaseEntity *other)
    {
        return (this->GetEntity() != (IBaseEntity*)other); 
    }
    inline bool operator == (IBaseEntity *other)
    {
        return (this->GetEntity() == other); 
    }
    inline bool operator != (IBaseEntity *other)
    {
        return (this->GetEntity() != other); 
    }
    inline bool operator == (void *other)
    {
        return (this->GetEntity() == other); 
    }
    inline bool operator != (void *other)
    {
        return (this->GetEntity() != other); 
    }
    
private:
    bool GiveAmmo(ITerrorPlayer*, IBaseCombatWeapon*);

    IPhysicsProp *m_pEntity;
};

class CCPhysicsPropOnUse
{
public:
    void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
    static MemberClassFunctionWrapper<void, CCPhysicsPropOnUse, CBaseEntity*, CBaseEntity*, USE_TYPE, float> OnUse_Actual; 
};

MemberClassFunctionWrapper<void, CCPhysicsPropOnUse, CBaseEntity*, CBaseEntity*, USE_TYPE, float> CCPhysicsPropOnUse::OnUse_Actual;
CUtlVector<CAmmoBox*> m_hookList;

class CAmmoPack : public CAppSystem
{
public:
    CAmmoPack() : CAppSystem() { }
    virtual ~CAmmoPack() { }

	virtual void OnLoad()
    {
        PrecacheModels(MODEL_SHOTGUN_BOX);
        PrecacheModels(MODEL_PISTOL_BOX);

        m_pCPhysicsPropOnUse = CDetourManager::CreateDetour(
            (void *)GetCodeAddr(reinterpret_cast<VoidFunc>(&CCPhysicsPropOnUse::Use)), 
            CCPhysicsPropOnUse::OnUse_Actual, "CPhysicsPropOnUse");
            
        if(m_pCPhysicsPropOnUse)
        {
            m_pCPhysicsPropOnUse->EnableDetour();
        }
    }

	virtual void OnUnload()
    {
        if(m_pCPhysicsPropOnUse)
        {
            m_pCPhysicsPropOnUse->Destroy();
            m_pCPhysicsPropOnUse = nullptr;
        }
    }

    virtual ResultType OnClientCommand(int client, const CCommand &args)
    {
        IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
        if(!pPlayer ||
           !pPlayer->IsConnected() || 
           !pPlayer->IsInGame())
        {
            return Pl_Continue;
        }

		if(g_Sample.my_bStrcmp(args[0], "ukr_test_box"))
		{
            IBasePlayer *me = (IBasePlayer*)gamehelpers->ReferenceToEntity(client);
            if(me != nullptr)
            {
                Vector forvard;
                me->EyeVectors(&forvard);

                trace_t res;
                util_TraceLine(
                    me->EyePosition(), 
                    me->EyePosition() + 999999.9f * forvard, 
                    (MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE), 
                    me->GetNetworkable()->GetEntityHandle(), 
                    COLLISION_GROUP_NONE, 
                    &res);
            
                if(res.DidHit())
                {
                    OnCreate(res.endpos + Vector(0.f, 0.f, 10.f), RandomInt(1, 2) == 1 ? true : false);
                }
            }

			return Pl_Handled;
        }
        else if(g_Sample.my_bStrcmp(args[0], "ukr_stagger"))
        {
            CBaseEntity *pEntity = nullptr;
            ITerrorPlayer *pAttacer = GetVirtualClass<ITerrorPlayer>(client);
            Vector vecPos = pAttacer->GetAbsOrigin();
            float flRadius = 10000.f;

            for(CEntitySphereQuery_ sphere(vecPos, flRadius); (pEntity = sphere.GetCurrentEntity()) != nullptr; sphere.NextEntity())
            {
                ITerrorPlayer *pPlayer = access_dynamic_cast<ITerrorPlayer>((IBaseEntity*)pEntity, "CTerrorPlayer");
                if(pPlayer && pPlayer != pAttacer)
                {
                    if(pPlayer->GetTeamNumber() == 2)
                    {
                        if(!pPlayer->IsStaggering())
                            pPlayer->OnStaggered(pAttacer);
                    }
                    else if(pPlayer->GetTeamNumber() == 3)
                    {
                        if(pPlayer->GetClass() == ZombieClassTank)
                            pPlayer->OnStaggered(pAttacer);
                        else
                            pPlayer->OnShovedBySurvivor(pAttacer, vecPos);
                    }
                }
            }
            return Pl_Handled;
        }
        return Pl_Continue;
    }

	virtual void OnEntityDestroyed(IBaseEntity *pEntity)
    {
        if( !m_hookList.Size() ) return;

        FOR_EACH_VEC(m_hookList, i)
        {
            CAmmoBox *pBox = m_hookList[i];
            if( *pBox == pEntity)
            {
                delete pBox;
                m_hookList.Remove(i);
                break;
            }
        }
    }


private:
    CDetour *m_pCPhysicsPropOnUse;
};

CAmmoPack *g_pAmmoPakInit = new CAmmoPack();

void CCPhysicsPropOnUse::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
    OnUse_Actual(this, pActivator, pCaller, useType, value);

    if(m_hookList.Count() <= 0) return;

    FOR_EACH_VEC(m_hookList, i)
    {
        CAmmoBox *pBox = m_hookList[i];
        if(*pBox == this)
        {
            pBox->OnUse((IPhysicsProp*)this, (IBaseEntity*)pActivator, (IBaseEntity*)pCaller, useType, value);
        }
    }
}


CAmmoBox::CAmmoBox() : m_pEntity(nullptr)
{
}

CAmmoBox::~CAmmoBox()
{
}

bool CAmmoBox::Spawn(const Vector &vecOrigion, bool IsShotgun)
{
    QAngle vecAngle = QAngle(0.f, RandomFloat(0.f, 360.f), 0.f);
    m_pEntity = (IPhysicsProp*)CreateNoSpawn("prop_physics", vecOrigion, vecAngle, NULL);

    if(!m_pEntity)
    {
        return false;
    }

    if(IsShotgun) {
        m_pEntity->SetModel(MODEL_SHOTGUN_BOX);
    } else {
        m_pEntity->SetModel(MODEL_PISTOL_BOX);
    }

    m_pEntity->DispatchUpdateTransmitState();
    servertools->DispatchSpawn(m_pEntity);
    m_pEntity->SetFadeDistance(-1, 0);
    m_pEntity->DisableAutoFade();

    return true;
}

bool CAmmoBox::GiveAmmo(ITerrorPlayer *pPlayer, IBaseCombatWeapon *pWeapon)
{
    if(pWeapon != nullptr)
    {
        _CAmmoDef *ptr = nullptr;
        if((ptr = (_CAmmoDef *)g_CallHelper->GetAmmoDef()) == nullptr)
        {
            return false;
        }

        if(pWeapon->UsesPrimaryAmmo())
        {
            int ammoIndex = pWeapon->GetPrimaryAmmoType();
            if(ammoIndex != -1)
            {
                Ammo_t *szName = GetAmmoOfIndex(ptr, ammoIndex);
                int iAmmoType = Index((_CAmmoDef *)g_CallHelper->GetAmmoDef(), szName->pName);
                if(iAmmoType == -1)
                {
                    return false;
                }

                pPlayer->GiveAmmoBox(ukr_add_ammo_ammobox.GetInt(), iAmmoType);
                return true;
            }
        }

        if(pWeapon->UsesSecondaryAmmo() && pWeapon->HasSecondaryAmmo())
        {
            int ammoIndex = pWeapon->GetSecondaryAmmoType();
            if(ammoIndex != -1)
            {
                Ammo_t *szName = GetAmmoOfIndex(ptr, ammoIndex);
                int iAmmoType = Index((_CAmmoDef *)g_CallHelper->GetAmmoDef(), szName->pName);
                if(iAmmoType == -1)
                {
                    return false;
                }

                pPlayer->GiveAmmoBox(ukr_add_ammo_ammobox.GetInt(), iAmmoType);
                return true;
            }
        }
    }
    return false;
}

void CAmmoBox::OnUse(IPhysicsProp *pThis, IBaseEntity *pActivator, IBaseEntity *pCaller, USE_TYPE nType, float value)
{    
    ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(pActivator);
    Vector to = (pPlayer->GetAbsOrigin() - pThis->GetAbsOrigin());
    if(to.IsLengthGreaterThan(ukr_ammobox_use_distance.GetFloat()))
    {
        return;
    }

    IBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
    if(g_Sample.my_bStrcmp(STRING(pThis->GetModelName()), MODEL_SHOTGUN_BOX))
    {
        if(g_Sample.my_bStrcmp(pWeapon->GetName(), "weapon_autoshotgun") 
        || g_Sample.my_bStrcmp(pWeapon->GetName(), "weapon_pumpshotgun"))
        {
            GiveAmmo(pPlayer, pWeapon);
            g_CallHelper->UTIL_Remove(pThis);
            return;
        }
    }
    else
    {
        if(g_Sample.my_bStrcmp(pWeapon->GetName(), "weapon_hunting_rifle") 
        || g_Sample.my_bStrcmp(pWeapon->GetName(), "weapon_rifle") 
        || g_Sample.my_bStrcmp(pWeapon->GetName(), "weapon_smg"))
        {
            GiveAmmo(pPlayer, pWeapon);
            g_CallHelper->UTIL_Remove(pThis);
            return;
        }
    }

    int client = pPlayer->entindex();
    char msg[255];
    g_HL2->Translate(msg, sizeof(msg), "[I] %T", 2, NULL, "these_cartridges_weapon", &client);
    g_HL2->TextMsg(client, CHAT, msg);
}

int OnCreate(const Vector &vecOrigion, bool IsShotgun)
{
    int i = m_hookList.AddToTail(new CAmmoBox());
    CAmmoBox *pBox = m_hookList[i];

    if(pBox->Spawn(vecOrigion, IsShotgun))
    {
        return pBox->GetEntity()->entindex();
    }
    return -1;
}

