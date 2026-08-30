# NPC script translation

`Data/Info/NPCScript.inf` holds every line an NPC speaks — 1193 scripts, each with
one subject (what the NPC says) and any number of contents (the replies the player
picks from). It ships in CP949 Korean and is not part of this repo, so the English
text lives here instead and is applied over the loaded table at runtime, the same
arrangement `String.inf` has with `Client/MGameStringTable.cpp`.

## Files

| File | What |
|---|---|
| `npcscript_dump.pl` | reads `NPCScript.inf`, writes `npcscript.ko.tsv` |
| `npcscript.ko.tsv` | one row per slot: id, kind (`S`/`C`), index, owner, Korean |
| `npcscript.en.tsv` | the translation: Korean, English — **this is the file to edit** |
| `npcscript_gen.pl` | reads both, writes `Client/MNPCScriptTableEnglish.cpp` |

The translation is keyed by the Korean text rather than by position, so the 2646
slots collapse to the 2041 distinct lines that actually need translating, and a
slot the table has no entry for keeps whatever the data file supplied.

## Regenerating

```bash
perl tools/i18n/npcscript_dump.pl /path/to/Data/Info/NPCScript.inf tools/i18n/npcscript.ko.tsv
perl tools/i18n/npcscript_gen.pl tools/i18n/npcscript.ko.tsv tools/i18n/npcscript.en.tsv Client/MNPCScriptTableEnglish.cpp
```

`npcscript_gen.pl` reports how many slots it translated and how many distinct lines
are still Korean, which is the number to watch after the data file changes.

## Editing the translation

- Both columns carry the dump's escaping: `\\`, `\r`, `\n`, `\t`, and `\xNN` for any
  other control byte. Keep the two-character escapes rather than real control
  characters, or a string will run onto the next line and the file will not parse.
- Keep every `%(Name)` parameter — `%(MonsterName)`, `%(QuestZone)`, `%(UserName)`,
  `%(GuildName)`, `%(CastleName)` and the rest. The client substitutes them at
  display time; drop one and the text renders with a hole in it.
- English only, ASCII only. The client transcodes CP949 to UTF-8 when it renders,
  and non-ASCII here would be transcoded a second time.
- Keep subjects under 2048 bytes: `UIDialog` copies one into a buffer that size.

One shipped string does not decode as CP949 (script 19400's subject, which has a
stray byte in it). The dump keeps it as raw bytes and counts it in the
"undecodable" total; it still round-trips, because the key is compared byte for
byte.
