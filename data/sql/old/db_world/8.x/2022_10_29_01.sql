-- DB update 2022_10_29_00 -> 2022_10_29_01
--
ALTER TABLE `areatrigger_tavern` ADD COLUMN `faction` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `name`;

SET @FACTION_ALLIANCE        := 2;
SET @FACTION_HORDE           := 4;
SET @FACTION_BOTH            := 6;

UPDATE `areatrigger_tavern` SET `faction` = @FACTION_ALLIANCE WHERE `id` IN (
71, 
562, 
682, 
707, 
708,
709,
710,
712,
713,
715,
716,
717,
1024,
1042,
2266,
2786,
4090,
4240,
4241,
4337,
4374,
4376,
4377,
4381,
4383,
4499,
4961,
4964,
4965,
4966,
4993,
4528,
4558,
4753,
4756,
4769,
4867,
5182);

UPDATE `areatrigger_tavern` SET `faction` = @FACTION_HORDE WHERE `id` IN (
178,
719, 
720, 
721, 
722, 
742, 
843, 
844, 
862, 
982, 
1022,
1025,
1606,
1646,
2267,
2286,
2610,
3547,
3690,
3886,
4108,
4109,
4265,
4336,
4373,
4375,
4378,
4380,
4486,
4494,
4526,
5323,
5360,
4967,
4970,
4979,
4868,
4910,
5045,
4861,
4775,
4755,
4595);

UPDATE `areatrigger_tavern` SET `faction` = @FACTION_BOTH WHERE `id` IN (
98,  
743, 
1023,
2287,
3985,
4058,
4300,
4382,
4498,
4521,
4555,
4577,
5183,
5200,
5204,
5217,
5227,
5314,
5315,
5316,
5317,
5327,
5062,
5164,
5030,
4975,
4976,
4977,
4607,
4608,
4640,
4714,
4847);
