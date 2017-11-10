<?php

namespace spec\BitOne\PhpMemInfo\Analyzer;

use PhpSpec\ObjectBehavior;
use Prophecy\Argument;

class SummaryCreatorSpec extends ObjectBehavior
{
    function it_is_initializable()
    {
        $this->beConstructedWith([]);
        $this->shouldHaveType('BitOne\PhpMemInfo\Analyzer\SummaryCreator');
    }

    function it_creates_summary_with_primitives()
    {
        $this->beConstructedWith([
            "0x7fb321a94050" => [
                "type" => "string",
                "size" => "29"
            ],
            "0x7fb321a94080" => [
                "type" => "string",
                "size" => "29"
            ],
            "0x7fb321a94378" => [
                "type" => "array",
                "size" => "96"
            ],
            "0x7fb321a94108" => [
                "type" => "string",
                "size" => "37"
            ],
            "0x7fb321a941e0" => [
                "type" => "integer",
                "size" => "24"
            ],
            "0x7fb321a94268" => [
                "type" => "integer",
                "size" => "24"
            ]
        ]);

        $this->createSummary()->shouldReturn([
            "string" => [
                "count"     => 3,
                "self_size" => 95
            ],
            "integer" => [
                "count"     => 2,
                "self_size" => 48
            ],
            "array" => [
                "count"     => 1,
                "self_size" => 96
            ]
        ]);

    }

    function it_creates_summary_with_objects()
    {
        $this->beConstructedWith([
            "0x7fb321a94050" => [
                "type"  => "object",
                "class" => "MyClassA",
                "size"  => "56"
            ],
            "0x7fb321a94080" => [
                "type"  => "object",
                "class" => "MyClassA",
                "size"  => "56"
            ],
            "0x7fb321a94378" => [
                "type"  => "object",
                "class" => "MyClassB",
                "size"  => "56"
            ],
            "0x7fb321a94108" => [
                "type"  => "object",
                "class" => "MyClassA",
                "size"  => "56"
            ],
            "0x7fb321a941e0" => [
                "type"  => "object",
                "class" => "MyClassC",
                "size"  => "56"
            ],
            "0x7fb321a94268" => [
                "type"  => "object",
                "class" => "MyClassC",
                "size"  => "56"
            ]
        ]);

        $this->createSummary()->shouldReturn([
            "MyClassA" => [
                "count"     => 3,
                "self_size" => 168
            ],
            "MyClassC" => [
                "count"     => 2,
                "self_size" => 112
            ],
            "MyClassB" => [
                "count"     => 1,
                "self_size" => 56
            ]
        ]);
    }

    function it_creates_summary_with_primitives_and_objects()
    {
        $this->beConstructedWith([
            "0x7fb321a94050" => [
                "type" => "string",
                "size" => "29"
            ],
            "0x7fb321a94080" => [
                "type" => "string",
                "size" => "29"
            ],
            "0x7fb321a94108" => [
                "type" => "string",
                "size" => "37"
            ],
            "0x7fe321a94108" => [
                "type" => "string",
                "size" => "45"
            ],
            "0x7fb321a941e0" => [
                "type" => "integer",
                "size" => "24"
            ],
            "0x7fb321a94268" => [
                "type" => "integer",
                "size" => "24"
            ],
            "0x7fc321a94050" => [
                "type"  => "object",
                "class" => "MyClassA",
                "size"  => "56"
            ],
            "0x7fc321a94080" => [
                "type"  => "object",
                "class" => "MyClassA",
                "size"  => "56"
            ],
            "0x7fc321a94378" => [
                "type"  => "object",
                "class" => "MyClassB",
                "size"  => "56"
            ]
        ]);

        $this->createSummary()->shouldReturn([
            "string" => [
                "count"     => 4,
                "self_size" => 140
            ],
            "MyClassA" => [
                "count"     => 2,
                "self_size" => 112
            ],
            "integer" => [
                "count"     => 2,
                "self_size" => 48
            ],
            "MyClassB" => [
                "count"     => 1,
                "self_size" => 56
            ]
        ]);
    }
}
