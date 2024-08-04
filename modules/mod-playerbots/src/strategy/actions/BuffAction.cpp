/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BuffAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbots.h"

class FindBuffVisitor : public IterateItemsVisitor
{
    public:
        FindBuffVisitor(Player* bot) : IterateItemsVisitor(), bot(bot)
        {
        }

        bool Visit(Item* item) override
        {
            if (bot->CanUseItem(item->GetTemplate()) != EQUIP_ERR_OK)
                return true;

            ItemTemplate const* proto = item->GetTemplate();

            if (proto->Class != ITEM_CLASS_CONSUMABLE)
                return true;

            if (proto->SubClass != ITEM_SUBCLASS_ELIXIR && proto->SubClass != ITEM_SUBCLASS_FLASK && proto->SubClass != ITEM_SUBCLASS_SCROLL &&
                proto->SubClass != ITEM_SUBCLASS_FOOD && proto->SubClass != ITEM_SUBCLASS_CONSUMABLE_OTHER && proto->SubClass != ITEM_SUBCLASS_ITEM_ENHANCEMENT)
                return true;

            for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
            {
                uint32 spellId = proto->Spells[i].SpellId;
                if (!spellId)
                    continue;

                if (bot->HasAura(spellId))
                    return true;

                Item* itemForSpell = *GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<Item*>("item for spell", spellId);
                if (itemForSpell && itemForSpell->IsInWorld() && itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                    return true;

                if (items.find(proto->SubClass) == items.end())
                    items[proto->SubClass] = std::vector<Item*>();

                items[proto->SubClass].push_back(item);
                break;
            }

            return true;
        }

        std::map<uint32, std::vector<Item*> > items;

    private:
        Player* bot;
};

void BuffAction::TellHeader(uint32 subClass)
{
    const char* ELIXIR_TEXT = "--- 药剂 ---";
    const char* FLASK_TEXT = "--- 瓶子 ---";
    const char* SCROLL_TEXT = "--- 卷轴 ---";
    const char* FOOD_TEXT = "--- 食物 ---";
    const char* ENHANCEMENT_TEXT = "--- 附魔 ---";
    const char* UNKNOWN_TEXT = "--- 未知 ---";

    switch (subClass)
    {
    case ITEM_SUBCLASS_ELIXIR:
        botAI->TellMaster(ELIXIR_TEXT);
        break;
    case ITEM_SUBCLASS_FLASK:
        botAI->TellMaster(FLASK_TEXT);
        break;
    case ITEM_SUBCLASS_SCROLL:
        botAI->TellMaster(SCROLL_TEXT);
        break;
    case ITEM_SUBCLASS_FOOD:
        botAI->TellMaster(FOOD_TEXT);
        break;
    case ITEM_SUBCLASS_ITEM_ENHANCEMENT:
        botAI->TellMaster(ENHANCEMENT_TEXT);
        break;
    default:
        botAI->TellMaster(UNKNOWN_TEXT);        // 处理未知的subClass
        break;
    }
}

bool BuffAction::Execute(Event event)
{
    std::string const text = event.getParam();

    FindBuffVisitor visitor(bot);
    IterateItems(&visitor);

    uint32 oldSubClass = -1;
    for (std::map<uint32, std::vector<Item*> >::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
    {
        std::vector<Item*> items = i->second;

        uint32 subClass = i->first;
        if (oldSubClass != subClass)
        {
            if (!items.empty())
                TellHeader(subClass);

            oldSubClass = subClass;
        }

        for (std::vector<Item*>::iterator j = items.begin(); j != items.end(); ++j)
        {
            Item* item = *j;
            std::ostringstream out;
            out << chat->FormatItem(item->GetTemplate(), item->GetCount());
            botAI->TellMaster(out);
        }
    }

    return true;
}
