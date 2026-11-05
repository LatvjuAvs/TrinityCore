-- Arenas 2v2 / 3v3: "All Arenas" (6) needs start points to load.
-- 0.1.29 core only skips the start point check for a battleground that lists more than one map. "All Arenas" lists one
-- map (2995, Mak'gora Arena), so with start points 0 it was dropped at start ("Loaded 5 battlegrounds") and every arena
-- join stopped with "template bg (all arenas) not found". Give it the Mak'gora Arena start points.
UPDATE `battleground_template` SET `AllianceStartLoc`=1104110, `HordeStartLoc`=1104111 WHERE `ID`=6;
