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
            "0x7fb321a94378" => 12,
            "0x7fb321a941e0" => 7,
            "0x7fb321a94080" => 6
        ]);
    }
}
