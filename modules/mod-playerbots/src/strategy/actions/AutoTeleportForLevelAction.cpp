#include "AutoTeleportForLevelAction.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"

bool AutoTeleportForLevelAction::Execute(Event event) {
    AutoUpgradeEquip();

    if (!sPlayerbotAIConfig->autoTeleportForLevel || !sRandomPlayerbotMgr->IsRandomBot(bot)) {
        return false;
    }
    if (botAI->HasRealPlayerMaster()) {
        return false;
    }
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
    return true;
}

void AutoTeleportForLevelAction::AutoUpgradeEquip() {
    if (!sPlayerbotAIConfig->autoUpgradeEquip || !sRandomPlayerbotMgr->IsRandomBot(bot)) {
        return;
    }
    //改成紫装
    PlayerbotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_EPIC);
    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel) {
        factory.InitEquipment(true);
    }
    factory.InitAmmo();
}