-- DB update 2022_03_21_00 -> 2022_03_21_01
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2022_03_21_00';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2022_03_21_00 2022_03_21_01 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1643077401563937639'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1643077401563937639');

UPDATE `gameobject_loot_template` SET `Chance`= 25 WHERE  `Entry`=9677 AND `Item` IN (10715, 10717, 10718, 10722);

--
-- END UPDATING QUERIES
--
UPDATE version_db_world SET date = '2022_03_21_01' WHERE sql_rev = '1643077401563937639';
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
