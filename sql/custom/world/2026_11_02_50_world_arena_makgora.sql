-- Arenas 2v2 / 3v3 on the Forever "Mak'gora Arena" (BattlemasterList 1169, map 2995 Hyjal Crater). Needs
-- hotfixes 2026_11_02_50_hotfixes_arena_makgora.sql.
-- The arena is the round crater east of Nordrassil: a flat floor about 200 yards across at height 1453.4 (navmesh
-- 2995_21_37/21_38/22_37: ground, height spread 0.7 yards), ringed by walls 50 to 120 yards high that have no walkable
-- slope, so nobody walks out. Centre about (5440, -2805). The two teams start 90 yards apart on the north-south line
-- through the centre, facing each other. StartMaxDist 15: during the one minute of preparation a player who walks
-- further than 15 yards from their start point is put back. Read from the 70707 navmesh, to be checked in game (.gps).
SET @ARENA_A_X := 5440, @ARENA_A_Y := -2850, @ARENA_A_Z := 1453.9, @ARENA_A_O := 1.5708;
SET @ARENA_H_X := 5440, @ARENA_H_Y := -2760, @ARENA_H_Z := 1453.9, @ARENA_H_O := 4.7124;

DELETE FROM `world_safe_locs` WHERE `ID` IN (1104110,1104111);
INSERT INTO `world_safe_locs` (`ID`,`MapID`,`LocX`,`LocY`,`LocZ`,`Facing`,`Comment`) VALUES
(1104110,2995,@ARENA_A_X,@ARENA_A_Y,@ARENA_A_Z,@ARENA_A_O,'Mak''gora Arena - team 1 start'),
(1104111,2995,@ARENA_H_X,@ARENA_H_Y,@ARENA_H_Z,@ARENA_H_O,'Mak''gora Arena - team 2 start');

-- 6 = the "All Arenas" queue (no start points of its own), 1169 = the arena it picks (the only one on map 2995).
DELETE FROM `battleground_template` WHERE `ID` IN (6,1169);
INSERT INTO `battleground_template` (`ID`,`AllianceStartLoc`,`HordeStartLoc`,`StartMaxDist`,`Weight`,`ScriptName`,`Comment`) VALUES
(6,0,0,0,1,'','All Arenas'),
(1169,1104110,1104111,15,1,'','Mak''gora Arena');
