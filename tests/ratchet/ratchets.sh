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
# 992: 993 - 1. Task 2.2's registry (PacketHandlerRegistry.cpp, a
# recorded +1: composition-root wiring, not legacy debt) is offset by
# the finished migration deleting CGHandlersStub.cpp, whose no-op CG
# handler stubs existed only to satisfy the now-deleted execute()
# bodies.
R1_BASELINE=992

R1_VCXPROJ=""
for candidate in "$BUILD_DIR/DarkEden.vcxproj" "build/vs2022/DarkEden.vcxproj"; do
	if [ -n "$candidate" ] && [ -f "$candidate" ]; then
		R1_VCXPROJ="$candidate"
		break
	fi
done

if [ -n "$R1_VCXPROJ" ]; then
	R1=$(grep -c "<ClCompile Include" "$R1_VCXPROJ")
	check "R1 (TUs in DarkEden.vcxproj: $R1_VCXPROJ)" "$R1" "$R1_BASELINE"
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

R2=$(grep -rlE '^void\s+\w+::execute\s*\(\s*Player' \
	Client/Packet/Gpackets Client/Packet/Cpackets Client/Packet/Lpackets \
	Client/Packet/Rpackets Client/Packet/Upackets \
	--include='*.cpp' 2>/dev/null | grep -v Handler | wc -l)
check "R2 (packet cpps defining execute)" "$R2" "$R2_BASELINE"

#----------------------------------------------------------------------
# R3 - sprintf/strcpy/strcat call sites under Client/Packet.
# (snprintf does not match: \b rejects the preceding 'n'.)
#----------------------------------------------------------------------
R3_BASELINE=61

R3=$(grep -rE '\b(sprintf|strcpy|strcat)\s*\(' Client/Packet --include='*.cpp' | wc -l)
check "R3 (unsafe format/copy sites in Client/Packet)" "$R3" "$R3_BASELINE"

#----------------------------------------------------------------------
# R4 - library-compiled .cpp files referencing g_p* client globals.
#
# Membership: every .cpp under the whole-directory library trees, plus
# the two explicit source lists in the top-level CMakeLists.txt
# (VS_UI_CLIENT_SOURCES and PACKETWIRE_SOURCES). Extraction work
# (Phase 4) shrinks this by cutting the global seams.
#----------------------------------------------------------------------
R4_BASELINE=83

lib_members () {
	find basic Client/SpriteLib Client/DXLib Client/framelib Client/TextSystem VS_UI \
		-name '*.cpp' 2>/dev/null
	sed -n '/set(VS_UI_CLIENT_SOURCES/,/^	)/p' CMakeLists.txt \
		| grep -oE 'Client/[A-Za-z0-9_/]+\.cpp'
	sed -n '/set(PACKETWIRE_SOURCES/,/^)/p' CMakeLists.txt \
		| grep -oE 'Client/Packet/[A-Za-z0-9_/]+\.cpp'
}

R4=$(lib_members | sort -u | while read -r f; do
	[ -f "$f" ] && grep -lE '\bg_p[A-Z]' "$f"
done | wc -l)
check "R4 (library cpps referencing g_p globals)" "$R4" "$R4_BASELINE"

#----------------------------------------------------------------------

if [ "$FAIL" -ne 0 ]; then
	echo "ratchets: FAILED"
	exit 1
fi
echo "ratchets: all green"
exit 0
