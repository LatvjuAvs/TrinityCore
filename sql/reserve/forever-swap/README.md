# Swapping class spells back to Forever

Most class spell numbers on the server come from VMaNGOS (vanilla 1.12): `classic_spell_value` (damage / heal / buff dice,
points per level and combo point, coefficients), `classic_spell_cost` (mana) and `classic_spell_info` (cast, cooldown,
duration, range). The swap removes those rows in batches, so the client's Forever values (plus our live hotfixes) are used.

Compared 2026-10-31: client 70685 DB2 + the live hotfix cache of build 70707 (the source wowsims/forever, MIT,
https://github.com/wowsims/forever, builds from). Full list: wowsims-crossref/class-spell-vanilla-holds-1031.tsv in the project
files. Class spells = spells on class skill lines, talents, and what they trigger: 1,552 value rows, 1,319 cost rows, 17 info rows.

| Batch | File | Rows | In game |
| --- | --- | --- | --- |
| 1 | sql/custom/world/2026_10_31_40_world_forever_class_spell_rows_unchanged.sql | 706 value, 1,281 cost, 7 info | nothing changes (rows equal to the client, or already skipped) |
| 2 | sql/custom/world/2026_10_31_41_world_forever_talent_values.sql | 108 talent value, 2 talent cost | talents use Forever's rank values (rule: talents follow Forever) |
| 3 | sql/custom/world/2026_11_03_40_world_forever_class_ability_values.sql | 487 ability value rows | abilities where Forever is within 30% of vanilla or stronger (rule: vanilla only when Forever is 30%+ lower) |
| hold | sql/reserve/full-forever/04 | 222 ability value rows | Forever 30%+ lower (Lightning Bolt, Holy Fire, Moonfire, Shadow Bolt...): stay vanilla until gear and balance are there |

Batch 3 (chris 2026-11-03: "swap over to forever now") was compared again with the live cache of 2026-11-03 01:10Z. Its push
113534 is Forever's launch hunter tuning (hotfixes 2026_11_03_40), so Aspect of the Hawk now equals vanilla and its rows go; Aimed
Shot is still 40% lower and stays vanilla. Rows whose value equals Forever but whose coefficient or per-level differs go too (Forever
design). Undo for batch 1, 2 and 3: 01_undo_batch1.sql, 02_undo_batch2.sql, 03_undo_batch3.sql here (world DB). After batch 1/2, full-forever/04_undo.sql
would also bring their rows back; run the batch file again after it.
