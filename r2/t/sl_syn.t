#!perl
# Copyright 2012 Jeffrey Kegler
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

# Synopsis for scannerless parsing, main POD page

use 5.010;
use strict;
use warnings;

use Test::More tests => 5;
use English qw( -no_match_vars );
use lib 'inc';
use Marpa::R2::Test;
use Marpa::R2;

my $grammar = Marpa::R2::Scanless::G->new(
    {
        action_object        => 'My_Actions',
        source          => \(<<'END_OF_SOURCE'),
:start ::= <number sequence>
<number sequence> ::= <number>+ action => add_sequence
number ~ digit+
digit ~ [0-9]

:discard ~ whitespace
whitespace ~ [\s]+
END_OF_SOURCE
    }
);

package My_Actions;
our $SELF;
sub new { return $SELF }
sub add_sequence {
    my ($self, @numbers) = @_;
    return List::Util::sum @numbers, 0;
}

sub show_sequence_so_far {
    my ($self) = @_;
    my $slr = $self->{slr};
    my ( $start, $end ) = $slr->last_completed_range('number sequence');
    return if not defined $start;
    my $sequence_so_far = $slr->range_to_string( $start, $end );
    return $sequence_so_far;
} ## end sub show_sequence_so_far

package main;

sub my_parser {
    my ( $grammar, $string ) = @_;

    my $self = bless { grammar => $grammar, input => \$string, },
        'My_Actions';
    local $My_Actions::SELF = $self;

    my $slr = Marpa::R2::Scanless::R->new( { grammar => $grammar } );
    $self->{slr} = $slr;
    my $event_count;
    my ( $parse_value, $parse_status, $sequence_so_far );

    if ( not defined eval { $event_count = $slr->read(\$string); 1 } ) {
        die join " ", 'No parse', $EVAL_ERROR, $self->show_sequence_so_far();
    }
    if ( not defined $event_count ) {
        my $error = $slr->error();
        die join " ",
        'No parse', $error, $self->show_sequence_so_far();
    }
    my $value_ref = $slr->value;
    if ( not defined $value_ref ) {
        die join " ",
            'No parse', 'Input read to end but no parse',
            $self->show_sequence_so_far();
    } ## end if ( not defined $value_ref )
    return ${$value_ref};
} ## end sub my_parser

my @tests_data = (
    [ ' 1 2 3   1 2 4', 13, ],
    [ ' 8675311',       8675311, ],
    [ '867 5311',       6178, ],
    [ ' 8 6 7 5 3 1 1', 31, ],
    [ '1234',           1234 ],
);

TEST:
for my $test_data (@tests_data) {
    my ($test_string,     $expected_value) = @{$test_data};
    my ($actual_value ) = my_parser( $grammar, $test_string );
    Test::More::is( $actual_value, $expected_value, qq{Value of "$test_string"} );
} ## end TEST: for my $test_string (@test_strings)

# vim: expandtab shiftwidth=4:
