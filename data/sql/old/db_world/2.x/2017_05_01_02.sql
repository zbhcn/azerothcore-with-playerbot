-- DB update 2017_05_01_01 -> 2017_05_01_02
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2017_05_01_01';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2017_05_01_01 2017_05_01_02 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1489592591354258700'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--
INSERT INTO version_db_world (`sql_rev`) VALUES ('1489592591354258700');
DELETE FROM `item_loot_template` WHERE `Entry` IN (50301,54218);
INSERT INTO `item_loot_template` (`Entry`,`Item`,`ChanceOrQuestChance`,`LootMode`,`GroupId`,`mincountOrRef`,`maxcount`) VALUES
(50301,46779,36.36,1,1,50,50),
(50301,35223,36.36,1,1,50,50),
(50301,45047,27.28,1,1,50,50),
(50301,32588,0.45,1,2,1,1),
(50301,38050,0.45,1,2,1,1),
(50301,34493,0.10,1,2,1,1),
(54218,46779,36.36,1,1,50,50),
(54218,35223,36.36,1,1,50,50),
(54218,45047,27.28,1,1,50,50),
(54218,23720,0.50,1,2,1,1),
(54218,49284,0.30,1,2,1,1),
(54218,49283,0.30,1,2,1,1),
(54218,49286,0.30,1,2,1,1),
(54218,49285,0.10,1,2,1,1),
(54218,49282,0.20,1,2,1,1);
--
-- END UPDATING QUERIES
--
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
