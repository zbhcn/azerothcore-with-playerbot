/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InventoryChangeFailureAction.h"
#include "Event.h"
#include "Playerbots.h"

std::map<InventoryResult, std::string> InventoryChangeFailureAction::messages;

bool InventoryChangeFailureAction::Execute(Event event)
{
    if (!botAI->GetMaster())
        return false;

    if (messages.empty())
    {
        messages[EQUIP_ERR_CANT_EQUIP_LEVEL_I] = "我的等级太低了";
        messages[EQUIP_ERR_CANT_EQUIP_SKILL] = "我的技能等级太低了";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT] = "无效插槽";
        messages[EQUIP_ERR_BAG_FULL] = "我包包满了";
        messages[EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG] = "这个包包不是空的";
        messages[EQUIP_ERR_CANT_TRADE_EQUIP_BAGS] = "不能交易已装备的包包";
        messages[EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE] = "无效插槽(只需要弹药)";
        messages[EQUIP_ERR_NO_REQUIRED_PROFICIENCY] = "我没有必要的技能";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE] = "没有可用的装备插槽";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM] = "我从来不会用这个啊";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2] = "我从来不会用这个呀";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE2] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED] = "无法装备双手武器";
        messages[EQUIP_ERR_CANT_DUAL_WIELD] = "我不能双持";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG] = "这个包里装不下这个东西啊";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2] = "这个包里装不下这个东西呀";
        messages[EQUIP_ERR_CANT_CARRY_MORE_OF_THIS] = "我再也拿不了了";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE3] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_ITEM_CANT_STACK] = "物品无法堆叠";
        messages[EQUIP_ERR_ITEM_CANT_BE_EQUIPPED] = "物品无法装备";
        messages[EQUIP_ERR_ITEMS_CANT_BE_SWAPPED] = "无法交换这些装备";
        messages[EQUIP_ERR_SLOT_IS_EMPTY] = "没有任何装备";
        messages[EQUIP_ERR_ITEM_NOT_FOUND] = "找不到装备";
        messages[EQUIP_ERR_CANT_DROP_SOULBOUND] = "无法丢弃灵魂绑定的物品";
        messages[EQUIP_ERR_OUT_OF_RANGE] = "我超出了范围";
        messages[EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT] = "拆分编号无效";
        messages[EQUIP_ERR_COULDNT_SPLIT_ITEMS] = "无法拆分这个";
        messages[EQUIP_ERR_MISSING_REAGENT] = "缺少药剂";
        messages[EQUIP_ERR_NOT_ENOUGH_MONEY] = "钱不够";
        messages[EQUIP_ERR_NOT_A_BAG] = "这不是包包";
        messages[EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS] = "包包不是空的";
        messages[EQUIP_ERR_DONT_OWN_THAT_ITEM] = "这不是我的物品";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER] = "只能装备箭袋";
        messages[EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT] = "我必须购买背包空位";
        messages[EQUIP_ERR_TOO_FAR_AWAY_FROM_BANK] = "我离银行太远了";
        messages[EQUIP_ERR_ITEM_LOCKED] = "物品锁了";
        messages[EQUIP_ERR_YOU_ARE_STUNNED] = "我被晕了";
        messages[EQUIP_ERR_YOU_ARE_DEAD] = "我死了";
        messages[EQUIP_ERR_CANT_DO_RIGHT_NOW] = "我现在不能这么做";
        messages[EQUIP_ERR_INT_BAG_ERROR] = "内部错误";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_BOLT] = "只允许使用螺栓";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH] = "允许偷弹药";
        messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED] = "无法缠绕";
        messages[EQUIP_ERR_EQUIPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_WRAPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BOUND_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_UNIQUE_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BAGS_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_ALREADY_LOOTED] = "已经拾取";
        messages[EQUIP_ERR_INVENTORY_FULL] = "我的包包满了";
        messages[EQUIP_ERR_BANK_FULL] = "我的银行满了";
        messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT] = "商品已售罄";
        messages[EQUIP_ERR_BAG_FULL3] = messages[EQUIP_ERR_BANK_FULL];
        messages[EQUIP_ERR_ITEM_NOT_FOUND2] = messages[EQUIP_ERR_ITEM_NOT_FOUND];
        messages[EQUIP_ERR_ITEM_CANT_STACK2] = messages[EQUIP_ERR_ITEM_CANT_STACK];
        messages[EQUIP_ERR_BAG_FULL4] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_ITEM_SOLD_OUT] = messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT];
        messages[EQUIP_ERR_OBJECT_IS_BUSY] = "此对象正忙";
        messages[EQUIP_ERR_NOT_IN_COMBAT] = "我没进战斗";
        messages[EQUIP_ERR_NOT_WHILE_DISARMED] = "解除武装时无法执行";
        messages[EQUIP_ERR_BAG_FULL6] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_CANT_EQUIP_RANK] = "装备级别不够";
        messages[EQUIP_ERR_CANT_EQUIP_REPUTATION] = "声望不够";
        messages[EQUIP_ERR_TOO_MANY_SPECIAL_BAGS] = "特殊袋子太多";
        messages[EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW] = "现在不能捡";
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 err;
    p >> err;
    if (err == EQUIP_ERR_OK)
        return false;

    std::string const msg = messages[(InventoryResult)err];
    if (!msg.empty())
    {
        botAI->TellError(msg);
        return true;
    }

    return false;
}
