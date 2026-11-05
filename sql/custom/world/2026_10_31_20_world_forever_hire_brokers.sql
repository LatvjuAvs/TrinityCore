-- The hire brokers move to entries 6110000-6110299 (and spawn guids 61100000-61100099): the client reads a creature entry in 23 bits,
-- so an entry of 9110000 arrived as 721392 (9110000 - 8388608) and the client showed an empty "Level 1" unit that cannot be talked to.
-- The first lines remove the rows of the old numbers where an earlier test build placed them. Safe to run again.
DELETE FROM `creature` WHERE `id` BETWEEN 9110000 AND 9110299;
DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 9110000 AND 9110299;
DELETE FROM `creature_template_difficulty` WHERE `Entry` BETWEEN 9110000 AND 9110299;
DELETE FROM `creature_template` WHERE `entry` BETWEEN 9110000 AND 9110299;

-- Hired bots (Playerbot.Hire.*, src/modules/PlayerbotV2/Fleet/HireBroker.cpp): the hire brokers. Idea from Neylem (WoW Forever
-- Discord, 2026-10-29). Each stands beside the innkeeper of its town, 2.5 yards to the innkeeper's left and facing the same way,
-- placed from the innkeeper's own spawn in the 2026-10-14 world data; fine-tuned in game where a counter or wall is in the way.
--   6110000-6110099  Sellsword Captain    15 Alliance and 15 Horde inns: Sellswords at the hirer's level
--   6110100-6110199  Veteran Recruiter    the six capitals: Sellswords, and Veterans (level cap, raid gear)
--   6110200-6110299  Black Market Broker  Gadgetzan, Booty Bay, Ratchet: Legends (level cap, best in slot), both factions
-- Faction and level tuning are the innkeeper's; the models are existing Classic guard and mercenary models. Gossip only (npcflag 1),
-- the menu is the script's; the brokers do nothing while Playerbot.Hire.Mode is 0 except hire Veterans and Legends on request.

DELETE FROM `creature` WHERE `guid` BETWEEN 61100000 AND 61100099;
DELETE FROM `creature_template` WHERE `entry` BETWEEN 6110000 AND 6110299;
INSERT INTO `creature_template` (`entry`,`KillCredit1`,`KillCredit2`,`name`,`femaleName`,`subname`,`TitleAlt`,`IconName`,`RequiredExpansion`,`VignetteID`,`faction`,`npcflag`,`speed_walk`,`speed_run`,`scale`,`Classification`,`dmgschool`,`BaseAttackTime`,`RangeAttackTime`,`BaseVariance`,`RangeVariance`,`unit_class`,`unit_flags`,`unit_flags2`,`unit_flags3`,`family`,`trainer_class`,`type`,`VehicleId`,`AIName`,`MovementType`,`ExperienceModifier`,`RacialLeader`,`movementId`,`WidgetSetID`,`WidgetSetUnitConditionID`,`RegenHealth`,`CreatureImmunitiesId`,`flags_extra`,`ScriptName`,`StringId`,`VerifiedBuild`) VALUES
(6110000,0,0,'Brannock Ironhale','','Sellsword Captain',NULL,NULL,0,0,12,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Goldshire
(6110001,0,0,'Thordin Coppervein','','Sellsword Captain',NULL,NULL,0,0,55,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Kharanos
(6110002,0,0,'Hargan Stonefist','','Sellsword Captain',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Dolanaar
(6110003,0,0,'Durgan Flintbeard','','Sellsword Captain',NULL,NULL,0,0,12,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Lakeshire
(6110004,0,0,'Moran Gravelhand','','Sellsword Captain',NULL,NULL,0,0,12,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Darkshire
(6110005,0,0,'Kelgar Anvilmar','','Sellsword Captain',NULL,NULL,0,0,55,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Menethil Harbor
(6110006,0,0,'Bruni Hammerfell','','Sellsword Captain',NULL,NULL,0,0,12,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Southshore
(6110007,0,0,'Gorvik Deepforge','','Sellsword Captain',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Astranaar
(6110008,0,0,'Tharin Ironbrow','','Sellsword Captain',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Auberdine
(6110009,0,0,'Belgrom Rockshield','','Sellsword Captain',NULL,NULL,0,0,894,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Theramore
(6110010,0,0,'Rurik Stoutmantle','','Sellsword Captain',NULL,NULL,0,0,55,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Thelsamar
(6110011,0,0,'Dagna Emberforge','','Sellsword Captain',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Stonetalon Peak
(6110012,0,0,'Haldor Frostbeard','','Sellsword Captain',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Nijel's Point
(6110013,0,0,'Brakka Ironsong','','Sellsword Captain',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Feathermoon Stronghold
(6110014,0,0,'Fendrel Skyhammer','','Sellsword Captain',NULL,NULL,0,0,694,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Aerie Peak
(6110015,0,0,'Mak''zar Bloodfist','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Razor Hill
(6110016,0,0,'Torgal Ironhide','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Bloodhoof Village
(6110017,0,0,'Kragosh Deadeye','','Sellsword Captain',NULL,NULL,0,0,68,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Brill
(6110018,0,0,'Urzog Skullsplitter','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- The Crossroads
(6110019,0,0,'Gharn Blackmaw','','Sellsword Captain',NULL,NULL,0,0,68,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- The Sepulcher
(6110020,0,0,'Rokash Grimtusk','','Sellsword Captain',NULL,NULL,0,0,68,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Tarren Mill
(6110021,0,0,'Drogar Warblade','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Splintertree Post
(6110022,0,0,'Brakhar Stonejaw','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Hammerfall
(6110023,0,0,'Zugrak Bonecleaver','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Camp Taurajo
(6110024,0,0,'Nargash Swampfang','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Stonard
(6110025,0,0,'Kurzog Dustmane','','Sellsword Captain',NULL,NULL,0,0,104,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Freewind Post
(6110026,0,0,'Gorrak Stoneclaw','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Sun Rock Retreat
(6110027,0,0,'Murgol Ashfist','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Kargath
(6110028,0,0,'Vrakka Saltblade','','Sellsword Captain',NULL,NULL,0,0,104,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Shadowprey Village
(6110029,0,0,'Grushak Ironjaw','','Sellsword Captain',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Camp Mojache
(6110100,0,0,'Commander Ardrel Vaine','','Veteran Recruiter',NULL,NULL,0,0,12,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Stormwind
(6110101,0,0,'Commander Haldrun Forgeheart','','Veteran Recruiter',NULL,NULL,0,0,55,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Ironforge
(6110102,0,0,'Sentinel Commander Ilyara Moonblade','','Veteran Recruiter',NULL,NULL,0,0,80,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Darnassus
(6110103,0,0,'Blood Guard Gor''thak','','Veteran Recruiter',NULL,NULL,0,0,29,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Orgrimmar
(6110104,0,0,'Blood Guard Harnak Grimhoof','','Veteran Recruiter',NULL,NULL,0,0,104,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Thunder Bluff
(6110105,0,0,'Deathguard Commander Morvane','','Veteran Recruiter',NULL,NULL,0,0,68,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Undercity
(6110200,0,0,'Krixel Pocketlint','','Black Market Broker',NULL,NULL,0,0,474,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Gadgetzan
(6110201,0,0,'Zazzo Silkpurse','','Black Market Broker',NULL,NULL,0,0,120,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0), -- Booty Bay
(6110202,0,0,'Nixxle Quickcoin','','Black Market Broker',NULL,NULL,0,0,69,1,1,1.14286,1,0,0,2000,2000,1,1,1,0,0,0,0,0,7,0,'',0,1,0,0,0,0,1,0,2,'npc_fb_hire_broker',NULL,0); -- Ratchet

DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 6110000 AND 6110299;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`) VALUES
(6110000,0,1598,1,1,0),
(6110001,0,1598,1,1,0),
(6110002,0,1598,1,1,0),
(6110003,0,1598,1,1,0),
(6110004,0,1598,1,1,0),
(6110005,0,1598,1,1,0),
(6110006,0,1598,1,1,0),
(6110007,0,1598,1,1,0),
(6110008,0,1598,1,1,0),
(6110009,0,1598,1,1,0),
(6110010,0,1598,1,1,0),
(6110011,0,1598,1,1,0),
(6110012,0,1598,1,1,0),
(6110013,0,1598,1,1,0),
(6110014,0,1598,1,1,0),
(6110015,0,4259,1,1,0),
(6110016,0,4259,1,1,0),
(6110017,0,4259,1,1,0),
(6110018,0,4259,1,1,0),
(6110019,0,4259,1,1,0),
(6110020,0,4259,1,1,0),
(6110021,0,4259,1,1,0),
(6110022,0,4259,1,1,0),
(6110023,0,4259,1,1,0),
(6110024,0,4259,1,1,0),
(6110025,0,4259,1,1,0),
(6110026,0,4259,1,1,0),
(6110027,0,4259,1,1,0),
(6110028,0,4259,1,1,0),
(6110029,0,4259,1,1,0),
(6110100,0,3167,1,1,0),
(6110101,0,1598,1,1,0),
(6110102,0,2306,1,1,0),
(6110103,0,14360,1,1,0),
(6110104,0,2141,1,1,0),
(6110105,0,7117,1,1,0),
(6110200,0,7107,1,1,0),
(6110201,0,7107,1,1,0),
(6110202,0,7107,1,1,0);

DELETE FROM `creature_template_difficulty` WHERE `Entry` BETWEEN 6110000 AND 6110299;
INSERT INTO `creature_template_difficulty` (`Entry`,`DifficultyID`,`LevelScalingDeltaMin`,`LevelScalingDeltaMax`,`ContentTuningID`,`HealthScalingExpansion`,`HealthModifier`,`ManaModifier`,`ArmorModifier`,`DamageModifier`,`CreatureDifficultyID`,`TypeFlags`,`TypeFlags2`,`TypeFlags3`,`LootID`,`PickPocketLootID`,`SkinLootID`,`GoldMin`,`GoldMax`,`StaticFlags1`,`StaticFlags2`,`StaticFlags3`,`StaticFlags4`,`StaticFlags5`,`StaticFlags6`,`StaticFlags7`,`StaticFlags8`,`VerifiedBuild`) VALUES
(6110000,0,0,0,73,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110001,0,0,0,74,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110002,0,0,0,72,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110003,0,0,0,10,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110004,0,0,0,13,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110005,0,0,0,15,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110006,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110007,0,0,0,12,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110008,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110009,0,0,0,23,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110010,0,0,0,9,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110011,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110012,0,0,0,19,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110013,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110014,0,0,0,20,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110015,0,0,0,70,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110016,0,0,0,71,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110017,0,0,0,75,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110018,0,0,0,3,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110019,0,0,0,7,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110020,0,0,0,11,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110021,0,0,0,12,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110022,0,0,0,17,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110023,0,0,0,16,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110024,0,0,0,31,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110025,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110026,0,0,0,14,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110027,0,0,0,27,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110028,0,0,0,19,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110029,0,0,0,22,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110100,0,0,0,864,11,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110101,0,0,0,868,11,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110102,0,0,0,876,11,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110103,0,0,0,866,11,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110104,0,0,0,865,11,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110105,0,0,0,867,11,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110200,0,0,0,28,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110201,0,0,0,24,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
(6110202,0,0,0,3,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnDifficulties`,`phaseUseFlags`,`PhaseId`,`PhaseGroup`,`terrainSwapMap`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`wander_distance`,`currentwaypoint`,`curHealthPct`,`MovementType`,`npcflag`,`unit_flags`,`unit_flags2`,`unit_flags3`,`ScriptName`,`StringId`,`VerifiedBuild`) VALUES
(61100000,6110000,0,0,0,'0',0,0,0,-1,0,0,-9462.921,13.705,57.046,3.0369,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Goldshire, beside innkeeper 295
(61100001,6110001,0,0,0,'0',0,0,0,-1,0,0,-5603.720,-532.528,399.737,2.1293,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Kharanos, beside innkeeper 1247
(61100002,6110002,1,0,0,'0',0,0,0,-1,0,0,9804.700,982.826,1313.980,4.7997,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Dolanaar, beside innkeeper 6736
(61100003,6110003,0,0,0,'0',0,0,0,-1,0,0,-9224.198,-2159.610,64.017,3.0543,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Lakeshire, beside innkeeper 6727
(61100004,6110004,0,0,0,'0',0,0,0,-1,0,0,-10514.057,-1162.783,28.116,4.0317,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Darkshire, beside innkeeper 6790
(61100005,6110005,0,0,0,'0',0,0,0,-1,0,0,-3828.907,-829.600,10.091,0.4014,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Menethil Harbor, beside innkeeper 1464
(61100006,6110006,0,0,0,'0',0,0,0,-1,0,0,-859.596,-570.707,11.264,1.5533,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Southshore, beside innkeeper 2352
(61100007,6110007,1,0,0,'0',0,0,0,-1,0,0,2779.835,-435.117,116.665,2.5831,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Astranaar, beside innkeeper 6738
(61100008,6110008,1,0,0,'0',0,0,0,-1,0,0,6404.119,516.098,8.726,1.2741,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Auberdine, beside innkeeper 6737
(61100009,6110009,1,0,0,'0',0,0,0,-1,0,0,-3618.127,-4472.325,14.329,2.2166,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Theramore, beside innkeeper 6272
(61100010,6110010,0,0,0,'0',0,0,0,-1,0,0,-5380.355,-2974.430,323.252,1.7802,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Thelsamar, beside innkeeper 6734
(61100011,6110011,1,0,0,'0',0,0,0,-1,0,0,2731.850,1496.601,237.593,4.0492,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Stonetalon Peak, beside innkeeper 16458
(61100012,6110012,1,0,0,'0',0,0,0,-1,0,0,255.876,1251.274,192.224,3.2463,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Nijel's Point, beside innkeeper 11103
(61100013,6110013,1,0,0,'0',0,0,0,-1,0,0,-4381.677,3291.948,13.627,0.0349,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Feathermoon Stronghold, beside innkeeper 7736
(61100014,6110014,0,0,0,'0',0,0,0,-1,0,0,397.098,-2096.599,131.562,4.9305,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Aerie Peak, beside innkeeper 7744
(61100015,6110015,1,0,0,'0',0,0,0,-1,0,0,342.527,-4687.540,16.541,4.1888,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Razor Hill, beside innkeeper 6928
(61100016,6110016,1,0,0,'0',0,0,0,-1,0,0,-2364.313,-345.044,-8.874,5.8468,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Bloodhoof Village, beside innkeeper 6747
(61100017,6110017,0,0,0,'0',0,0,0,-1,0,0,2270.944,242.896,34.340,3.7525,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Brill, beside innkeeper 5688
(61100018,6110018,1,0,0,'0',0,0,0,-1,0,0,-406.392,-2647.611,96.306,3.4383,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- The Crossroads, beside innkeeper 3934
(61100019,6110019,0,0,0,'0',0,0,0,-1,0,0,513.217,1635.493,126.027,4.3110,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- The Sepulcher, beside innkeeper 6739
(61100020,6110020,0,0,0,'0',0,0,0,-1,0,0,-6.942,-944.585,57.162,2.7415,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Tarren Mill, beside innkeeper 2388
(61100021,6110021,1,0,0,'0',0,0,0,-1,0,0,2339.973,-2568.630,102.857,2.2864,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Splintertree Post, beside innkeeper 12196
(61100022,6110022,0,0,0,'0',0,0,0,-1,0,0,-912.374,-3527.420,72.768,3.1416,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Hammerfall, beside innkeeper 9501
(61100023,6110023,1,0,0,'0',0,0,0,-1,0,0,-2378.457,-1994.528,96.788,1.0647,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Camp Taurajo, beside innkeeper 7714
(61100024,6110024,0,0,0,'0',0,0,0,-1,0,0,-10487.561,-3256.354,21.113,0.1047,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Stonard, beside innkeeper 6930
(61100025,6110025,1,0,0,'0',0,0,0,-1,0,0,-5478.623,-2458.052,89.367,0.3142,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Freewind Post, beside innkeeper 11116
(61100026,6110026,1,0,0,'0',0,0,0,-1,0,0,895.014,930.044,106.364,5.7072,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Sun Rock Retreat, beside innkeeper 7731
(61100027,6110027,0,0,0,'0',0,0,0,-1,0,0,-6648.213,-2151.050,245.352,3.9968,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Kargath, beside innkeeper 9356
(61100028,6110028,1,0,0,'0',0,0,0,-1,0,0,-1590.675,3149.020,46.578,4.1888,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Shadowprey Village, beside innkeeper 11106
(61100029,6110029,1,0,0,'0',0,0,0,-1,0,0,-4461.312,244.909,39.191,0.5061,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Camp Mojache, beside innkeeper 7737
(61100030,6110100,0,0,0,'0',0,0,0,-1,0,0,-8865.583,674.847,97.986,5.2011,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Stormwind, beside innkeeper 6740
(61100031,6110101,0,0,0,'0',0,0,0,-1,0,0,-4838.201,-856.703,501.997,4.8695,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Ironforge, beside innkeeper 5111
(61100032,6110102,1,0,0,'0',0,0,0,-1,0,0,10125.903,2223.285,1328.810,2.2166,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Darnassus, beside innkeeper 6735
(61100033,6110103,1,0,0,'0',0,0,0,-1,0,0,1633.053,-4441.688,15.517,2.7576,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Orgrimmar, beside innkeeper 6929
(61100034,6110104,1,0,0,'0',0,0,0,-1,0,0,-1301.605,40.606,129.292,0.5585,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Thunder Bluff, beside innkeeper 6746
(61100035,6110105,0,0,0,'0',0,0,0,-1,0,0,1634.669,220.940,-43.020,2.8449,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Undercity, beside innkeeper 6741
(61100036,6110200,1,0,0,'0',0,0,0,-1,0,0,-7161.278,-3842.547,8.848,1.9548,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Gadgetzan, beside innkeeper 7733
(61100037,6110201,0,0,0,'0',0,0,0,-1,0,0,-14455.842,493.675,15.213,3.9794,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0), -- Booty Bay, beside innkeeper 6807
(61100038,6110202,1,0,0,'0',0,0,0,-1,0,0,-1049.351,-3662.397,23.968,6.0039,300,0,0,100,0,NULL,NULL,NULL,NULL,'',NULL,0); -- Ratchet, beside innkeeper 6791
