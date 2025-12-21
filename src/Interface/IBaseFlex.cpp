#include "IBaseFlex.h"

void IBaseFlex::SetFlexWeight(LocalFlexController_t index, float value)
{
    if(index >= 0 && index < GetNumFlexControllers())
    {
        CStudioHdr *pStudioHdr = GetModelPtr();
        if(!pStudioHdr)
            return;

        mstudioflexcontroller_t* pFlexcontroller = pStudioHdr->pFlexcontroller(index);
        if(pFlexcontroller->max != pFlexcontroller->min)
        {
			value = (value - pFlexcontroller->min) / (pFlexcontroller->max - pFlexcontroller->min);
			value = clamp( value, 0.0f, 1.0f );
        }

        m_flexWeight.Set( index, value);
    }
}

float IBaseFlex::GetFlexWeight(LocalFlexController_t index)
{
    if(index >= 0 && index < GetNumFlexControllers())
    {
        CStudioHdr *pStudioHdr = GetModelPtr();
        if(!pStudioHdr)
            return 0.0f;

        mstudioflexcontroller_t *pflexcontroller = pStudioHdr->pFlexcontroller(index);

        if(pflexcontroller->max != pflexcontroller->min)
        {
            return m_flexWeight[index] * (pflexcontroller->max - pflexcontroller->min) + pflexcontroller->min;
        }
        return m_flexWeight[index];
    }
    return 0.0f;
}

LocalFlexController_t IBaseFlex::FindFlexController(const char *szName)
{
    for(LocalFlexController_t i = LocalFlexController_t(0); i < GetNumFlexControllers(); i++)
    {
        if(V_stricmp(GetFlexControllerName(i), szName) == 0)
        {
            return i;
        }
    }

    return LocalFlexController_t(0);
}

bool IBaseFlex::IsSuppressedFlexAnimation( CSceneEventInfo *info )
{
	if (info->m_pScene && (*((unsigned char *)info->m_pScene + 524) & 1))
	{
		return m_flLastFlexAnimationTime > g_pGlobals->curtime - GetAnimTimeInterval() * 1.5;
	}

    m_flLastFlexAnimationTime = g_pGlobals->curtime;
	return false;
}


void CSceneEventInfo::InitWeight( IBaseFlex *pActor )
{
	if (pActor->IsSuppressedFlexAnimation( this ))
	{
		m_flWeight = 0.0;
	}
	else
	{
		m_flWeight = 1.0;
	}
}

float CSceneEventInfo::UpdateWeight( IBaseFlex *pActor )
{
	// decay if this is a background scene and there's other flex animations playing
	if (pActor->IsSuppressedFlexAnimation( this ))
	{
		m_flWeight = MAX( m_flWeight - 0.2, 0.0 );
	}
	else
	{
		m_flWeight = MIN( m_flWeight + 0.1, 1.0 );
	}
	return m_flWeight;
}

