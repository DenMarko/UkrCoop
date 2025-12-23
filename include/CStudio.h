#ifndef _INCLUDE_STUDIO_HEADER_
#define _INCLUDE_STUDIO_HEADER_

#include "studio.h"

extern const virtualmodel_t *ResetVModel(const CStudioHdr *pStudio, const virtualmodel_t* pVModel);
extern bool SeqencesAvailable(const CStudioHdr *pStudio);
extern int GetNumAttachments(const CStudioHdr *pStudio);
extern int GetNumPoseParameters(const CStudioHdr* pStudio);
extern const studiohdr_t* GroupStudioHdr(const CStudioHdr *pSudio, int i);
extern const mstudioattachment_t &pAttachments(const CStudioHdr *pStudio, int i);
extern int StudioFindAttachment(const CStudioHdr *pStudioHdr, const char* szName);
extern int StudioBoneIndexByName( const CStudioHdr *pStudioHdr, const char *pName );
extern const mstudioposeparamdesc_t &pPoseParameter(const CStudioHdr *pStudioHdrs, int i);
extern int GetSequenceFlags(const CStudioHdr* pStudioHdr, int nSequence);
extern mstudioseqdesc_t &pSeqdesc(const CStudioHdr* pStudioHdr, int i);
extern void SetEventIndexForSequence(mstudioseqdesc_t &seqdesc);
extern int GetNumSeq(const CStudioHdr* pStudio);
extern float Studio_Duration( const CStudioHdr *pStudioHdr, int iSequence, const float poseParameter[] );
extern float Studio_SetPoseParameter( const CStudioHdr *pStudioHdr, int iParameter, float flValue, float &ctlValue );
extern float Studio_GetPoseParameter( const CStudioHdr *pStudioHdr, int iParameter, float ctlValue );
extern int FindBodygroupByName( CStudioHdr *pstudiohdr, const char *name );
extern int GetNumBodyGroups( CStudioHdr *pstudiohdr );
extern int GetBodygroup( CStudioHdr *pstudiohdr, int body, int iGroup );
extern const char *GetBodygroupPartName( CStudioHdr *pstudiohdr, int iGroup, int iPart );
extern int GetBodygroupCount( CStudioHdr *pstudiohdr, int iGroup );
extern void SetBodygroup( CStudioHdr *pstudiohdr, int& body, int iGroup, int iValue );
extern int GetSequenceActivity( CStudioHdr *pstudiohdr, int sequence, int *pweight = NULL );
extern int GetSharedPoseParameter(CStudioHdr *pThis, int iSequence, int iLocalPose);

#endif