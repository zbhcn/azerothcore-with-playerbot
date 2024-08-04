/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UseMeetingStoneAction.h"
#include "Event.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool UseMeetingStoneAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid guid;
    p >> guid;

	if (master->GetTarget() && master->GetTarget() != bot->GetGUID())
		return false;

	if (!master->GetTarget() && master->GetGroup() != bot->GetGroup())
		return false;

    if (master->IsBeingTeleported())
        return false;

    if (bot->IsInCombat())
    {
        botAI->TellError("我在战斗中");
        return false;
    }

    Map* map = master->GetMap();
    if (!map)
        return false;

    GameObject *gameObject = map->GetGameObject(guid);
    if (!gameObject)
        return false;

	GameObjectTemplate const* goInfo = gameObject->GetGOInfo();
	if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SUMMONING_RITUAL)
        return false;

    return Teleport(master, bot);
}

class AnyGameObjectInObjectRangeCheck
{
    public:
        AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) { }
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(GameObject* go)
        {
            if (go && i_obj->IsWithinDistInMap(go, i_range) && go->isSpawned() && go->GetGOInfo())
                return true;

            return false;
        }

    private:
        WorldObject const* i_obj;
        float i_range;
};

bool SummonAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;
    
    if (Pet* pet = bot->GetPet()) {
        pet->SetReactState(REACT_PASSIVE);
        pet->GetCharmInfo()->SetIsCommandFollow(true);
        pet->GetCharmInfo()->IsReturning();
    }

    if (master->GetSession()->GetSecurity() >= SEC_PLAYER) {
        // botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({});
        SET_AI_VALUE(std::list<FleeInfo>, "recently flee info", {});
        return Teleport(master, bot);
    }

    if (SummonUsingGos(master, bot) || SummonUsingNpcs(master, bot))
    {
        botAI->TellMasterNoFacing("Hi!");
        return true;
    }

    if (SummonUsingGos(bot, master) || SummonUsingNpcs(bot, master))
    {
        botAI->TellMasterNoFacing("欢迎!");
        return true;
    }

    return false;
}

bool SummonAction::SummonUsingGos(Player* summoner, Player* player)
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(summoner, sPlayerbotAIConfig->sightDistance);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(summoner, targets, u_check);
    Cell::VisitAllObjects(summoner, searcher, sPlayerbotAIConfig->sightDistance);

    for (GameObject* go : targets)
    {
        if (go->isSpawned() && go->GetGoType() == GAMEOBJECT_TYPE_MEETINGSTONE)
            return Teleport(summoner, player);
    }

    botAI->TellError(summoner == bot ? "附近没有集合石" : "你附近没有集合石");
    return false;
}

bool SummonAction::SummonUsingNpcs(Player* summoner, Player* player)
{
    if (!sPlayerbotAIConfig->summonAtInnkeepersEnabled)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(summoner, sPlayerbotAIConfig->sightDistance);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(summoner, targets, u_check);
    Cell::VisitAllObjects(summoner, searcher, sPlayerbotAIConfig->sightDistance);

    for (Unit* unit : targets)
    {
        if (unit && unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER))
        {
            if (!player->HasItemCount(6948, 1, false))
            {
                botAI->TellError(player == bot ? "我没有炉石" : "你没有炉石");
                return false;
            }

            if (player->HasSpellCooldown(8690))
            {
                botAI->TellError(player == bot ? "我的炉石没冷却" : "你的炉石没冷却");
                return false;
            }

            // Trigger cooldown
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(8690);
            if (!spellInfo)
                return false;

            Spell spell(player, spellInfo, TRIGGERED_NONE);
            spell.SendSpellCooldown();

            return Teleport(summoner, player);
        }
    }

    botAI->TellError(summoner == bot ? "附近没有客栈老板" : "你的附近没有客栈老板");
    return false;
}

bool SummonAction::Teleport(Player* summoner, Player* player)
{
    Player* master = GetMaster();
    if(!master)
        return false;
    if (master->GetMap() && master->GetMap()->IsDungeon()) {
        InstanceMap* map = master->GetMap()->ToInstanceMap();
        if (map) {
            if (map->CannotEnter(player) == Map::CANNOT_ENTER_MAX_PLAYERS) {
                botAI->TellError("我无法进入这个副本");
                return false;
            }
        }
    }
    //if(master && master->GetMap() && !master->GetMap()->IsDungeon())
    //{
    //    botAI->TellError("你不能把机器人带出副本");
    //    return false;
    //}

    if (!summoner->IsBeingTeleported() && !player->IsBeingTeleported() && summoner != player)
    {
        float followAngle = GetFollowAngle();
        for (float angle = followAngle - M_PI; angle <= followAngle + M_PI; angle += M_PI / 4)
        {
            uint32 mapId = summoner->GetMapId();
            float x = summoner->GetPositionX() + cos(angle) * sPlayerbotAIConfig->followDistance;
            float y = summoner->GetPositionY() + sin(angle) * sPlayerbotAIConfig->followDistance;
            float z = summoner->GetPositionZ();

            if (summoner->IsWithinLOS(x, y, z) && master->IsInWorld())
            {
                if (master->IsInCombat() && !sPlayerbotAIConfig->allowSummonInCombat)
                {
                    botAI->TellError("无法在战斗中复活");
                    return false;
                }

                if (!master->IsAlive() && !sPlayerbotAIConfig->allowSummonWhenMasterIsDead)
                {
                    botAI->TellError("主人还没复活我无法复活");
                    return false;
                }

                if (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST) && !sPlayerbotAIConfig->allowSummonWhenBotIsDead)
                {
                    botAI->TellError("我死了，你不能召唤我，你需要先释放我的灵魂");
                    return false;
                }

                if (bot->isDead() && sPlayerbotAIConfig->reviveBotWhenSummoned)
                {
                    bot->ResurrectPlayer(1.0f, false);
                    if(sPlayerbotAIConfig->botRepairWhenSummon)
                        bot->DurabilityRepairAll(false, 1.0f, false);
                    botAI->TellMasterNoFacing("我复活了!");
                    botAI->ResetStrategies();
                    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
                    {
                        botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
                    }
                }
                player->GetMotionMaster()->Clear();
                player->TeleportTo(mapId, x, y, z, 0);
                return true;
            } 
        }
    }

    botAI->TellError("没有足够的空间传送");
    return false;
}
