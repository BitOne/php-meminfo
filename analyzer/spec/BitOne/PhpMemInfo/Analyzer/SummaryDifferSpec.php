<?php

namespace spec\BitOne\PhpMemInfo\Analyzer;

use PhpSpec\ObjectBehavior;

class SummaryDifferSpec extends ObjectBehavior
{

    public function it_is_initializable()
    {
        $this->beConstructedWith([], []);
        $this->shouldHaveType('BitOne\PhpMemInfo\Analyzer\SummaryDiffer');
    }

    public function it_generates_diff()
    {
        $firstItems = [
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
        ];

        $secondItems = [
            "0x7fd04f5d4bb8" => [
                "type" => "string",
                "size" => "24"
            ],
            "0x7fc321a94050" => [
                "type" => "object",
                "class" => "MyClassA",
                "size" => "56"
            ],
            "0x7fc321a94080" => [
                "type" => "object",
                "class" => "MyClassA",
                "size" => "56"
            ],
            "0x7fd04f5d4bc8" => [
                "type" => "object",
                "class" => "MyClassA",
                "size" => "26",
            ],
            "0x7fd04f5d4bd8" => [
                "type" => "object",
                "class" => "MyClassA",
                "size" => "16",
            ],
            "0x7fb321a941e0" => [
                "type" => "integer",
                "size" => "24"
            ],
            "0x7fb321a94268" => [
                "type" => "integer",
                "size" => "24"
            ],
            "0x7fd04f5d4bf8" => [
                "type"  => "object",
                "class" => "MyClassC",
                "size"  => "56"
            ]
        ];

        $this->beConstructedWith($firstItems, $secondItems);

        $this->generateDiff()->shouldReturn([
            ['MyClassA', 2, 112, '+2', '+42'],
            ['integer', 2, 48, '0', '0'],
            ['MyClassC', 0, 0, '+1', '+56'],
            ['string', 4, 140, '-3', '-116'],
            ['MyClassB', 1, 56, '-1', '-56'],
        ]);
    }
}
