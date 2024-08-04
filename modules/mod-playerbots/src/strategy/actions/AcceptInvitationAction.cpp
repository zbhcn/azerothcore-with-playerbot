/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AcceptInvitationAction.h"
#include "Event.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PlayerbotSecurity.h"
#include "WorldPacket.h"

bool AcceptInvitationAction::Execute(Event event)
{
    Group* grp = bot->GetGroupInvite();

    if (!grp)
        return false;
    WorldPacket packet = event.getPacket();
    uint8 flag;
    std::string name;
    packet >> flag >> name;

    // Player* inviter = ObjectAccessor::FindPlayer(grp->GetLeaderGUID());
    Player* inviter = ObjectAccessor::FindPlayerByName(name, true);

    //Player* inviter = ObjectAccessor::FindPlayer(grp->GetLeaderGUID());

    //Player* inviter = ObjectAccessor::FindPlayer(bot->Getinvitingplayer());
    //if (!inviter)
        //inviter = ObjectAccessor::FindPlayer(grp->GetLeaderGUID());
    if (!inviter)
        return false;

    if (!botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, inviter) || !bot->HasHealSpec())// && !bot->HasHealSpec())//允许奶妈组队
    {
        WorldPacket data(SMSG_GROUP_DECLINE, 10);
        data << bot->GetName();
        inviter->SendDirectMessage(&data);
        bot->UninviteFromGroup();
        return false;
    }

    WorldPacket p;
    uint32 roles_mask = 0;
    p << roles_mask;
    bot->GetSession()->HandleGroupAcceptOpcode(p);

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
        botAI->SetMaster(inviter);
    //else
        //sPlayerbotDbStore->Save(botAI);

    botAI->ResetStrategies();
    botAI->Reset();
    botAI->ChangeStrategy("+follow,-lfg,-bg", BOT_STATE_NON_COMBAT);
    botAI->TellMaster("你好,请多关照");

    if (sPlayerbotAIConfig->summonWhenGroup) {
        Teleport(inviter, bot);
    }
    return true;
}