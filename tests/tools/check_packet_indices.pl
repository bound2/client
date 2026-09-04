#!/usr/bin/perl
#----------------------------------------------------------------------
# check_packet_indices.pl - server-supplied values used as array
# subscripts in the packet handlers (ctest `packet_indices`).
#----------------------------------------------------------------------
#
# Code-health priority 1: "Client/Packet/Gpackets/ passes server-supplied
# lengths, indices and item classes straight into array subscripts". The
# lengths half is ratchet R3, now at 0. This is the indices half.
#
# IT WALKS THE VALUE, NOT THE SPELLING, and that is the whole point.
# `array[pPacket->getSlotID()]` is one spelling and the tree has two live
# instances of it; the population is `int slot = pPacket->getSlotID();`
# twenty lines above `array[slot]`, and there are a hundred of those.
# Grepping the first shape and concluding anything about the second is
# the mistake ratchet R7 made about format strings, twice.
#
# What it reports is deliberately narrower than what it finds. Most of
# the population subscripts a CTypeTable - g_pActionInfoTable,
# g_pSkillInfoTable, g_pEffectStatusTable, g_pSkillManager - and
# CTypeTable::operator[] range-checks in EVERY build (e65ab7a), so an
# out-of-range index there yields a default-constructed element rather
# than a wild access. Those are counted and excluded. What is left is
# the subscripts into raw arrays, where nothing stands between the wire
# and the memory except a guard someone remembered to write.
#
# The floor is a ceiling: a rise means a new raw-container subscript,
# which has to be read and either guarded or explained. It cannot tell a
# guarded site from an unguarded one - detecting the guard is beyond a
# script - so it holds the population steady and makes a human look at
# each addition.
#
#----------------------------------------------------------------------
# The eleven it reports today are all guarded, by the index pass of
# 2026-09-04 that found three defects among them:
#
#   - GCPhoneConnected / GCPhoneDisconnected / GCPhoneSay / GCRing index
#     UserInformation's PCSUserName[MAX_PCS_SLOT] and
#     OtherPCSNumber[MAX_PCS_SLOT] with a SlotID_t - a BYTE - that no
#     read() bounds. MString::operator= reads m_pString out of whatever
#     is at that offset and delete[]s it, so an out-of-range slot was an
#     arbitrary free and an arbitrary write at a server's discretion.
#   - GCRemoveFromGear reads addonSlot[slotID] from arrays of 15 and 16
#     entries, while MSlayerGear::RemoveItem bounds slotID to m_Size -
#     MAX_GEAR_SLAYER (27) or MAX_GEAR_VAMPIRE/OUSTERS (28). Unequipping
#     a ZAP, a PDA, a shoulder or a blood bible read up to twelve ints
#     past a stack array. No hostile server needed for that one.
#   - GCLearnSkillReady reads SKILLDOMAIN_NAME[domainType] from an
#     unbounded BYTE, then hands the result to the string table as an id.
#----------------------------------------------------------------------
use strict;
use warnings;
use File::Find;

my $ROOT = shift || '.';

# A rise means a new raw-container subscript. Read it, guard it, then
# raise this line in the same commit.
my $MAXIMUM_RAW = 11;

my @sources;
find(sub { push @sources, $File::Find::name if /\.cpp$/i }, "$ROOT/Client/PacketHandler");

die "check_packet_indices: found no sources under $ROOT/Client/PacketHandler\n"
	unless @sources;

my $checked  = 0;
my $raw      = 0;
my @raw_hits;

for my $path (sort @sources)
{
	open my $fh, '<:raw', $path or die "$path: $!\n";
	my $text = do { local $/; <$fh> };
	close $fh;

	# Comments out, newlines kept so the reported line numbers are real.
	$text =~ s{/\*.*?\*/}{ my $m = $&; $m =~ s/[^\r\n]//g; $m }gse;
	$text =~ s{//[^\r\n]*}{}g;

	# Locals taking a packet value, with an optional cast.
	my %from_packet;

	while ($text =~ /(\w+)\s*=\s*(?:\([^()\r\n]*\)\s*)?pPacket\s*->\s*(get\w+)\s*\(/g)
	{
		$from_packet{$1} = $2;
	}

	next unless %from_packet;

	my @lines = split /\r?\n/, $text;

	for my $i (0 .. $#lines)
	{
		my $line = $lines[$i];

		# The index class forbids a quote, so a string literal cannot
		# masquerade as an index.
		while ($line =~ /\[([^\[\]"\r\n]{1,60})\]/g)
		{
			my $index = $1;
			my $end   = pos($line);

			my ($name) = grep { $index =~ /\b\Q$_\E\b/ } sort keys %from_packet;
			next unless defined $name;

			$checked++;

			# Whatever is being subscripted: the token run before '['.
			my $before    = substr($line, 0, $end - length($index) - 2);
			my $container = ($before =~ /([\w\]\)>.:*\-]{1,50})$/) ? $1 : '?';

			# A dereferenced global table pointer is a CTypeTable, which
			# range-checks itself. Everything else is raw.
			#
			# Matched WITHOUT the opening paren, because the container
			# capture above stops at it: the class that walks back from
			# the '[' deliberately excludes '(' so it cannot run off
			# into a preceding call's arguments, which leaves
			# "*g_pActionInfoTable)". The first draft of this line
			# asked for "\(\*g_p\w+\)" and excluded nothing, so all 102
			# were reported as raw - a filter that silently matches
			# nothing is the failure this whole file exists to avoid.
			next if $container =~ /^\*g_p\w+\)$/;

			$raw++;
			push @raw_hits, sprintf("  %s:%d  %s[%s]  <- %s()",
				$path, $i + 1, $container, $index, $from_packet{$name});
		}
	}
}

print "$_\n" for @raw_hits;

printf "check_packet_indices: %d packet-indexed subscript(s), %d into a raw container\n",
	$checked, $raw;

if ($raw > $MAXIMUM_RAW)
{
	printf "FAIL: %d raw-container subscripts, more than the %d recorded here.\n",
		$raw, $MAXIMUM_RAW;
	print  "      A server-supplied index reaches an array nothing range-checks.\n";
	print  "      Read the new site, guard it, and raise \$MAXIMUM_RAW in this file\n";
	print  "      in the same commit.\n";
	exit 1;
}

if ($raw < $MAXIMUM_RAW)
{
	printf "FAIL: %d raw-container subscripts, fewer than the %d recorded here - progress!\n",
		$raw, $MAXIMUM_RAW;
	print  "      Lower \$MAXIMUM_RAW in this file in the same commit.\n";
	exit 1;
}

print "OK\n";
exit 0;
