<?php

namespace spec\BitOne\PhpMemInfo\Analyzer;

use PhpSpec\ObjectBehavior;
use Prophecy\Argument;

class TopSearcherSpec extends ObjectBehavior
{
    function it_is_initializable()
    {
        $this->beConstructedWith([]);
        $this->shouldHaveType('BitOne\PhpMemInfo\Analyzer\TopSearcher');
    }

    function it_is_top_size()
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

        $this->createTopSize(3)->shouldReturn([
            [
                "size" => "29",
                "ref"  => "0x7fb321a94050"
            ],
            [
                "size" => "37",
                "ref"  => "0x7fb321a94108"
            ],
            [
                "size" => "96",
                "ref"  => "0x7fb321a94378"
            ]
        ]);
    }

    function it_is_top_children()
    {
        $this->beConstructedWith([
            "0x7fb321a94050" => [
                "children" => range(0, 3)
            ],
            "0x7fb321a94080" => [
                "children" => range(0, 5)
            ],
            "0x7fb321a94378" => [
                "children" => range(0, 11)
            ],
            "0x7fb321a94108" => [
                "children" => range(0, 1)
            ],
            "0x7fb321a941e0" => [
                "children" => range(0, 6)
            ],
            "0x7fb321a94268" => [
                "children" => range(0, 1)
            ]
        ]);

        $this->createTopChildren(3)->shouldReturn([
            [
                "links" => 6,
                "ref"  => "0x7fb321a94080"
            ],
            [
                "links" => 7,
                "ref"  => "0x7fb321a941e0"
            ],
            [
                "links" => 12,
                "ref"  => "0x7fb321a94378"
            ]
        ]);
    }
}
