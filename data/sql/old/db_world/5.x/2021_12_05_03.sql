-- DB update 2021_12_05_02 -> 2021_12_05_03
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2021_12_05_02';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2021_12_05_02 2021_12_05_03 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1638658608'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1638658608');

/* Zul'Farrak Shallow Grave - On Gossip Hello - Despawn after 5min */
DELETE FROM `smart_scripts` WHERE `entryorguid` = 128308 AND `id` = 1 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(128308, 1, 1, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 41, 300000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Shallow Grave - On Gossip Hello - Despawn');

/* Make generically spawned traps invisible - They share displayId */
/* For example, fixes: ZF Grave traps, shellfish traps, resonite crystal traps and more..*/
UPDATE `gameobject_template` SET `data11` = 1 WHERE `displayid` = 327 AND `type` = 6;

--
-- END UPDATING QUERIES
--
UPDATE version_db_world SET date = '2021_12_05_03' WHERE sql_rev = '1638658608';
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
