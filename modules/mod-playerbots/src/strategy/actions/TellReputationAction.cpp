/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellReputationAction.h"
#include "ReputationMgr.h"
#include "Event.h"
#include "Playerbots.h"

bool TellReputationAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid selection = master->GetTarget();
    if (selection.IsEmpty())
        return false;

    Unit* unit = ObjectAccessor::GetUnit(*master, selection);
    if (!unit)
        return false;

    FactionTemplateEntry const* factionTemplate = unit->GetFactionTemplateEntry();
    if(!factionTemplate)
        return false;//修复宕机
    uint32 faction = factionTemplate->faction;
    if (!faction)
        return false;//修复宕机
    FactionEntry const* entry = sFactionStore.LookupEntry(faction);
    if (!entry)
        return false;//修复宕机
    int32 reputation = bot->GetReputationMgr().GetReputation(faction);
    if (!reputation)
        return false;//修复宕机
    std::ostringstream out;
    out << entry->name[0] << ": ";
    out << "|cff";

    ReputationRank rank = bot->GetReputationMgr().GetRank(entry);
    if (!rank)
        return false;//修复宕机
    switch (rank)
    {
        case REP_HATED:
            out << "cc2222hated";
            break;
        case REP_HOSTILE:
            out << "ff0000hostile";
            break;
        case REP_UNFRIENDLY:
            out << "ee6622unfriendly";
            break;
        case REP_NEUTRAL:
            out << "ffff00neutral";
            break;
        case REP_FRIENDLY:
            out << "00ff00friendly";
            break;
        case REP_HONORED:
            out << "00ff88honored";
            break;
        case REP_REVERED:
            out << "00ffccrevered";
            break;
        case REP_EXALTED:
            out << "00ffffexalted";
            break;
        default:
            out << "808080unknown";
            break;
    }

    out << "|cffffffff";

    int32 base = ReputationMgr::Reputation_Cap + 1;
    for (int32 i = MAX_REPUTATION_RANK - 1; i >= rank; --i)
        base -= ReputationMgr::PointsInRank[i];

    out << " (" << (reputation - base) << "/" << ReputationMgr::PointsInRank[rank] << ")";
    botAI->TellMaster(out);

    return true;
}
