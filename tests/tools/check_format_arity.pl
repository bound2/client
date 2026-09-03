#!/usr/bin/env perl
#----------------------------------------------------------------------
# check_format_arity.pl - the check SafeFormat cannot make for itself
#----------------------------------------------------------------------
#
# SafeFormat::Format (basic/SafeFormat.h) makes a data-file format string
# safe: a conversion it cannot satisfy is printed as text instead of
# performed. What it cannot do is tell that the entry MEANS what the call
# site passes. A converted site whose entry carries one %s more than the
# call site hands over is no longer a memory-safety defect - but it now
# puts a literal "%s" in the user interface, which is a visible bug where
# the old code merely printed rubbish.
#
# So every conversion done for docs/RESTRUCTURING.md task 5.4 is checked
# here, against the built-in English table in Client/MGameStringTable.cpp.
# That table is not a sample: InitGameStringTable() installs it over the
# file data on the English path, so it is exactly what the default build
# formats with.
#
# What fails this check:
#
#   * an entry with MORE conversions than its call site passes arguments.
#     This is the direction that mattered - it is what made sprintf read a
#     stack word as a char* - and it is the direction that now shows a
#     specifier to the player. It is also the only check here that needs
#     no judgement: both numbers are countable from the text.
#   * a conversion whose argument is provably of the other kind: a %s
#     handed a numeric literal or an arithmetic expression, or a %d handed
#     something with the unmistakable shape of a string (.c_str(),
#     GetString(), a quoted literal).
#
# Deliberately NOT a failure: a %s whose argument this cannot classify.
# The first draft failed those, and reported 29 problems of which every
# one was a false positive - weapon_speed_string[i], szString, grade_string[i]
# are all char arrays that no amount of pattern matching on an expression
# can recognise without a type. A check that guesses cannot be a gate; the
# summary says how many positions went unclassified so the number is at
# least visible.
#
# What is reported but does NOT fail: an entry with FEWER conversions than
# the call site passes. A surplus argument is harmless in both the old and
# the new code, and there are real sites where the English entry takes no
# conversion while the localised one it was written against takes a %s -
# dropping the argument would break the localised build to tidy this one.
#
# What this cannot see: LANGUAGE != 3. No String.inf ships in this
# repository, so for a localised build the entries are unknowable here.
# That build is the one the load-time gate protects and the one the
# formatter's run-time refusal matters most in.
#
#----------------------------------------------------------------------

use strict;
use warnings;
use File::Basename;
use File::Spec;

my $root = File::Spec->rel2abs(
	File::Spec->catdir(dirname(__FILE__), File::Spec->updir, File::Spec->updir));

my $table = File::Spec->catfile($root, 'Client', 'MGameStringTable.cpp');

#----------------------------------------------------------------------
# The built-in table.
#----------------------------------------------------------------------
my %entry;
{
	open my $fh, '<:raw', $table
		or die "check_format_arity: cannot read $table: $!\n";
	local $/;
	my $t = <$fh>;
	close $fh;

	while ($t =~ /\(\*g_pGameStringTable\)\[\s*([A-Za-z_]\w*)\s*\]\s*=\s*"((?:[^"\\]|\\.)*)"\s*;/g)
	{
		$entry{$1} = $2;
	}
}

if (!%entry)
{
	print "FAIL: no built-in string table entries found in Client/MGameStringTable.cpp\n";
	print "      (the parse broke, which would make every check below vacuous)\n";
	exit 1;
}

#----------------------------------------------------------------------
# The conversions in one format string, as a list of kinds.
#----------------------------------------------------------------------
sub conversions
{
	my ($fmt) = @_;
	my @kinds;
	my $i = 0;

	while ($i < length $fmt)
	{
		if (substr($fmt, $i, 1) ne '%') { $i++; next; }
		$i++;
		if ($i < length($fmt) && substr($fmt, $i, 1) eq '%') { $i++; next; }

		$i++ while $i < length($fmt) && substr($fmt, $i, 1) =~ /[-+ 0#]/;
		$i++ while $i < length($fmt) && substr($fmt, $i, 1) =~ /[0-9*]/;

		if ($i < length($fmt) && substr($fmt, $i, 1) eq '.')
		{
			$i++;
			$i++ while $i < length($fmt) && substr($fmt, $i, 1) =~ /[0-9*]/;
		}

		$i++ while $i < length($fmt) && substr($fmt, $i, 1) =~ /[hljztLwI0-9]/;

		last if $i >= length $fmt;

		my $c = substr($fmt, $i, 1);
		$i++;

		if    ($c =~ /[diuoxXc]/)  { push @kinds, 'int' }
		elsif ($c eq 's')          { push @kinds, 'str' }
		elsif ($c =~ /[eEfFgGaA]/) { push @kinds, 'flt' }
		elsif ($c eq 'p')          { push @kinds, 'ptr' }
		else                       { push @kinds, "?$c" }
	}

	return @kinds;
}

#----------------------------------------------------------------------
# Split a call's arguments at top level.
#----------------------------------------------------------------------
sub split_args
{
	my ($s) = @_;

	# Declared one at a time on purpose: a list assignment whose first
	# target is an array gives every scalar after it undef, and the first
	# draft of this file did exactly that - which made every call parse as
	# nothing, so the check ran over 224 sites, verified none of them, and
	# reported OK.
	my @out;
	my $cur   = '';
	my $depth = 0;
	my $instr = 0;
	my $inchr = 0;

	for my $i (0 .. length($s)-1)
	{
		my $c = substr($s, $i, 1);
		my $p = $i ? substr($s, $i-1, 1) : '';

		if ($instr) { $cur .= $c; $instr = 0 if $c eq '"' && $p ne "\\"; next }
		if ($inchr) { $cur .= $c; $inchr = 0 if $c eq "'" && $p ne "\\"; next }

		if ($c eq '"')  { $instr = 1; $cur .= $c; next }
		if ($c eq "'")  { $inchr = 1; $cur .= $c; next }
		if ($c =~ /[([]/) { $depth++; $cur .= $c; next }
		if ($c =~ /[)\]]/) { $depth--; $cur .= $c; next }
		if ($c eq ',' && $depth == 0) { push @out, $cur; $cur = ''; next }

		$cur .= $c;
	}

	push @out, $cur if $cur =~ /\S/;
	return @out;
}

#----------------------------------------------------------------------
# Classify an argument expression, but only when the shape is certain.
# Returns 'str', 'num' or '' for "cannot tell", which is the common case
# and must never be treated as a defect.
#----------------------------------------------------------------------
sub arg_kind
{
	my ($a) = @_;
	$a =~ s/^\s+|\s+$//g;

	return 'str' if $a =~ /\.c_str\s*\(\s*\)$/;
	return 'str' if $a =~ /GetString\s*\(\s*\)$/;
	return 'str' if $a =~ /GetGameString\s*\(/;
	return 'str' if $a =~ /GetHName\s*\(\s*\)$/;
	return 'str' if $a =~ /^"/;

	# A number, or arithmetic over names and numbers with no call, index,
	# member access or address-of in it - i+1, n*2, LEVEL_BASE+g. Nothing
	# of that shape is a char*.
	return 'num' if $a =~ /^-?\d+$/;
	return 'num' if $a =~ /^[A-Za-z_0-9 \t+\-*\/%]+$/ && $a =~ /[+\-*\/%]/;

	return '';
}

#----------------------------------------------------------------------
# Every converted call site.
#----------------------------------------------------------------------
my @sources;
for my $dir ('Client', 'VS_UI')
{
	my $path = File::Spec->catdir($root, $dir);
	open my $find, '-|', "find \"$path\" -name '*.cpp'"
		or die "check_format_arity: cannot list $path: $!\n";
	while (<$find>) { chomp; push @sources, $_ }
	close $find;
}

my ($sites, $checked, $unresolved, $failures, $notes, $unclassified)
	= (0, 0, 0, 0, 0, 0);

for my $file (@sources)
{
	open my $fh, '<:raw', $file or next;
	local $/;
	my $text = <$fh>;
	close $fh;

	next unless $text =~ /SafeFormat::Format/;

	my $rel = $file;
	$rel =~ s/^\Q$root\E[\/\\]?//;
	$rel =~ s/\\/\//g;

	while ($text =~ /SafeFormat::Format\s*\(/g)
	{
		my $open = pos $text;

		# The line this call starts on, and whether it is commented out.
		my $before = substr($text, 0, $open);
		my $line   = 1 + ($before =~ tr/\n//);
		my ($tail) = ($before =~ /([^\n]*)$/);
		next if defined $tail && $tail =~ m{//};

		my $depth = 1;
		my $i = $open;
		while ($i < length($text) && $depth)
		{
			my $c = substr($text, $i, 1);
			$depth++ if $c eq '(';
			$depth-- if $c eq ')';
			$i++;
		}

		my @a = split_args(substr($text, $open, $i - $open - 1));
		next if @a < 2;
		$sites++;

		# A literal format is not a table site at all - the formatter is
		# used at a few of those purely for the bound.
		if ($a[1] =~ /^\s*"/)
		{
			$sites--;
			next;
		}

		my ($id) = ($a[1] =~ /GetGameString\s*\(\s*([A-Za-z_]\w*)\s*\)/);

		unless (defined $id && exists $entry{$id})
		{
			# A computed id, or one the English table does not set. Those
			# are the sites GetGameString's range check exists for.
			$unresolved++;
			next;
		}

		$checked++;

		my @conv = conversions($entry{$id});
		my @args = @a[2 .. $#a];

		if (@conv > @args)
		{
			printf "FAIL %s:%d  %s\n     entry \"%s\" takes %d conversion(s), the call passes %d argument(s)\n",
				$rel, $line, $id, $entry{$id}, scalar(@conv), scalar(@args);
			$failures++;
			next;
		}

		if (@conv < @args)
		{
			printf "note %s:%d  %s: %d surplus argument(s) against the English entry \"%s\"\n",
				$rel, $line, $id, scalar(@args) - scalar(@conv), $entry{$id};
			$notes++;
		}

		for my $j (0 .. $#conv)
		{
			my $wants = $conv[$j] eq 'str' ? 'str'
					  : $conv[$j] eq 'ptr' ? ''
					  :                      'num';

			next unless $wants;

			my $got = arg_kind($args[$j]);

			unless ($got)
			{
				$unclassified++;
				next;
			}

			next if $got eq $wants;

			my $shown = $args[$j];
			$shown =~ s/^\s+|\s+$//g;

			printf "FAIL %s:%d  %s\n     entry \"%s\" wants %s for conversion %d, the call passes %s, which is %s\n",
				$rel, $line, $id, $entry{$id},
				($wants eq 'str' ? 'a string' : 'a number'),
				$j+1, $shown,
				($got eq 'str' ? 'a string' : 'arithmetic');
			$failures++;
		}
	}
}

printf "\ncheck_format_arity: %d converted site(s), %d checked against the built-in table, %d with an id it does not set, %d argument(s) whose kind could not be told, %d note(s)\n",
	$sites, $checked, $unresolved, $unclassified, $notes;

#----------------------------------------------------------------------
# A check that checked nothing must not report OK.
#
# The first draft of this file did exactly that: a Perl list assignment
# whose first target is an array left every scalar in split_args undef,
# so no call parsed, all 224 sites fell into the unresolved bucket, and
# it printed OK. A tool written to catch a silent failure is worth
# nothing if it can fail silently itself.
#----------------------------------------------------------------------
if ($sites && !$checked)
{
	print "FAIL: found converted sites but resolved none of them to a table entry - the parse is broken\n";
	exit 1;
}

if ($failures)
{
	printf "FAILED: %d problem(s)\n", $failures;
	exit 1;
}

print "OK\n";
exit 0;
