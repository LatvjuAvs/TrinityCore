-- Hire brokers (6110000-6110299): right-click opened nothing. The server sends a creature's NPC flags to each player through
-- Player::CanSeeGossipOn, which drops the gossip flag when the creature has no gossip menu in the database (creature_template_gossip
-- plus a gossip_menu row). The brokers had only their script menu, so every client got NPC flags 0: no talk cursor and no
-- CMSG_TALK_TO_GOSSIP (packet log, Goldshire, 2026-11-01 23:04). One shared menu, 6110000, with the broker's own text, npc_text 3959
-- ("Greetings, $r."). The script still builds the options (npc_fb_hire_broker, menu id 6110000). Safe to run again.

DELETE FROM `gossip_menu` WHERE `MenuID`=6110000;
INSERT INTO `gossip_menu` (`MenuID`,`TextID`,`VerifiedBuild`) VALUES
(6110000,3959,0);

DELETE FROM `creature_template_gossip` WHERE `CreatureID` BETWEEN 6110000 AND 6110299;
INSERT INTO `creature_template_gossip` (`CreatureID`,`MenuID`,`VerifiedBuild`) VALUES
(6110000,6110000,0),
(6110001,6110000,0),
(6110002,6110000,0),
(6110003,6110000,0),
(6110004,6110000,0),
(6110005,6110000,0),
(6110006,6110000,0),
(6110007,6110000,0),
(6110008,6110000,0),
(6110009,6110000,0),
(6110010,6110000,0),
(6110011,6110000,0),
(6110012,6110000,0),
(6110013,6110000,0),
(6110014,6110000,0),
(6110015,6110000,0),
(6110016,6110000,0),
(6110017,6110000,0),
(6110018,6110000,0),
(6110019,6110000,0),
(6110020,6110000,0),
(6110021,6110000,0),
(6110022,6110000,0),
(6110023,6110000,0),
(6110024,6110000,0),
(6110025,6110000,0),
(6110026,6110000,0),
(6110027,6110000,0),
(6110028,6110000,0),
(6110029,6110000,0),
(6110100,6110000,0),
(6110101,6110000,0),
(6110102,6110000,0),
(6110103,6110000,0),
(6110104,6110000,0),
(6110105,6110000,0),
(6110200,6110000,0),
(6110201,6110000,0),
(6110202,6110000,0);
