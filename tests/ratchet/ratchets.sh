#!/usr/bin/env bash
#----------------------------------------------------------------------
# ratchets.sh - shrink-only ratchets (docs/RESTRUCTURING.md, task 0.2)
#----------------------------------------------------------------------
#
# Each ratchet measures one axis of legacy debt. The baselines below are
# the committed truth: a measurement ABOVE its baseline fails (debt may
# not grow), and a measurement BELOW it also fails (progress must be
# recorded - tighten the baseline here AND in docs/RESTRUCTURING.md's
# ratchet table in the same commit as the change that earned it).
#
# Run from anywhere; the script cd's to the repo root. Optional first
# argument: a CMake build directory whose generated DarkEden.vcxproj is
# current (ctest passes its own binary dir) - used by R1.
#
# Never generates into the source tree; measurements are grep-only.
#
#----------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.." || exit 2

BUILD_DIR="${1:-}"
FAIL=0

check () {
	local name="$1" measured="$2" baseline="$3"
	# An unmeasurable metric is a failure, never a pass: with an empty
	# $measured both comparisons below error out as "false" and would
	# fall through to OK.
	if ! [ "$measured" -ge 0 ] 2>/dev/null; then
		echo "FAIL $name: could not measure (got '$measured')"
		FAIL=1
		return
	fi
	if [ "$measured" -gt "$baseline" ]; then
		echo "FAIL $name: measured $measured > baseline $baseline - this change grows debt the ratchet exists to shrink"
		FAIL=1
	elif [ "$measured" -lt "$baseline" ]; then
		echo "FAIL $name: measured $measured < baseline $baseline - progress! Record it: tighten the baseline in tests/ratchet/ratchets.sh AND docs/RESTRUCTURING.md in this same commit"
		FAIL=1
	else
		echo "OK   $name = $measured"
	fi
}

#----------------------------------------------------------------------
# R1 - translation units compiled directly into the DarkEden target.
#
# Counted from the generated DarkEden.vcxproj, which reflects the CMake
# source lists after configure. Preference order: the build dir ctest
# passed in, then the day-to-day tree. On generators that produce no
# .vcxproj the ratchet is skipped with a message - skipped, not passed.
#----------------------------------------------------------------------
# 515: 516 - 1. Task 5.2 deleted MitemTableInit.cpp, the server's in-code
# item data (dead here). History: 516 = 517 - 1 (task 4.4's first slice
# moved MItemTable.cpp into gamemodel).
# History: 517 = 518 - 1 (task 4.2 moved MMoneyManager.cpp into gamemodel,
# another of the double-compiled VS_UI_CLIENT_SOURCES entries).
# 518 = 528 - 10. Task 4.1's gamemodel library took its ten members out
# of the executable: ExpInfo.cpp, MString.cpp, MStringArray.cpp and
# DebugLog.cpp, and the six tables that were nominally VS_UI's - the
# relative VS_UI_CLIENT_SOURCES list never matched the exe glob's
# absolute paths in REMOVE_ITEM, so they compiled into both (the
# LNK4217 trap); the membership removal is absolute and asserted.
# History: 493 = 492 + 1 (task 5.3: TextServiceScreen.cpp is a new exe TU.
# A recorded GROWTH, the same trade as 4.4's MItemUse.cpp and
# MSkillAvailable.cpp: TextService::RenderText draws through g_pLast, the
# executable's back buffer, and was the only thing in the TextSystem
# library reaching a client global - so a test binary had to define that
# global to link the library at all. The definition moved to the
# executable side and tests/stubs/ is deleted. One more TU here buys a
# library that links on its own.)
# History: 492 = 493 - 1 (task 5.1's second slice: ClientCommunicationManager.cpp
# moved into packetwire once the three tuning values it read from the
# executable's config went behind WireHost).
# History: 493 = 495 - 2 (task 5.1: Player.cpp and DatagramSocket.cpp
# moved into packetwire once the debug facilities stopped gating the
# wire layer's include rules - the logging header is in basic/ now).
# History: 495 = 497 - 3 + 1 (task 4.4's fourth slice: MSkillManager.cpp,
# MSkillInfoTable.cpp and SkillDef.cpp moved into gamemodel; the
# player-facing half split out of the first, MSkillAvailable.cpp, is a
# new exe TU).
# History: 497 = 502 - 5 (task 4.4's third slice: MPlayerGear.cpp,
# MSlayerGear.cpp, MVampireGear.cpp, MOustersGear.cpp and MShop.cpp
# moved into gamemodel).
# History: 502 = 503 - 1 (task 4.2's third slice: MPriceManager.cpp moved
# into gamemodel).
# History: 503 = 505 - 2 (task 4.2's second slice: MTradeManager.cpp and
# MSortedItemManager.cpp moved into gamemodel).
# History: 505 = 508 - 3 (task 4.3's second slice: MInventory.cpp,
# MStorage.cpp and MShopShelf.cpp moved into gamemodel).
# History: 508 = 512 - 4 (task 4.3's first slice: MItemManager.cpp,
# MGridItemManager.cpp, MSlotItemManager.cpp and MQuickSlot.cpp moved
# into gamemodel).
# History: 512 = 515 + 2 - 5 (task 4.4's second slice: MItem.cpp,
# MObject.cpp, UserInformation.cpp, ClientConfig.cpp and
# MTimeItemManager.cpp moved into gamemodel; the executable halves split
# out of the first two, MItemUse.cpp and MObjectScreen.cpp, are new exe
# TUs). 515 = 516 - 1 (task 5.2 deleted the dead MitemTableInit.cpp).
# 516 = 517 - 1 (4.4's first slice moved MItemTable.cpp). 517 = 518 - 1
# (4.2 moved MMoneyManager.cpp). 518 = 528 - 10 (4.1's gamemodel).
# History: 528 = 529 - 1 (task 2.5 deleted CRRequest2, a dead duplicate
# of CRRequest claiming the same packet id, with its handler
# Client/PacketHandler/CRRequest2Handler.cpp). 529 = 992 - 463. Task 2.4 moved every packet class, the
# factory/validator tables and the last held-back info classes (465
# .cpp files, tests/arch/packetwire_files.txt) into packetwire; the exe
# gained the split-out GCExchangeBuyHandler.cpp (+1) and the five root
# info classes that moved under Client/Packet were already counted
# before (0 net). 992 = 993 - 1 (task 2.2's PacketHandlerRegistry.cpp,
# a recorded +1, offset by the finished migration deleting
# CGHandlersStub.cpp).
R1_BASELINE=493

R1_VCXPROJ=""
for candidate in "$BUILD_DIR/DarkEden.vcxproj" "build/vs2022/DarkEden.vcxproj"; do
	if [ -n "$candidate" ] && [ -f "$candidate" ]; then
		R1_VCXPROJ="$candidate"
		break
	fi
done

if [ -n "$R1_VCXPROJ" ]; then
	# A tree configured before the source lists were last edited
	# measures a different tree (parallel sessions share this checkout);
	# that is a failure to measure, not a pass. The configure time is
	# CMakeFiles/generate.stamp - the Visual Studio generator touches it
	# on every generate, while it leaves an unchanged .vcxproj (and an
	# unchanged CMakeCache.txt) alone, so those mtimes would report a
	# fresh reconfigure as stale.
	R1_CACHE="$(dirname "$R1_VCXPROJ")/CMakeFiles/generate.stamp"
	if [ ! -f "$R1_CACHE" ] || [ CMakeLists.txt -nt "$R1_CACHE" ] || [ tests/arch/packetwire_files.txt -nt "$R1_CACHE" ]; then
		echo "FAIL R1: $(dirname "$R1_VCXPROJ") was configured before CMakeLists.txt or the packetwire membership file last changed - reconfigure that tree first"
		FAIL=1
	else
		R1=$(grep -c "<ClCompile Include" "$R1_VCXPROJ")
		check "R1 (TUs in DarkEden.vcxproj: $R1_VCXPROJ)" "$R1" "$R1_BASELINE"
	fi
else
	echo "SKIP R1: no generated DarkEden.vcxproj found (non-MSVC generator?)"
fi

#----------------------------------------------------------------------
# R2 - packet .cpp files still defining a packet-style ::execute
# (first parameter Player*). The client twin of the server's R4; task
# 2.2 drove it to 0 and this ratchet holds it there. The regex was
# refined when it reached 0: the original '::execute\s*\(' also
# matched comments and handler bodies defined inside packet files
# (GCExchangeBuy), which are not the debt being measured. Anchored
# 'void X::execute(Player' matches exactly the legacy virtual.
#----------------------------------------------------------------------
R2_BASELINE=0

# A zero baseline is fail-open if the measured tree silently is not
# there (a directory rename would make grep count 0 of nothing and
# PASS), so the directories are asserted first.
for d in Client/Packet/Gpackets Client/Packet/Cpackets Client/Packet/Lpackets \
	Client/Packet/Rpackets Client/Packet/Upackets; do
	if [ ! -d "$d" ]; then
		echo "FAIL R2: directory $d is missing - fix the path list in this script"
		FAIL=1
	fi
done

R2=$(grep -rlE '^void\s+\w+::execute\s*\(\s*Player' \
	Client/Packet/Gpackets Client/Packet/Cpackets Client/Packet/Lpackets \
	Client/Packet/Rpackets Client/Packet/Upackets \
	--include='*.cpp' 2>/dev/null | grep -v Handler | wc -l)
check "R2 (packet cpps defining execute)" "$R2" "$R2_BASELINE"

#----------------------------------------------------------------------
# R3 - live sprintf/strcpy/strcat lines in the packet tree: the wire
# classes under Client/Packet AND the handlers under Client/PacketHandler
# (task 2.4 moved the handlers out; they are where most of the strcpy
# targets fed by server strings live, so the metric follows them).
# (snprintf does not match: \b rejects the preceding 'n'. The // comment
# tail of every line is stripped BEFORE matching, so commented-out code
# does not count - a quarter of the first baseline was - while a live
# call with a trailing comment still does; the earlier whole-line
# exclusion let `sprintf(...); // was strcpy` through.)
#
# 18: 19 - 1. The review round of task 5.4's first slice found a
# pre-existing overflow one line below a site that slice had converted:
# GCBloodBibleListHandler sprintf'd "%3d %s" into char[192] from a
# char[192], four bytes short. Bounding it converts a 28th sprintf line,
# even though its format is a literal and it was never a C19 site - which
# is the difference between R3 and R7 in one example.
# History: 19 = 46 - 27. Task 5.4's first slice converted all 31
# sprintf-family calls in Client/PacketHandler whose format came from the
# game string table to SafeFormat::Format. Only 27 of them moved this
# number: the other four are wsprintf, which \b rejects because of the
# preceding 'w'. R7 below counts all 31, which is why both exist - each
# is blind to something the other sees.
#----------------------------------------------------------------------
R3_BASELINE=18

for d in Client/Packet Client/PacketHandler; do
	if [ ! -d "$d" ]; then
		echo "FAIL R3: directory $d is missing - fix the path list in this script"
		FAIL=1
	fi
done

R3=$(grep -rhE '\b(sprintf|strcpy|strcat)\s*\(' Client/Packet Client/PacketHandler --include='*.cpp' \
	| sed -e 's://.*::' | grep -cE '\b(sprintf|strcpy|strcat)\s*\(')
check "R3 (unsafe format/copy lines in Client/Packet + Client/PacketHandler)" "$R3" "$R3_BASELINE"

#----------------------------------------------------------------------
# R4 - library-compiled .cpp files referencing g_p* client globals.
#
# Membership: every .cpp under the whole-directory library trees, plus
# the packetwire and gamemodel membership files (tests/arch/
# packetwire_files.txt, tests/arch/gamemodel_files.txt - the CMake
# targets and the include checker read the same files). Extraction work
# (Phase 4) shrinks this by cutting the global seams. The pattern is the
# g_p prefix: the two clocks the item host carries (g_CurrentFrame,
# g_CurrentTime) were never in this count, so cutting them did not move
# it.
#
# 25: 27 - 2, a reclassification (task 4.4's fourth slice moved the skill
# core into gamemodel): VS_UI_SKILL_VIEW.cpp and VS_UI_skill_tree.cpp
# reached past the libraries only for g_pSkillInfoTable and
# g_pSkillManager, which MSkillManager.cpp defines. Note what this
# ratchet cannot see: it matches g_p* only, so a library file calling an
# executable-side function - VS_UI_GameCommon.cpp calls
# g_pSkillAvailable->SetAvailableSkills(), which lives in the
# executable's MSkillAvailable.cpp - is a seam the number does not
# count, before or after.
# 27: 28 - 1, a reclassification (task 4.4's third slice moved the gear
# into gamemodel): VS_UI_Game.cpp's only reaches past the libraries were
# g_pSlayerGear, g_pVampireGear and g_pOustersGear, which the gear
# sources define.
# 28: 35 + 1 - 8, again a reclassification, by the library-wide
# definition rule below (task 4.4's second slice). MItem.cpp joined the
# library reading gamemodel's own tables (+1 under the old per-file
# rule); the union rule then excludes it and seven earlier members:
# Datagram.cpp (g_pPacketFactoryManager, packetwire's own) and six
# VS_UI sources (AcceleratorDef, VS_UI_ELEVATOR, VS_UI_Item,
# VS_UI_Message, VS_UI_Shop, VS_UI_progress) whose only reaches are
# gamemodel's tables (g_pItemTable, g_pItemOptionTable,
# g_pGameStringTable, g_pUserInformation), packetwire's g_pFileDef, or
# their own library's g_pKeyAccelerator and g_pSystemAvailableManager.
# 35: 59 - 24, a RECLASSIFICATION, not progress on the seams: the 36
# Client/*.cpp files VS_UI_CLIENT_SOURCES listed were compiled into
# both VS_UI.lib and the executable, and the executable's objects were
# the ones that linked; the list is gone and they compile once, into
# the executable, so the 24 of them that reach g_p globals are
# executable debt now, outside this ratchet (R1 already counts them).
# History: 21 = 25 - 4 (task 5.3). Partly seam-cutting and partly a
# refined measurement, and it is worth being clear which. The real cut
# is TextSystem: TextService.cpp lost its live g_pLast reach when
# RenderText moved to the executable. The other three never had one -
# they only NAMED a global in a comment, and R4 counted that, unlike R3
# and R5 which have always stripped comment lines. Dropped:
# Client/TextSystem/TextService.cpp (which after the move would have
# counted for the comment 5.3 wrote ABOUT the seam it had just cut -
# that is how the flaw surfaced), Client/Packet/SocketOutputStream.cpp
# (a commented-out g_pLogManager call), Client/SpriteLib/
# SpriteLibBackendSDL.cpp and VS_UI/src/VS_UI_WebBrowser.cpp. The 21
# that remain are real reaches, and every one is a VS_UI file.
# History: 59 = 61 - 2. Task 4.1 cut the two g_pFileDef seams in MGameStringTable
# (UseEnglishText takes the Properties table) and SystemAvailabilities
# (LoadFromStream; the executable reads the archive); the four support
# sources gamemodel added (ExpInfo, MString, MStringArray, DebugLog)
# reference no game global - DebugLog.cpp's dead #if 0 block that named
# g_pDebugMessage was deleted rather than counted.
# History: 61 = 81 - 20. Task 2.4 grew the membership from 52 to 517 files, and
# the count still went DOWN because the measurement was refined with it
# (see the R4 loop): a file referencing only globals it defines itself
# no longer counts. The two dead server-only blocks that referenced
# game globals (GCSelectQuestID's PlayerCreature constructor,
# GCStashList::setStashItem from a live Item*) were deleted rather than
# grandfathered.
#----------------------------------------------------------------------
R4_BASELINE=21

lib_members () {
	# The directory trees minus the files CMake excludes from the
	# library builds (VS_UI/WinMain.cpp under WIN32; the hangul Ci/FL2
	# pair under USE_SDL_BACKEND, which is forced ON) - the original
	# raw find counted two never-compiled g_p-referencing files (WinMain, Ci) as library debt; FL2 is excluded for consistency though it references none.
	find basic Client/SpriteLib Client/DXLib Client/framelib Client/TextSystem VS_UI \
		-name '*.cpp' 2>/dev/null \
		| grep -vE 'VS_UI/WinMain\.cpp$|VS_UI/src/hangul/(Ci|FL2)\.cpp$'
	sed -e 's/#.*//' tests/arch/packetwire_files.txt \
		| grep -oE 'Client/Packet/[A-Za-z0-9_/]+\.cpp'
	sed -e 's/#.*//' tests/arch/gamemodel_files.txt \
		| grep -oE 'Client/[A-Za-z0-9_/]+\.cpp'
}

# A global DEFINED by a library file is not a seam into the executable:
# the reference resolves inside the libraries. The packet tables own
# g_pPacketFactoryManager / g_pPacketValidator (task 2.4); since task
# 4.4's item core, MItem.cpp reads g_pItemTable, g_pItemOptionTable,
# g_pGameStringTable, g_pUserInformation, g_pClientConfig and
# g_pTimeItemManager, every one defined by another gamemodel member -
# so the subtraction is against the union of every library file's
# definitions, not the file's own. (Until 4.4 it was per file, which
# would have counted the item core as debt for reading a table that
# sits beside it in the same library.)
lib_defs () {
	lib_members | sort -u | while read -r f; do
		[ -f "$f" ] || continue
		grep -oE '^[A-Za-z_][A-Za-z0-9_:<>]*[[:space:]]*\*?[[:space:]]*g_p[A-Z]\w*[[:space:]]*(=|;)' "$f" \
			| grep -oE '\bg_p[A-Z]\w*'
	done | sort -u
}
R4=$(defs=$(lib_defs); lib_members | sort -u | while read -r f; do
	[ -f "$f" ] || continue
	# Comment lines are not references. R3 and R5 have always filtered
	# them; R4 did not, so a file that only NAMED a global counted as
	# reaching it - which is worse than noise, because it can hide a
	# real tightening behind a sentence somebody wrote about the seam
	# they had just cut. Task 5.1's second slice worked around it by
	# rewording a comment; task 5.3 hit it again and fixed the
	# measurement instead. Line-based, so the same blindness R5
	# documents applies: a reference inside a /* */ block still counts.
	refs=$(grep -vE '^[[:space:]]*(//|\*|/\*)' "$f" \
		| grep -oE '\bg_p[A-Z]\w*' | sort -u)
	[ -n "$refs" ] || continue
	if [ -n "$(comm -23 <(echo "$refs") <(echo "$defs"))" ]; then
		echo "$f"
	fi
done | wc -l)
check "R4 (library cpps referencing g_p globals)" "$R4" "$R4_BASELINE"

#----------------------------------------------------------------------
# R5 - direct packet execute() call sites outside Client/Packet.
#
# The receive loops and PacketDispatcher are the only sanctioned
# callers; the adversarial review of task 2.2 found the client also
# fabricates packets locally and executed them directly (skill echoes,
# GM system messages, the login-path CGConnectSetKey), which the R2
# migration would have silently broken. Those sites now go through
# PacketDispatcher::dispatch. The baseline of 1 is the commented-out
# PacketAttackMelee block in CGameUpdate.cpp (~line 5867), which lives
# inside a /* */ block this line-based grep cannot see; it leaves with
# that dead block's deletion. Client/PacketHandler (where task 2.4 moved
# the handlers) is IN scope: two handlers turned out to fabricate a
# CGConnectSetKey and execute() it directly - invisible while they lived
# under the excluded Client/Packet, caught by the compiler when
# Packet::execute was deleted, and now routed through the dispatcher.
# With the virtual gone a new direct caller is a compile error first.
#----------------------------------------------------------------------
R5_BASELINE=1

R5=$(grep -rnE '(\.|->)execute\s*\(\s*(g_pSocket|NULL|this|0)\s*\)' \
	Client VS_UI --include='*.cpp' 2>/dev/null \
	| grep -v 'Client/Packet/' | grep -vE ':\s*//' | wc -l)
check "R5 (direct packet execute callers outside Client/Packet)" "$R5" "$R5_BASELINE"

#----------------------------------------------------------------------
# R7 - call sites that hand a game string table entry to printf as its
# format argument.
#
# Finding C19 (docs/code-health-review-2026-08-29.md) as a number. Every
# UI string in this client is read from Data/Info/String.inf, and at
# each of these sites it is the *format* rather than an argument: an
# entry carrying one %s more than the call site passes makes the CRT
# read a stack word as a char* and copy it, unbounded, into a
# destination of fifty or a hundred bytes. What converts a site is
# SafeFormat::Format (basic/SafeFormat.h), which checks the entry's
# conversions against the arguments it was really handed, so this counts
# what is left to do (task 5.4).
#
# The AddFormat family is in scope even though CMessageArray bounds its
# own buffer (finding C20, fixed in 0e9d247): what is left there is the
# arity half of the same defect.
#
# Two measurement decisions, each of them earned while this was written:
#
#   -a   Defensive, and NOT the reason it is here. VS_UI's sources are
#        CP949 encoded, and grep calls such a file binary and stops at
#        the first byte it dislikes - which is how the exploratory
#        per-file `grep -r` used to survey this population reported 113
#        of VS_UI's real 192. It does not bite the pipeline below,
#        where everything arrives on one stdin stream that grep does not
#        classify per file: measured, this pattern gives 192 with the
#        flag and 192 without. The review round of task 5.4's first
#        slice caught the comment claiming otherwise. The flag stays,
#        because a future per-file variant would need it and nothing
#        signals when binary detection truncates a scan.
#   tr   The files are joined into one stream before matching, because
#        four of Client/PacketHandler's sites put the destination and
#        the format on different lines. A line-based count cannot see
#        those, so converting them would have measured as a no-op -
#        which is the failure mode task 5.3 found in R4 and fixed there.
#        The gap this pattern allows between the destination and the
#        format admits no quote, paren or semicolon, which is what stops
#        a joined stream from matching across two unrelated statements;
#        counted per file and joined, the two agree exactly.
#
# It stays a count of sites rather than of files, because a file here is
# converted a call at a time and half a file is real progress.
#----------------------------------------------------------------------
R7_BASELINE=257

for d in Client VS_UI; do
	if [ ! -d "$d" ]; then
		echo "FAIL R7: directory $d is missing - fix the path list in this script"
		FAIL=1
	fi
done

# The format is at a different argument position in each of the three
# families, which is why this is three alternatives and not one. The
# first draft had only the first and the third, and so could not match a
# counted call in ANY form - it listed _snprintf and swprintf in the
# alternation while requiring the format at argument two, where those
# take a size. That was not academic: it missed a live site
# (vs_ui_gamecommon2.cpp, the guild quest mission line), and it left the
# door open, since a newly written snprintf(dst, sizeof dst,
# GetGameString(...), ...) could not have raised the count.
GAMESTRING='(\(\*g_pGameStringTable\)\[|GetGameString[[:space:]]*\()'

# sprintf family: format at argument 2.
R7_PATTERN="\\b(sprintf|wsprintf|swprintf|vsprintf)[[:space:]]*\\([[:space:]]*[^,;()\"]+,[[:space:]]*$GAMESTRING"

# counted family: format at argument 3, argument 2 being a size, which is
# usually sizeof(dst) and so may contain parentheses - bounded instead by
# refusing a semicolon or a quote and by a length cap.
R7_PATTERN="$R7_PATTERN|\\b(_?snprintf|_?vsnprintf|_?swprintf)[[:space:]]*\\([[:space:]]*[^,;()\"]+,[^;\"]{0,60},[[:space:]]*$GAMESTRING"

# message array family: format at argument 1.
R7_PATTERN="$R7_PATTERN|\\bAddFormat(VL)?[[:space:]]*\\([[:space:]]*$GAMESTRING"

R7=$(find Client VS_UI -name '*.cpp' -print0 2>/dev/null | xargs -0 cat 2>/dev/null \
	| sed -e 's://.*::' | tr '\n' ' ' \
	| grep -aoE "$R7_PATTERN" \
	| wc -l)
check "R7 (data-file format strings passed to printf)" "$R7" "$R7_BASELINE"

#----------------------------------------------------------------------
# R6 was here for exactly one slice, and retired by doing its job.
#
# Task 5.1 stubbed SendBugReport in tests/stubs/client_globals.cpp so
# the test binary would link, which disarmed the only thing that can
# see a library file calling an executable-side function - W1/W2 read
# includes, R4 greps g_p* globals, and neither sees a call. R6 counted
# packetwire members calling it, and the next slice tripped it at once:
# promoting ClientCommunicationManager.cpp took the count to 2. The
# answer was to move SendBugReport itself into the wire layer
# (Client/Packet/WireHost.cpp), so packetwire defines it, the stub is
# gone. The failed-link detector is back, narrower than this check
# was, since it catches a call only in a library unit_tests links and
# only in an object some test pulls in, which is what the link-proof
# tests are for. A ratchet over a symbol nothing is on the wrong side
# of measures nothing.
#----------------------------------------------------------------------

#----------------------------------------------------------------------

if [ "$FAIL" -ne 0 ]; then
	echo "ratchets: FAILED"
	exit 1
fi
echo "ratchets: all green"
exit 0
