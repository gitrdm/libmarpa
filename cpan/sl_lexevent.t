#!perl
# Copyright 2013 Jeffrey Kegler
# This file is part of Marpa::R2.  Marpa::R2 is free software: you can
# redistribute it and/or modify it under the terms of the GNU Lesser
# General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# Marpa::R2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser
# General Public License along with Marpa::R2.  If not, see
# http://www.gnu.org/licenses/.

# Test of scannerless parsing -- named lexeme events
# deactivation and reactivation

use 5.010;
use strict;
use warnings;

use Test::More tests => 44;
use English qw( -no_match_vars );
use lib 'inc';
use Marpa::R2::Test;
use Marpa::R2;

my $rules = <<'END_OF_GRAMMAR';
:start ::= sequence
sequence ::= char*
char ::= a | b | c | d
a ~ 'a'
b ~ 'b'
c ~ 'c'
d ~ 'd'

# Marpa::R2::Display
# name: SLIF named lexeme event synopsis

:lexeme ~ <a> pause => before event => 'before a'
:lexeme ~ <b> pause => after event => 'after b'
:lexeme ~ <c> pause => before event => 'before c'
:lexeme ~ <d> pause => after event => 'after d'

# Marpa::R2::Display::End

END_OF_GRAMMAR

# This test the order of events
# No more than one of each event type per line
# so that order is non-arbitrary
my $events_expected = <<'END_OF_EVENTS';
END_OF_EVENTS

my $grammar = Marpa::R2::Scanless::G->new(
    {   action_object => 'My_Actions', source          => \$rules }
);

my $string        = q{aabbbcccdaaabccddddabcd};
my $actual_events = q{};
my $slr           = Marpa::R2::Scanless::R->new( { grammar => $grammar } );
my $length        = length $string;
my $pos           = $slr->read( \$string );
READ: while (1) {

    my @actual_events = ();
    my $event_name;
    my $end_of_lexeme;
    EVENT:
    for my $event (@{$slr->events()}) {
        ($event_name, undef, $end_of_lexeme) = @{$event};
        push @actual_events, $event_name;
    }
    if (@actual_events) {
        $actual_events .= join q{ }, $pos, @actual_events;
        $actual_events .= "\n";
    }
    say STDERR "pos=$pos eol=$end_of_lexeme";
    $pos = $end_of_lexeme // $pos;
    last READ if $pos >= $length;
    $pos = $slr->resume($pos);
} ## end READ: while (1)
my $value_ref = $slr->value();
if ( not defined $value_ref ) {
    die "No parse\n";
}
my $actual_value = ${$value_ref};
Test::More::is( $actual_value, q{1792}, qq{Value for lexeme named event test} );
my $expected_events = q{};
Marpa::R2::Test::is( $actual_events, $expected_events, qq{Events for lexeme named event test} );

package My_Actions;

sub OK { return 1792 }
sub new { return {}; }

# vim: expandtab shiftwidth=4: