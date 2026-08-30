#!/usr/bin/perl
#-----------------------------------------------------------------------------
# npcscript_dump.pl - dump Data/Info/NPCScript.inf to a reviewable TSV
#-----------------------------------------------------------------------------
# The shipped NPCScript.inf is CP949 and holds every line of NPC dialogue the
# client shows: one "subject" (what the NPC says) and N "contents" (the answers
# the player picks from) per script id. MNPCScriptTable.h documents the layout:
#
#	int   totalNumberOfScript
#	{
#		int    scriptID
#		int    ownerIDLength;  char ownerID[]
#		int    numberOfSubject
#		{ int length; char text[] }
#		int    numberOfContent
#		{ int length; char text[] }
#	}
#
# The header count is larger than the number of records actually present, and the
# file ends with a four byte remnant that is not a record, so the reader stops at
# end of file rather than trusting the count.
#
# Output is a five column TSV - id, kind (S/C), index, owner, text - with the
# text decoded to UTF-8 and its control characters escaped, so one string is
# always one line. npcscript_gen.pl turns the translated file back into the C++
# override table.
#
#	perl npcscript_dump.pl <NPCScript.inf> <out.tsv>
#-----------------------------------------------------------------------------

use strict;
use warnings;
use Encode ();

my ($inPath, $outPath) = @ARGV;
die "usage: $0 <NPCScript.inf> <out.tsv>\n" unless defined $inPath && defined $outPath;

open my $in, '<:raw', $inPath or die "$inPath: $!\n";
local $/;
my $buf = <$in>;
close $in;

my $len = length $buf;
my $pos = 0;

sub u32 {
	die "truncated integer at offset $pos\n" if $pos + 4 > $len;
	my $v = unpack 'V', substr($buf, $pos, 4);
	$pos += 4;
	return $v;
}

sub bstr {
	my $n = u32();
	die "string length $n at offset $pos runs past end of file\n" if $pos + $n > $len;
	my $s = substr($buf, $pos, $n);
	$pos += $n;
	return $s;
}

# CP949 is the encoding the game data ships in. A string that does not decode is
# kept as raw bytes and flagged, rather than silently mangled.
#
# decode() consumes what it converts when CHECK is set, so it is handed a copy
# and the original is what the failure path returns.
sub decode_cp949 {
	my ($bytes) = @_;
	my $copy = $bytes;
	my $text = eval { Encode::decode('cp949', $copy, Encode::FB_CROAK) };
	return defined $text ? (Encode::encode('UTF-8', $text), 1) : ($bytes, 0);
}

# One string per line: escape the backslash first so the escapes stay reversible.
sub escape {
	my ($s) = @_;
	$s =~ s/\\/\\\\/g;
	$s =~ s/\r/\\r/g;
	$s =~ s/\n/\\n/g;
	$s =~ s/\t/\\t/g;
	$s =~ s/([\x00-\x1f\x7f])/sprintf('\\x%02x', ord $1)/ge;
	return $s;
}

open my $out, '>:raw', $outPath or die "$outPath: $!\n";

my $declared = u32();
my ($scripts, $subjects, $contents, $undecodable) = (0, 0, 0, 0);

# A record is at least an id, an owner length and two counts, so anything shorter
# left over is the trailing remnant and not a truncated record.
while ($len - $pos >= 16) {
	my $id      = u32();
	my $owner   = bstr();
	my @subject = map { bstr() } 1 .. u32();
	my @content = map { bstr() } 1 .. u32();

	$scripts++;
	$subjects += scalar @subject;
	$contents += scalar @content;

	my ($ownerText, $ownerOk) = decode_cp949($owner);
	$undecodable++ unless $ownerOk;
	$ownerText = escape($ownerText);

	for my $i (0 .. $#subject) {
		my ($text, $ok) = decode_cp949($subject[$i]);
		$undecodable++ unless $ok;
		print $out join("\t", $id, 'S', $i, $ownerText, escape($text)), "\n";
	}

	for my $i (0 .. $#content) {
		my ($text, $ok) = decode_cp949($content[$i]);
		$undecodable++ unless $ok;
		print $out join("\t", $id, 'C', $i, $ownerText, escape($text)), "\n";
	}
}

close $out;

printf STDERR "%s: %d scripts (header claims %d), %d subjects, %d contents, %d undecodable, %d trailing bytes\n",
	$inPath, $scripts, $declared, $subjects, $contents, $undecodable, $len - $pos;
