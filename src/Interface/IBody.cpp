#include "IBody.h"
#include "INextBot.h"

void IBody::AimHeadTowards(const Vector &lookAtPos, LookAtPriorityType priority, float duration, INextBotReply *replyWhenAimed, const char *reason)
{
    if(replyWhenAimed)
    {
        replyWhenAimed->OnFail(GetBot(), INextBotReply::FAILED);
    }
}

void IBody::AimHeadTowards(CBaseEntity *subject, LookAtPriorityType priority, float duration, INextBotReply *replyWhenAimed, const char *reason)
{
    if(replyWhenAimed)
    {
        replyWhenAimed->OnFail(GetBot(), INextBotReply::FAILED);
    }
}

bool IBody::IsHeadAimingOnTarget(void) const
{
    return false;
}

bool IBody::SetPosition( const Vector& pos )
{
    return true;
}

const Vector &IBody::GetEyePosition(void) const
{
    return vec3_origin;
}

const Vector &IBody::GetViewVector(void) const
{
    return vec3_origin;
}
