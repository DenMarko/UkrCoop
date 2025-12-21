#include "extension.h"
#include "CStudio.h"
#include <HL2.h>

const virtualmodel_t *ResetVModel(const CStudioHdr *pStudio, const virtualmodel_t* pVModel)
{
    if(pVModel != nullptr)
    {
        pStudio->m_pVModel = (virtualmodel_t *)pVModel;

        int v1 = *(DWORD*)(*((DWORD*)pStudio + 1) + 100);
        pStudio->m_pStudioHdrCache.SetCount(v1);

        int i;
        for(i = 0; i < pStudio->m_pStudioHdrCache.Count(); i++)
        {
            pStudio->m_pStudioHdrCache[i] = nullptr;
        }
        return const_cast<virtualmodel_t *>(pVModel);
    }
    else
    {
        pStudio->m_pVModel = nullptr;
        return nullptr;
    }
}

bool SeqencesAvailable(const CStudioHdr *pStudio)
{
    if(pStudio->m_pStudioHdr->numincludemodels == 0)
    {
        return true;
    }

    if(pStudio->GetVirtualModel() == nullptr)
    {
        virtualmodel_t *pVModel = nullptr;
        if(pStudio->m_pStudioHdr->numincludemodels)
        {
            pVModel = g_pModelInfo->GetVirtualModel(pStudio->m_pStudioHdr);
        }

        return (ResetVModel(pStudio, pVModel) != nullptr);
    }
    else
    {
        return true;
    }
}

int GetNumAttachments(const CStudioHdr *pStudio)
{
    int v1 = *((DWORD *)pStudio + 1);
    if(v1)
        return *(DWORD *)(v1 + 60);
    else
        return *(DWORD *)(*(DWORD *)pStudio + 240);
}

int GetNumPoseParameters(const CStudioHdr* pStudio)
{
    int v1 = *((DWORD *)pStudio + 1);
    if (!v1)
    {
        if(pStudio->m_pStudioHdr)
            return pStudio->GetRenderHdr()->numlocalposeparameters;
        else
            return 0;
    }
    return *(DWORD*)(v1 + 80);
}

NOINLINE const studiohdr_t *virtualgroup_t::GetStudioHdr( void ) const
{
    return g_pModelInfo->FindModel(this->cache);
}

NOINLINE const studiohdr_t* GroupStudioHdr(const CStudioHdr *pSudio, int i)
{
    if(!pSudio)
    {
        Msg( "Call to NULL GroupStudioHdr()\n" );
    }

    if(pSudio->m_nFrameUnlockCounter != *pSudio->m_pFrameUnlockCounter)
    {
        pSudio->m_FrameUnlockCounterMutex.Lock();
        if(*pSudio->m_pFrameUnlockCounter != pSudio->m_nFrameUnlockCounter)
        {
            memset(pSudio->m_pStudioHdrCache.Base(), 0, pSudio->m_pStudioHdrCache.Count() * sizeof(studiohdr_t *));
            pSudio->m_nFrameUnlockCounter = *pSudio->m_pFrameUnlockCounter;
        }
        pSudio->m_FrameUnlockCounterMutex.Unlock();
    }

    int nSize = *((DWORD*)pSudio + 5);
    if(i < 0 || i >= nSize)
    {
		const char *pszName = ( pSudio->m_pStudioHdr ) ? pSudio->m_pStudioHdr->pszName() : "<<null>>";
        Msg( "Invalid index passed to CStudioHdr(%s)::GroupStudioHdr(): %d, but max is %d\n", pszName, i, pSudio->m_pStudioHdrCache.Count() );
        return pSudio->GetRenderHdr();
    }

    int v1 = *((DWORD *)pSudio + 2);
    int v2 = 4 * i;

    const studiohdr_t *pStudioHdr = *(studiohdr_t **)(v1 + v2);
    if(pStudioHdr == nullptr)
    {
        int v3 = *((DWORD *)pSudio + 1);
        int v4 = 144 * i;

        virtualgroup_t *pGroup = (virtualgroup_t *)(*(DWORD *)(v3 + 88) + v4);

        pStudioHdr = pGroup->GetStudioHdr();
        *(studiohdr_t **)(v1 + v2) = (studiohdr_t *)pStudioHdr;
    }
    return pStudioHdr;
}

const mstudioattachment_t &pAttachments(const CStudioHdr *pStudio, int i)
{
    int v2 = *((DWORD *)pStudio + 1);
    if(!v2)
    {
        return *pStudio->GetRenderHdr()->pLocalAttachment(i);
    }

    int v1 = 8 * i;
    virtualgeneric_t &m_attachment = *(virtualgeneric_t *)(v1 + *(DWORD*)(v2 + 48));

    const studiohdr_t *pStudioHdr = GroupStudioHdr(pStudio, m_attachment.group); //(*(DWORD*)(*(DWORD*)(v2 + 48) + 8 * i))
    return *pStudioHdr->pLocalAttachment(m_attachment.index); //(*(DWORD *)(*(DWORD *)(v2 + 48) + 8 * i + 4))
}

int StudioFindAttachment(const CStudioHdr *pStudioHdr, const char* szName)
{
    if(pStudioHdr && SeqencesAvailable(pStudioHdr))
    {
        for(int i = 0; i < GetNumAttachments(pStudioHdr); i++)
        {
            if(!V_stricmp(szName, pAttachments(pStudioHdr, i).pszName()))
            {
                return i;
            }
        }
    }
    return -1;
}

int StudioBoneIndexByName( const CStudioHdr *pStudioHdr, const char *pName )
{
	if ( pStudioHdr )
	{
		int start = 0, end = pStudioHdr->numbones()-1;
		const byte *pBoneTable = pStudioHdr->GetBoneTableSortedByName();
		mstudiobone_t *pbones = pStudioHdr->pBone( 0 );
		while (start <= end)
		{
			int mid = (start + end) >> 1;
			int cmp = V_stricmp( pbones[pBoneTable[mid]].pszName(), pName );

			if ( cmp < 0 )
			{
				start = mid + 1;
			}
			else if ( cmp > 0 )
			{
				end = mid - 1;
			}
			else
			{
				return pBoneTable[mid];
			}
		}
	}

	return -1;
}

const mstudioposeparamdesc_t &pPoseParameter(const CStudioHdr *pStudioHdrs, int i)
{
    int v3 = *((DWORD *)pStudioHdrs + 1);
    if(v3)
    {
        int v1 = 8 * i;
        virtualgeneric_t &m_pose = *(virtualgeneric_t*)(v1 + *(DWORD*)(v3 + 68));
        //int group = *(DWORD*)(*(DWORD*)(v3 + 68) + 8 * i);
        //int index = *(DWORD*)(*(DWORD*)(v3 + 68) + 8 * i + 4);

        if(!m_pose.group)
        {
            return *pStudioHdrs->GetRenderHdr()->pLocalPoseParameter(m_pose.index);
        }

        const studiohdr_t *pStudioHdr = GroupStudioHdr(pStudioHdrs, m_pose.group);
        return *pStudioHdr->pLocalPoseParameter(m_pose.index);
    }
    return *pStudioHdrs->GetRenderHdr()->pLocalPoseParameter(i);
}

int GetNumSeq(const CStudioHdr* pStudio)
{
    int v1 = *((DWORD*)pStudio + 1);
    if(v1)
        return *(DWORD*)(v1 + 20);
    else
        return *(DWORD*)(*(DWORD*)pStudio + 188);
}

mstudioseqdesc_t &pSeqdesc(const CStudioHdr* pStudioHdr, int i)
{
    if(i < 0 || i >= GetNumSeq(pStudioHdr))
    {
        i = 0;
    }

    int v1 = *((DWORD *)pStudioHdr + 1);
    if(v1)
    {
        int v2 = 16 * i;
        virtualsequence_t &m_seq = *(virtualsequence_t*)(v2 + *(DWORD*)(v1 + 8));

        const studiohdr_t *pSudio = GroupStudioHdr(pStudioHdr, m_seq.group);
        return *pSudio->pLocalSeqdesc(m_seq.index);
    }

    return *pStudioHdr->m_pStudioHdr->pLocalSeqdesc(i);
}

mstudioanimdesc_t &pAnimdesc(CStudioHdr *pStudioHdr, int i )
{
    int v1 = *((DWORD *)pStudioHdr + 1);
    if(v1)
    {
        int v2 = 28 * i;
        virtualsequence_t &m_seq = *(virtualsequence_t*)(v2 + *(DWORD*)(v1 + 8));

        const studiohdr_t *pSudio = GroupStudioHdr(pStudioHdr, m_seq.group);

        return *pSudio->pLocalAnimdesc(m_seq.index);
    }

    return *pStudioHdr->m_pStudioHdr->pLocalAnimdesc(i);
}

int GetSharedPoseParameter(CStudioHdr *pThis, int a2, int a3)
{
    int result; // eax
    int v4; // ecx

    result = a3;
    v4 = *((DWORD *)pThis + 1);
    if ( v4 )
    {
        if ( a3 == -1 )
            return -1;
        else
            return *(DWORD *)(*(DWORD *)(*(DWORD *)(v4 + 88) + 144 * *(DWORD *)(*(DWORD *)(v4 + 8) + 16 * a2 + 8) + 104) + 4 * a3);
    }
    return result;
}

void Studio_LocalPoseParameter( const CStudioHdr *pStudioHdr, const float poseParameter[], mstudioseqdesc_t &seqdesc, int iSequence, int iLocalIndex, float &flSetting, int &index )
{
	if (!pStudioHdr)
	{
		flSetting = 0;
		index = 0;
		return;
	}

	int iPose = GetSharedPoseParameter(const_cast<CStudioHdr*>(pStudioHdr), iSequence, seqdesc.paramindex[iLocalIndex] );

	if (iPose == -1)
	{
		flSetting = 0;
		index = 0;
		return;
	}

	const mstudioposeparamdesc_t &Pose = pPoseParameter(pStudioHdr, iPose );

	float flValue = poseParameter[iPose];

	if (Pose.loop)
	{
		float wrap = (Pose.start + Pose.end) / 2.0 + Pose.loop / 2.0;
		float shift = Pose.loop - wrap;

		flValue = flValue - Pose.loop * floor((flValue + shift) / Pose.loop);
	}

	if (seqdesc.posekeyindex == 0)
	{
		float flLocalStart	= ((float)seqdesc.paramstart[iLocalIndex] - Pose.start) / (Pose.end - Pose.start);
		float flLocalEnd	= ((float)seqdesc.paramend[iLocalIndex] - Pose.start) / (Pose.end - Pose.start);

		// convert into local range
		flSetting = (flValue - flLocalStart) / (flLocalEnd - flLocalStart);

		// clamp.  This shouldn't ever need to happen if it's looping.
		if (flSetting < 0)
			flSetting = 0;
		if (flSetting > 1)
			flSetting = 1;

		index = 0;
		if (seqdesc.groupsize[iLocalIndex] > 2 )
		{
			// estimate index
			index = (int)(flSetting * (seqdesc.groupsize[iLocalIndex] - 1));
			if (index == seqdesc.groupsize[iLocalIndex] - 1) index = seqdesc.groupsize[iLocalIndex] - 2;
			flSetting = flSetting * (seqdesc.groupsize[iLocalIndex] - 1) - index;
		}
	}
	else
	{
		flValue = flValue * (Pose.end - Pose.start) + Pose.start;
		index = 0;

		while (1)
		{
			flSetting = (flValue - seqdesc.poseKey( iLocalIndex, index )) / (seqdesc.poseKey( iLocalIndex, index + 1 ) - seqdesc.poseKey( iLocalIndex, index ));
			if (index < seqdesc.groupsize[iLocalIndex] - 2 && flSetting > 1.0)
			{
				index++;
				continue;
			}
			break;
		}

		// clamp.
		if (flSetting < 0.0f)
			flSetting = 0.0f;
		if (flSetting > 1.0f)
			flSetting = 1.0f;
	}
}

int iRelativeAnim(CStudioHdr *pThis, int a2, int a3)
{
    int v3; // edx

    v3 = *((DWORD *)pThis + 1);
    if ( v3 )
        return *(DWORD *)(*(DWORD *)(*(DWORD *)(v3 + 88) + 144 * *(DWORD *)(*(DWORD *)(v3 + 8) + 16 * a2 + 8) + 64) + 4 * a3);
    else
        return a3;
}

void Studio_SeqAnims( const CStudioHdr *pStudioHdr, mstudioseqdesc_t &seqdesc, int iSequence, const float poseParameter[], mstudioanimdesc_t *panim[4], float *weight )
{
	if (!pStudioHdr || iSequence >= GetNumSeq(pStudioHdr))
	{
		weight[0] = weight[1] = weight[2] = weight[3] = 0.0;
		return;
	}

	int i0 = 0, i1 = 0;
	float s0 = 0, s1 = 0;
	
	Studio_LocalPoseParameter( pStudioHdr, poseParameter, seqdesc, iSequence, 0, s0, i0 );
	Studio_LocalPoseParameter( pStudioHdr, poseParameter, seqdesc, iSequence, 1, s1, i1 );

	panim[0] = &pAnimdesc(const_cast<CStudioHdr*>(pStudioHdr), iRelativeAnim(const_cast<CStudioHdr*>(pStudioHdr), iSequence, seqdesc.anim( i0  , i1 ) ) );
	weight[0] = (1 - s0) * (1 - s1);

	panim[1] = &pAnimdesc(const_cast<CStudioHdr*>(pStudioHdr), iRelativeAnim(const_cast<CStudioHdr*>(pStudioHdr), iSequence, seqdesc.anim( i0+1, i1 ) ) );
	weight[1] = (s0) * (1 - s1);

	panim[2] = &pAnimdesc(const_cast<CStudioHdr*>(pStudioHdr), iRelativeAnim(const_cast<CStudioHdr*>(pStudioHdr), iSequence, seqdesc.anim( i0  , i1+1 ) ) );
	weight[2] = (1 - s0) * (s1);

	panim[3] = &pAnimdesc(const_cast<CStudioHdr*>(pStudioHdr), iRelativeAnim(const_cast<CStudioHdr*>(pStudioHdr), iSequence, seqdesc.anim( i0+1, i1+1 ) ) );
	weight[3] = (s0) * (s1);
}


float Studio_CPS( const CStudioHdr *pStudioHdr, mstudioseqdesc_t &seqdesc, int iSequence, const float poseParameter[] )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, seqdesc, iSequence, poseParameter, panim, weight );

	float t = 0;

	for (int i = 0; i < 4; i++)
	{
		if (weight[i] > 0 && panim[i]->numframes > 1)
		{
			t += (panim[i]->fps / (panim[i]->numframes - 1)) * weight[i];
		}
	}
	return t;
}

float Studio_Duration( const CStudioHdr *pStudioHdr, int iSequence, const float poseParameter[] )
{
    mstudioseqdesc_t &seqdesc_ = pSeqdesc(pStudioHdr, iSequence);
    float cps = Studio_CPS(pStudioHdr, seqdesc_, iSequence, poseParameter);

    if(cps == 0)
        return 0.0f;

    return 1.0f/cps;
}

int GetSequenceFlags(const CStudioHdr* pStudioHdr, int nSequence)
{
    if( !pStudioHdr || 
        !SeqencesAvailable(pStudioHdr) || 
        nSequence < 0 || 
        nSequence >= GetNumSeq(pStudioHdr))
    {
        return 0;
    }

    mstudioseqdesc_t &seqdesc = pSeqdesc(pStudioHdr, nSequence);
    return seqdesc.flags;
}

void SetEventIndexForSequence(mstudioseqdesc_t &seqdesc)
{
    seqdesc.flags |= STUDIO_EVENT;
    if(seqdesc.numevents == 0)
    {
        return;
    }

    for(int index = 0; index < (int)seqdesc.numevents; index++)
    {
        mstudioevent_t* pEvents = seqdesc.pEvent(index);
        if(!pEvents)
        {
            continue;
        }
        
        if(pEvents->type & (1 << 10))
        {
            const char *pEventName = pEvents->pszEventName();
            int iEventIndex = g_CallHelper->EventListIndexForName(pEventName);
            if(iEventIndex == -1)
            {
                pEvents->event = g_CallHelper->EventListRegisterPrivateEvent(pEventName);
            }
            else
            {
                pEvents->event = iEventIndex;
                pEvents->type |= g_CallHelper->EventListGetEventType(iEventIndex);
            }
        }
    }
}

float Studio_SetPoseParameter( const CStudioHdr *pStudioHdr, int iParameter, float flValue, float &ctlValue )
{
	if (iParameter < 0 || iParameter >= GetNumPoseParameters(pStudioHdr))
	{
		return 0;
	}

	const mstudioposeparamdesc_t &PoseParam = pPoseParameter(pStudioHdr, iParameter );
	if (PoseParam.loop)
	{
		float wrap = (PoseParam.start + PoseParam.end) / 2.0 + PoseParam.loop / 2.0;
		float shift = PoseParam.loop - wrap;

		flValue = flValue - PoseParam.loop * floor((flValue + shift) / PoseParam.loop);
	}

	ctlValue = (flValue - PoseParam.start) / (PoseParam.end - PoseParam.start);

	if (ctlValue < 0) ctlValue = 0;
	if (ctlValue > 1) ctlValue = 1;

	return ctlValue * (PoseParam.end - PoseParam.start) + PoseParam.start;
}

float Studio_GetPoseParameter( const CStudioHdr *pStudioHdr, int iParameter, float ctlValue )
{
	if (iParameter < 0 || iParameter >= GetNumPoseParameters(pStudioHdr))
	{
		return 0;
	}

	const mstudioposeparamdesc_t &PoseParam = pPoseParameter(pStudioHdr, iParameter );

	return ctlValue * (PoseParam.end - PoseParam.start) + PoseParam.start;
}

int FindBodygroupByName( CStudioHdr *pstudiohdr, const char *name )
{
	if ( !pstudiohdr || !pstudiohdr->IsValid() )
		return -1;

	int group;
	for ( group = 0; group < pstudiohdr->numbodyparts(); group++ )
	{
		mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( group );
		if ( !Q_strcasecmp( name, pbodypart->pszName() ) )
		{
			return group;
		}
	}

	return -1;
}

int GetNumBodyGroups( CStudioHdr *pstudiohdr )
{
	if ( !pstudiohdr )
		return 0;

	return pstudiohdr->numbodyparts();
}

int GetBodygroup( CStudioHdr *pstudiohdr, int body, int iGroup )
{
	if (! pstudiohdr)
		return 0;

	if (iGroup >= pstudiohdr->numbodyparts())
		return 0;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );

	if (pbodypart->nummodels <= 1)
		return 0;

	int iCurrent = (body / pbodypart->base) % pbodypart->nummodels;

	return iCurrent;
}

const char *GetBodygroupPartName( CStudioHdr *pstudiohdr, int iGroup, int iPart )
{
	if ( !pstudiohdr)
		return "";

	if (iGroup >= pstudiohdr->numbodyparts())
		return "";

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );
	if ( iPart < 0 && iPart >= pbodypart->nummodels )
		return "";

	return pbodypart->pModel( iPart )->name;
}

int GetBodygroupCount( CStudioHdr *pstudiohdr, int iGroup )
{
	if ( !pstudiohdr )
		return 0;

	if (iGroup >= pstudiohdr->numbodyparts())
		return 0;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );
	return pbodypart->nummodels;
}


void SetBodygroup( CStudioHdr *pstudiohdr, int& body, int iGroup, int iValue )
{
	if (! pstudiohdr)
		return;

	if (iGroup >= pstudiohdr->numbodyparts())
		return;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );

	if (iValue >= pbodypart->nummodels)
		return;

	int iCurrent = (body / pbodypart->base) % pbodypart->nummodels;

	body = (body - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));
}

const char *GetSequenceActivityName( CStudioHdr *pstudiohdr, int iSequence )
{
	if( !pstudiohdr || iSequence < 0 || iSequence >= GetNumSeq(pstudiohdr) )
	{
		return "Unknown";
	}

	mstudioseqdesc_t	&seqdesc = pSeqdesc(pstudiohdr, iSequence );
	return seqdesc.pszActivityName( );
}

void SetActivityForSequence( CStudioHdr *pstudiohdr, int i )
{
	int iActivityIndex;
	const char *pszActivityName;
	mstudioseqdesc_t &seqdesc = pSeqdesc(pstudiohdr, i );

	seqdesc.flags |= STUDIO_ACTIVITY;

	pszActivityName = GetSequenceActivityName( pstudiohdr, i );
	if ( pszActivityName[0] != '\0' )
	{
		iActivityIndex = g_CallHelper->GetIndexForName( pszActivityName );
		
		if ( iActivityIndex == -1 )
		{
			seqdesc.activity = g_CallHelper->RegisterPrivateActivity( pszActivityName );
		}
		else
		{
			seqdesc.activity = iActivityIndex;
		}
	}
}

int GetSequenceActivity( CStudioHdr *pstudiohdr, int sequence, int *pweight )
{
	if (!pstudiohdr || !SeqencesAvailable(pstudiohdr) )
	{
		if (pweight)
			*pweight = 0;
		return 0;
	}

    mstudioseqdesc_t &seqdest = pSeqdesc(pstudiohdr, sequence);
	if (!(seqdest.flags & STUDIO_ACTIVITY))
	{
		SetActivityForSequence( pstudiohdr, sequence );
	}
	if (pweight)
		*pweight = seqdest.actweight;
	return seqdest.activity;
}