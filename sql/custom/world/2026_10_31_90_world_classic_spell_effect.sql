-- Local: SpellMgr::LoadClassicSpellEffects (taken from Karrz 0.1.26.3) reads this table; their source does not ship it. Empty means no effect overrides.
CREATE TABLE IF NOT EXISTS `classic_spell_effect` (
  `spell_id` int unsigned NOT NULL,
  `effect_index` tinyint unsigned NOT NULL,
  `apply_aura` smallint unsigned NOT NULL DEFAULT 0,
  `misc_value` int NOT NULL DEFAULT 0,
  `class_mask0` int unsigned NOT NULL DEFAULT 0,
  `class_mask1` int unsigned NOT NULL DEFAULT 0,
  `class_mask2` int unsigned NOT NULL DEFAULT 0,
  `class_mask3` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`spell_id`,`effect_index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
