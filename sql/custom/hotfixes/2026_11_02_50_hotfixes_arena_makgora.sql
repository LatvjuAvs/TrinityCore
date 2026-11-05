-- Arenas 2v2 / 3v3 on the Forever "Mak'gora Arena" (BattlemasterList 1169, map 2995 Hyjal Crater).
-- The Classic 1.60 client has no TBC arena maps (559, 562, 572, 617 are not in its Map.db2) and no BattlemasterList row 6
-- ("All Arenas", the queue every arena join uses). It does have BattlemasterList 1169 "Mak'gora Arena" (PvpType 1 = arena,
-- levels 1-60, up to 5 a side) tied to map 2995 by BattlemasterListXMap 1208.
-- Server side only (no hotfix_data): the client keeps its own rows. Rows checked against the 70707 client files.

-- Map 2995: the client row with InstanceType 4 (arena) and MaxPlayers 10, so the server makes an arena instance of it
-- instead of loading it as a world map. All other values are the client's own.
DELETE FROM `map` WHERE `ID` = 2995;
INSERT INTO `map` (`ID`,`Directory`,`MapName`,`MapDescription0`,`MapDescription1`,`PvpShortDescription`,`PvpLongDescription`,
  `CorpseX`,`CorpseY`,`MapType`,`InstanceType`,`ExpansionID`,`AreaTableID`,`LoadingScreenID`,`TimeOfDayOverride`,`ParentMapID`,
  `CosmeticParentMapID`,`TimeOffset`,`MinimapIconScale`,`CorpseMapID`,`MaxPlayers`,`WindSettingsID`,`ZmpFileDataID`,`WdtFileDataID`,
  `OceanLiquidTypeID`,`NavigationMaxDistance`,`PreloadFileDataID`,`Flags1`,`Flags2`,`Flags3`,`VerifiedBuild`) VALUES
(2995,'2995','Hyjal Crater','','','','',0,0,1,4,0,0,0,900,-1,-1,0,1,-1,10,136,0,7245741,2,0,0,8396381,1040,2281568,0);

-- BattlemasterList 6 "All Arenas": the arena queue (2v2, 3v3) every arena join goes through. PvpType 1 = arena.
DELETE FROM `battlemaster_list` WHERE `ID` = 6;
INSERT INTO `battlemaster_list` (`ID`,`Name`,`GameType`,`ShortDescription`,`LongDescription`,`InstanceType`,`PvpType`,`MinLevel`,`MaxLevel`,
  `RatedPlayers`,`MinPlayers`,`MaxPlayers`,`GroupsAllowed`,`MaxGroupSize`,`HolidayWorldState`,`Flags`,`IconFileDataID`,`RequiredPlayerConditionID`,`VerifiedBuild`) VALUES
(6,'All Arenas','','','',4,1,10,60,5,2,5,1,5,0,0,0,0,0);

-- The "All Arenas" queue plays on map 2995 (client row 1208 already ties 1169 to 2995). 1500: above the client's highest id 1208.
DELETE FROM `battlemaster_list_x_map` WHERE `ID` = 1500;
INSERT INTO `battlemaster_list_x_map` (`ID`,`MapID`,`BattlemasterListID`,`VerifiedBuild`) VALUES
(1500,2995,6,0);

-- Level brackets for map 2995, the same as Warsong Gulch: 10-19, 20-29, 30-39, 40-49, 50-59, 60.
-- (TBC style "level 60 only": keep only row 1905 and change its RangeIndex to 0.)
DELETE FROM `pvp_difficulty` WHERE `ID` BETWEEN 1900 AND 1905;
INSERT INTO `pvp_difficulty` (`ID`,`RangeIndex`,`MinLevel`,`MaxLevel`,`MapID`,`VerifiedBuild`) VALUES
(1900,0,10,19,2995,0),
(1901,1,20,29,2995,0),
(1902,2,30,39,2995,0),
(1903,3,40,49,2995,0),
(1904,4,50,59,2995,0),
(1905,5,60,60,2995,0);
