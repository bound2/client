#!/usr/bin/env perl
#----------------------------------------------------------------------
# check_includes.pl - include-graph architecture rules
# (docs/RESTRUCTURING.md, task 0.3)
#----------------------------------------------------------------------
#
# Walks the quote-include closure of every packetwire member and fails
# on forbidden edges. Perl, not python: the dev machines have Git for
# Windows' perl, and the wire-inventory generator is already perl.
#
# Rules:
#   W1  A file in the packetwire closure may quote-include only files
#       under Client/Packet/ or basic/, or Client/Client_PCH.h (the
#       shared precompiled header - its own includes are walked too).
#       Anything else means game code is reachable from the wire layer.
#   W2  The debug facilities whose definitions live in the executable
#       or VS_UI (MinTr.h, DebugInfo.h, DebugKit.h) may not be included
#       anywhere in the closure, so the library stays linkable into a
#       test binary without game stubs.
#
# An include that cannot be resolved against the search path at all is
# reported as a violation too - a silent skip is how a rule rots.
#
# Membership is parsed from the PACKETWIRE_SOURCES list in the
# top-level CMakeLists.txt, so the CMake target and this checker cannot
# disagree about what "the library" is.
#
# Grandfathered violations, if ever needed, live one per line in
# tests/arch/baseline.txt as "RULE|includer|include" - the list is
# frozen shrink-only: an entry that stops matching fails the run until
# it is deleted. It is currently empty by design.
#
#----------------------------------------------------------------------
use strict;
use warnings;
use File::Basename qw(dirname);

my $root = dirname(__FILE__) . "/../..";
chdir $root or die "cannot chdir to repo root: $!";

#----------------------------------------------------------------------
# Membership from CMakeLists.txt
#----------------------------------------------------------------------
my @members;
{
	open my $fh, '<', 'CMakeLists.txt' or die "CMakeLists.txt: $!";
	my $in = 0;
	while (<$fh>) {
		$in = 1 if /^set\(PACKETWIRE_SOURCES/;
		if ($in) {
			push @members, $1 if m{(Client/Packet/[A-Za-z0-9_/]+\.cpp)};
			last if /^\)/;
		}
	}
	close $fh;
}
die "no PACKETWIRE_SOURCES found in CMakeLists.txt" unless @members >= 10;

#----------------------------------------------------------------------
# Include resolution: the file's own directory first (how the compiler
# treats quote-includes), then the include path the library compiles
# with.
#----------------------------------------------------------------------
my @searchdirs = ('Client/Packet', 'Client/Packet/Types', 'Client', 'basic', '.');

sub normalize {
	my ($p) = @_;
	$p =~ s{\\}{/}g;
	my @out;
	for my $part (split m{/}, $p) {
		next if $part eq '.' or $part eq '';
		if ($part eq '..') { pop @out; next; }
		push @out, $part;
	}
	return join('/', @out);
}

sub resolve_include {
	my ($includer, $inc) = @_;
	for my $dir (dirname($includer), @searchdirs) {
		my $cand = normalize("$dir/$inc");
		return $cand if -f $cand;
	}
	return undef;
}

#----------------------------------------------------------------------
# Baseline (frozen, shrink-only)
#----------------------------------------------------------------------
my %baseline;
if (open my $fh, '<', 'tests/arch/baseline.txt') {
	while (<$fh>) {
		s/\r?\n$//;	# belt and braces beside the .gitattributes eol=lf
		next if /^\s*(#|$)/;
		$baseline{$_} = 0;	# 0 = not yet seen this run
	}
	close $fh;
}

#----------------------------------------------------------------------
# Closure walk
#----------------------------------------------------------------------
my %banned = map { $_ => 1 } qw(MinTr.h DebugInfo.h DebugKit.h);

my @violations;
my %seen;
my @queue = @members;

sub violate {
	my ($key) = @_;
	if (exists $baseline{$key}) {
		$baseline{$key} = 1;
		return;
	}
	push @violations, $key;
}

while (@queue) {
	my $file = shift @queue;
	next if $seen{$file}++;

	open my $fh, '<', $file or next;
	while (<$fh>) {
		next unless /^\s*#\s*include\s*"([^"]+)"/;
		my $inc = $1;

		my ($base) = $inc =~ m{([^/\\]+)$};
		if ($banned{$base}) {
			violate("W2|$file|$inc");
			next;
		}

		my $resolved = resolve_include($file, $inc);
		if (!defined $resolved) {
			violate("W1|$file|$inc (unresolved)");
			next;
		}

		if ($resolved =~ m{^Client/Packet/} or
		    $resolved =~ m{^basic/} or
		    $resolved eq 'Client/Client_PCH.h') {
			push @queue, $resolved;
		} else {
			violate("W1|$file|$inc -> $resolved");
		}
	}
	close $fh;
}

#----------------------------------------------------------------------
# Report
#----------------------------------------------------------------------
my $fail = 0;

for my $v (@violations) {
	print "VIOLATION $v\n";
	$fail = 1;
}
for my $b (sort keys %baseline) {
	next if $baseline{$b};
	print "STALE BASELINE $b - no longer a violation; delete the line (shrink-only)\n";
	$fail = 1;
}

my $count = scalar keys %seen;
if ($fail) {
	print "arch_includes: FAILED ($count files walked)\n";
	exit 1;
}
print "arch_includes: OK ($count files walked, W1/W2 clean)\n";
exit 0;
