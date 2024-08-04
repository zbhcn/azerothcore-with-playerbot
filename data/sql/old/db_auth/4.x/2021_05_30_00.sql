-- DB update 2021_05_26_00 -> 2021_05_30_00
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_auth' AND COLUMN_NAME = '2021_05_26_00';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_auth CHANGE COLUMN 2021_05_26_00 2021_05_30_00 bit;
SELECT sql_rev INTO OK FROM version_db_auth WHERE sql_rev = '1621715473238990700'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_auth` (`sql_rev`) VALUES ('1621715473238990700');

ALTER TABLE `version_db_auth`
	ADD COLUMN `date` VARCHAR(50) NULL DEFAULT NULL COLLATE 'utf8mb4_general_ci' AFTER `required_rev`;

--
-- END UPDATING QUERIES
--
UPDATE version_db_auth SET date = '2021_05_30_00' WHERE sql_rev = '1621715473238990700';
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
