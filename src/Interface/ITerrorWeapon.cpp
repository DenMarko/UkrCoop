#include "ITerrorWeapon.h"
#include "ITerrorPlayer.h"

void ITerrorWeapon::SetHelpingHandState(HelpingHandState state)
{
    HelpingHandState &m_helpingHandState = access_member<HelpingHandState>(this, 1460);
    IntervalTimers &m_HelpingHandTimer = access_member<IntervalTimers>(this, 1452);
    CHandle<IBaseEntity> &m_helpingHandTarget = access_member<CHandle<IBaseEntity>>(this, 1464);
    float &m_helpingHandExtendDuration = access_member<float>(this, 1468);

    if(m_helpingHandState == state)
        return;

    int oldState = m_helpingHandState;
    NetworkStateChanged(1460);
    m_helpingHandState = state;

    m_HelpingHandTimer.Start();

    ITerrorPlayer *pPlayerOwner = (ITerrorPlayer*)GetPlayerOwner();

    if(m_helpingHandState > 5)
        return;

    switch (m_helpingHandState)
    {
        case HELPING_HAND_DISABLED:
        {
            if(m_helpingHandTarget != NULL)
            {
                m_helpingHandTarget.Term();
                NetworkStateChanged(1464);
            }

            if(oldState != 1 && this == pPlayerOwner->GetActiveTerrorWeapon())
            {
                bool supportsHelpingHands = false;
                switch (GetIdealActivity())
                {
                    case ACT_VM_ITEMPICKUP_LOOP_PISTOL: // 1322
                    case ACT_VM_ITEMPICKUP_EXTEND_PISTOL: // 1324
                    case ACT_VM_HELPINGHAND_LOOP_PISTOL: // 1328
                    case ACT_VM_HELPINGHAND_EXTEND_PISTOL: // 1330
                    case ACT_VM_ITEMPICKUP_LOOP_DUAL_PISTOL: // 1334
                    case ACT_VM_ITEMPICKUP_EXTEND_DUAL_PISTOL: // 1336
                    case ACT_VM_HELPINGHAND_LOOP_DUAL_PISTOL: // 1340
                    case ACT_VM_HELPINGHAND_EXTEND_DUAL_PISTOL: // 1342
                    case ACT_VM_ITEMPICKUP_LOOP_RIFLE: // 1346
                    case ACT_VM_ITEMPICKUP_EXTEND_RIFLE: // 1348
                    case ACT_VM_HELPINGHAND_LOOP_RIFLE: // 1352
                    case ACT_VM_HELPINGHAND_EXTEND_RIFLE: // 1354
                    case ACT_VM_ITEMPICKUP_LOOP_SMG: // 1358
                    case ACT_VM_ITEMPICKUP_EXTEND_SMG: // 1360
                    case ACT_VM_HELPINGHAND_LOOP_SMG: // 1364
                    case ACT_VM_HELPINGHAND_EXTEND_SMG: // 1366
                    case ACT_VM_ITEMPICKUP_LOOP_SHOTGUN: // 1370
                    case ACT_VM_ITEMPICKUP_EXTEND_SHOTGUN: // 1372
                    case ACT_VM_HELPINGHAND_LOOP_SHOTGUN: // 1376
                    case ACT_VM_HELPINGHAND_EXTEND_SHOTGUN: // 1378
                    case ACT_VM_ITEMPICKUP_LOOP_AUTOSHOTGUN: // 1382
                    case ACT_VM_ITEMPICKUP_EXTEND_AUTOSHOTGUN: // 1384
                    case ACT_VM_HELPINGHAND_LOOP_AUTOSHOTGUN: // 1388
                    case ACT_VM_HELPINGHAND_EXTEND_AUTOSHOTGUN: // 1390
                    case ACT_VM_ITEMPICKUP_LOOP_SNIPER: // 1394
                    case ACT_VM_ITEMPICKUP_EXTEND_SNIPER: // 1396
                    case ACT_VM_HELPINGHAND_LOOP_SNIPER: // 1400
                    case ACT_VM_HELPINGHAND_EXTEND_SNIPER: // 1402
                    case ACT_VM_ITEMPICKUP_LOOP_PIPEBOMB: // 1406
                    case ACT_VM_ITEMPICKUP_EXTEND_PIPEBOMB: // 1408
                    case ACT_VM_HELPINGHAND_LOOP_PIPEBOMB: // 1412
                    case ACT_VM_HELPINGHAND_EXTEND_PIPEBOMB: // 1414
                    case ACT_VM_ITEMPICKUP_LOOP_MOLOTOV: // 1418
                    case ACT_VM_ITEMPICKUP_EXTEND_MOLOTOV: // 1420
                    case ACT_VM_HELPINGHAND_LOOP_MOLOTOV: // 1424
                    case ACT_VM_HELPINGHAND_EXTEND_MOLOTOV: // 1426
                    case ACT_VM_ITEMPICKUP_LOOP_MEDKIT: // 1430
                    case ACT_VM_ITEMPICKUP_EXTEND_MEDKIT: // 1432
                    case ACT_VM_HELPINGHAND_LOOP_MEDKIT: // 1436
                    case ACT_VM_HELPINGHAND_EXTEND_MEDKIT: // 1438
                    case ACT_VM_ITEMPICKUP_LOOP_PAINPILLS: // 1442
                    case ACT_VM_ITEMPICKUP_EXTEND_PAINPILLS: // 1444
                    case ACT_VM_HELPINGHAND_LOOP_PAINPILLS: // 1448
                    case ACT_VM_HELPINGHAND_EXTEND_PAINPILLS: // 1450
                    case ACT_VM_IDLE_BAREHAND: // 1632
                    case ACT_VM_ITEMPICKUP_EXTEND_BAREHAND: // 1637
                    case ACT_VM_ITEMPICKUP_LOOP_BAREHAND: // 1639
                    case ACT_VM_HELPINGHAND_EXTEND_BAREHAND: // 1643
                    case ACT_VM_HELPINGHAND_LOOP_BAREHAND: // 1645
                    case ACT_VM_IDLE_DUAL_PISTOL: // 1649
                    case ACT_VM_IDLE_PISTOL: // 1678
                    case ACT_VM_IDLE_RIFLE: // 1695
                    case ACT_VM_IDLE_SMG: // 1721
                    case ACT_VM_IDLE_SNIPER: // 1745
                    case ACT_VM_IDLE_SHOTGUN: // 1769
                    case ACT_VM_IDLE_AUTOSHOTGUN: // 1794
                    case ACT_VM_IDLE_PIPEBOMB: // 1819
                    case ACT_VM_IDLE_MOLOTOV: // 1828
                    case ACT_VM_IDLE_PAINPILLS: // 1837
                    case ACT_VM_IDLE_MEDKIT: // 1844
                    {
                        supportsHelpingHands = true;
                        break;
                    }
                    default:
                    {
                        supportsHelpingHands = false;
                        break;
                    }
                }

                if(supportsHelpingHands)
                {
                    if(oldState == 4 || oldState == 5)
                        SendWeaponAnim(ACT_VM_HELPINGHAND_RETRACT);
                    else
                        SendWeaponAnim(ACT_VM_ITEMPICKUP_RETRACT);
                }

                if(!m_bInReload)
                {
                    pPlayerOwner->m_flNextAttack = g_pGlobals->curtime;
                    m_flNextPrimaryAttack = g_pGlobals->curtime;
                    m_flNextSecondaryAttack = g_pGlobals->curtime;
                }
            }

            break;
        }
        case HELPING_HAND_READY:
        {
            SendWeaponAnim(ACT_VM_ITEMPICKUP_EXTEND);
            NetworkStateChanged(1468);
            m_helpingHandExtendDuration = SequenceDuration(GetSequence());
            break;
        } 
        case HELPING_HAND_ACTIVE:
        {
            SendWeaponAnim(ACT_VM_ITEMPICKUP_LOOP);
            break;
        } 
        case HELPING_HAND_PULLING:
        {
            SendWeaponAnim(ACT_VM_HELPINGHAND_EXTEND);
            NetworkStateChanged(1468);
            m_helpingHandExtendDuration = SequenceDuration(GetSequence());
            break;
        } 
        case HELPING_HAND_COMPLETE:
        {
            SendWeaponAnim(ACT_VM_HELPINGHAND_LOOP);
            break;
        }
        default:
        {
            return;
        }
    }
}

void ITerrorWeapon::SuppressHelpingHands(float duration)
{
    static ConVarRef survivor_helping_hand_inhibit_duration("survivor_helping_hand_inhibit_duration");
    IntervalTimers &m_helperTimer = access_member<IntervalTimers>(this, 1496);
    CountdownTimers &m_suppressTimer = access_member<CountdownTimers>(this, 1484);

    if(duration <= 0.f)
        duration = survivor_helping_hand_inhibit_duration.GetFloat();

    m_helperTimer.Invalidate();

    SetHelpingHandState(HELPING_HAND_DISABLED);

    if(!m_suppressTimer.HasStarted() 
    || duration > m_suppressTimer.GetRemainingTime())
    {
        m_suppressTimer.Start(duration);
    }
}