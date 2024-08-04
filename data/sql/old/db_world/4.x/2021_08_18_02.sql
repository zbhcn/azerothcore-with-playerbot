-- DB update 2021_08_18_01 -> 2021_08_18_02
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2021_08_18_01';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2021_08_18_01 2021_08_18_02 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1629117996105184400'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1629117996105184400');

DELETE FROM `quest_poi_points` WHERE `QuestID`=2518 AND `Idx1`=0 AND `Idx2`=0;
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild`) VALUES (2518, 0, 0, 10979, 1366, 0);

--
-- END UPDATING QUERIES
--
UPDATE version_db_world SET date = '2021_08_18_02' WHERE sql_rev = '1629117996105184400';
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
