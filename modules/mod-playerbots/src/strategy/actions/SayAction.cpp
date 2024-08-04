/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SayAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "ChannelMgr.h"
#include "GuildMgr.h"
#include <regex>

SayAction::SayAction(PlayerbotAI* botAI) : Action(botAI, "say"), Qualified()
{
}

bool SayAction::Execute(Event event)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target)
        target = AI_VALUE(Unit*, "current target");

    // set replace strings
    if (target) placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->getRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
        {
            switch (pItem->GetTemplate()->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    placeholders["<ammo>"] = "bullets";
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    placeholders["<ammo>"] = "arrows";
                    break;
            }
        }
    }

    if (bot->GetMap())
    {
        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetAreaId()))
            placeholders["<subzone>"] = area->area_name[0];
    }


    // set delay before next say
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    uint32 nextTime = time(nullptr) + urand(1, 30);
    botAI->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi) members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (auto & member : members)
        {
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                memberAi->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!sPlayerbotTextMgr->GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    else
        bot->Say(text, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    return true;
}

bool SayAction::isUseful()
{
    if (!botAI->AllowActivity())
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}

void ChatReplyAction::ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand
    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(msg);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (msg.find("？") != std::string::npos)
        is_quest = 1;
    if (word[0].find("有") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("谁") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "什")
        is_quest = 4;
    else if (word[0] == "哪")
        is_quest = 5;
    else if (word[0] == "为")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
//        // blame gm with chat tag
//        if (Player* plr = sObjectMgr->GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid1)))
//        {
//            if (plr->isGMChat())
//            {
//                replyType = REPLY_ADMIN_ABUSE;
//                found = true;
//                break;
//            }
//        }
//
        if (word[i] == "hi" || word[i] == "在吗" || word[i] == "hello" || word[i] == "你好")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "是" || word[i] == "吗" || word[i] == "嗯")
            {
                verb_pos = i;
                verb_type = 2; // present
                if (verb_pos == 0)
                    is_quest = 1;
            }
            else if (word[i] == "要" || word[i] == "组" || word[i] == "队")
            {
                verb_pos = i;
                verb_type = 3; // future
            }
            else if (word[i] == "已经" || word[i] == "过了")
            {
                verb_pos = i;
                verb_type = 1; // past
            }
            else if (word[i] == "闭嘴" || word[i] == "傻")
            {
                if (msg.find(bot->GetName()) == std::string::npos)
                {
                    continue; // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "对不起 %s, 我闭嘴了";
                    if (rnd == 1)
                        msg = "好的 %s";
                    if (rnd == 2)
                        msg = "好吧，我不会再和你说话了 %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
            case 2:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "悟空,你又调皮了";
                        break;
                    case 1:
                        msg = "咋回事？ %s";
                        break;
                    case 2:
                        msg = "谁？";
                        break;
                    case 3:
                        msg = "刚才没注意，怎么了？";
                        break;
                }

                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 4);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "没有";
                        break;
                    case 1:
                        msg = "我们都是";
                        break;
                    case 2:
                        msg = "也许是你, %s";
                        break;
                    case 3:
                        msg = "什么东东 %s";
                        break;
                    case 4:
                        msg = "是我吗?";
                        break;
                }

                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 4:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "等一下 %s";
                        break;
                    case 1:
                        msg = "可能要等一会儿";
                        break;
                    case 2:
                        msg = "绝不";
                        break;
                    case 3:
                        msg = "我看起来像什么，一个通灵者？";
                        break;
                    case 4:
                        msg = "几分钟，或者几天 ... 又或是几年?";
                        break;
                    case 5:
                        msg = "什么时候，好主意 %s";
                        break;
                    case 6:
                        msg = "不清楚 %s";
                        break;
                }

                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 5:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "真的想让我回答吗？";
                        break;
                    case 1:
                        msg = "在地图上？";
                        break;
                    case 2:
                        msg = "谁在乎";
                        break;
                    case 3:
                        msg = "afk?";
                        break;
                    case 4:
                        msg = "没有你的市场";
                        break;
                    case 5:
                        msg = "好的，在哪里？";
                        break;
                    case 6:
                        msg = "不知道 %s";
                        break;
                }

                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 6:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "我不懂 %s";
                        break;
                    case 1:
                        msg = "为什么？只是因为 %s";
                        break;
                    case 2:
                        msg = "为什么天空是蓝色的？";
                        break;
                    case 3:
                        msg = "别问我 %s, 我只是一个机器人";
                        break;
                    case 4:
                        msg = "你问错人了";
                        break;
                    case 5:
                        msg = "谁知道?";
                        break;
                    case 6:
                        msg = "鬼知道 %s";
                        break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            default:
            {
                switch (verb_type)
                {
                    case 1:
                    {
                        uint32 rnd = urand(0, 3);
                        std::string msg = "";

                        switch (rnd)
                        {
                            case 0:
                                msg = "这是真的, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                                break;
                            case 1:
                                msg = "好吧 %s 但是都过去了";
                                break;
                            case 2:
                                msg = "哈, 但是 " + word[verb_pos + 1] + " 会 " + word[verb_pos + 3] + " 尽管如此 %s";
                                break;
                            case 3:
                                msg = "之前没注意";
                                break;
                        }
                        msg = std::regex_replace(msg, std::regex("%s"), name);
                        respondsText = msg;
                        found = true;
                        break;
                    }
                    case 2:
                    {
                        uint32 rnd = urand(0, 6);
                        std::string msg = "";

                        switch (rnd)
                        {
                            case 0:
                                msg = "真的, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                                break;
                            case 1:
                                msg = "嗯 %s 那是真的";
                                break;
                            case 2:
                                msg = "可能 " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                                break;
                            case 3:
                                msg = "不清楚 %s";
                                break;
                            case 4:
                                msg = "不对吧？ %s";
                                break;
                            case 5:
                                msg = "是的";
                                break;
                            case 6:
                                msg = "不";
                                break;
                        }
                        msg = std::regex_replace(msg, std::regex("%s"), name);
                        respondsText = msg;
                        found = true;
                        break;
                    }
                    case 3:
                    {
                        uint32 rnd = urand(0, 8);
                        std::string msg = "";

                        switch (rnd)
                        {
                            case 0:
                                msg = "不清楚 %s";
                                break;
                            case 1:
                                msg = "难倒我了 %s";
                                break;
                            case 2:
                                msg = "我该怎么知道 %s";
                                break;
                            case 3:
                                msg = "别问我 %s, 我只是个机器人";
                                break;
                            case 4:
                                msg = "你问错人了";
                                break;
                            case 5:
                                msg = "我看起来像什么，一个小丑?";
                                break;
                            case 6:
                                msg = "当然 %s";
                                break;
                            case 7:
                                msg = "我不这样认为 %s";
                                break;
                            case 8:
                                msg = "也许吧";
                                break;
                        }
                        msg = std::regex_replace(msg, std::regex("%s"), name);
                        respondsText = msg;
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
            case 1:
            {
                uint32 rnd = urand(0, 2);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "yeah %s, 关键词是 " + word[verb_pos] + " " + word[verb_pos + 1];
                        break;
                    case 1:
                        msg = "ya %s 但是都过去了";
                        break;
                    case 2:
                        msg = word[verb_pos - 1] + " 将 " + word[verb_pos + 1] + " 尽管如此 %s";
                        break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 2);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "%s, 你什么意思 " + word[verb_pos + 1] + "?";
                        break;
                    case 1:
                        msg = "%s, 是什么 " + word[verb_pos + 1] + "?";
                        break;
                    case 2:
                        msg = "是的，我知道 " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " 是一个 " + word[verb_pos + 1];
                        break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 1);
                std::string msg = "";

                switch (rnd)
                {
                    case 0:
                        msg = "咋回事 %s?";
                        break;
                    case 1:
                        msg = "%s, 怎么了 %s?";
                        break;
                    case 2:
                        msg = "你是说 " + word[verb_pos - 1] + " 想 " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                        break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        // Name Responds
        if (msg.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // send responds
    //
    if (found)
    {
        // load text if needed
        if (respondsText.empty())
        {
            respondsText = BOT_TEXT2(replyType, name);
        }
        const char* c = respondsText.c_str();
        if (strlen(c) > 255)
            return;

        if (chanName == "世界频道")
        {
            if (ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId()))
            {
                std::string worldChan = "世界频道";
                if (Channel* chn = cMgr->GetJoinChannel(worldChan.c_str(), 0))
                    if (bot->GetTeamId() == TEAM_ALLIANCE)
                        chn->Say(bot->GetGUID(), c, LANG_COMMON);
                    else
                        chn->Say(bot->GetGUID(), c, LANG_ORCISH);
            }
        }
        else
        {
            if (type == CHAT_MSG_WHISPER)
            {
                ObjectGuid receiver = sCharacterCache->GetCharacterGuidByName(name);
                if (!receiver || !receiver.IsPlayer() || !ObjectAccessor::FindPlayer(receiver))
                {
                    return;
                }
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                {
                    bot->Whisper(c, LANG_COMMON, ObjectAccessor::FindPlayer(receiver));
                }
                else
                {
                    bot->Whisper(c, LANG_ORCISH, ObjectAccessor::FindPlayer(receiver));
                }
            }

            if (type == CHAT_MSG_SAY)
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                    bot->Say(respondsText, LANG_COMMON);
                else
                    bot->Say(respondsText, LANG_ORCISH);
            }

            if (type == CHAT_MSG_YELL)
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                    bot->Yell(respondsText, LANG_COMMON);
                else
                    bot->Yell(respondsText, LANG_ORCISH);
            }

            if (type == CHAT_MSG_GUILD)
            {
                if (!bot->GetGuildId())
                    return;

                Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
                if (!guild)
                    return;
                //禁止机器人在公会说空话替换为好的
                if (!respondsText.empty())
                    //respondsText = "好的";
                guild->BroadcastToGuild(bot->GetSession(), false, respondsText, LANG_UNIVERSAL);
            }
        }
        GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(nullptr) + urand(5, 25));
    }
}
