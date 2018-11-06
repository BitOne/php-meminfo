<?php

namespace spec\BitOne\PhpMemInfo\Analyzer;

use PhpSpec\ObjectBehavior;
use Prophecy\Argument;

class InclusiveSizeCalculatorSpec extends ObjectBehavior
{
    function it_is_initializable()
    {
        $this->beConstructedWith([]);
        $this->shouldHaveType('BitOne\PhpMemInfo\Analyzer\InclusiveSizeCalculator');
    }

    function it_computes_inclusive_size_for_non_parent_items()
    {
        $this->beConstructedWith([
            "item_a" => [
                "size" => 1
            ],
            "item_b" => [
                "size" => 2
            ]
        ]);

        $this->calculateInclusiveSize()->shouldReturn([
            "item_a" => [
                "size" => 1,
                "inclusive_size" => 1
            ],
            "item_b" => [
                "size" => 2,
                "inclusive_size" => 2
            ]
        ]);
    }

    function it_computes_inclusive_size_for_one_level_parent_items()
    {
        $this->beConstructedWith([
            "parent_item" => [
                "size" => 1,
                "children" => [
                    "a" => "item_a",
                    "b" => "item_b"
                ]
            ],
            "item_a" => [
                "size" => 2
            ],
            "item_b" => [
                "size" => 3
            ]
        ]);

        $this->calculateInclusiveSize()->shouldReturn([
            "parent_item" => [
                "size" => 1,
                "children" => [
                    "a" => "item_a",
                    "b" => "item_b"
                ],
                "inclusive_size" => 6
            ],
            "item_a" => [
                "size" => 2,
                "inclusive_size" => 2
            ],
            "item_b" => [
                "size" => 3,
                "inclusive_size" => 3
            ]
        ]);
    }

    function it_computes_inclusive_size_for_two_level_parent_items()
    {
        $this->beConstructedWith([
            "grand_parent_item" => [
                "size" => 1,
                "children" => [
                    "a" => "parent_item_a",
                    "b" => "parent_item_b"
                ]
            ],
            "parent_item_a" => [
                "size" => 2,
                "children" => [
                    "a" => "item_a",
                    "b" => "item_b"
                ]
            ],
            "parent_item_b" => [
                "size" => 3,
                "children" => [
                    "c" => "item_c"
                ]
            ],
            "item_a" => [
                "size" => 4
            ],
            "item_b" => [
                "size" => 5
            ],
            "item_c" => [
                "size" => 6
            ]
        ]);

        $this->calculateInclusiveSize()->shouldReturn([
            "grand_parent_item" => [
                "size" => 1,
                "children" => [
                    "a" => "parent_item_a",
                    "b" => "parent_item_b"
                ],
                "inclusive_size" => 21
            ],
            "parent_item_a" => [
                "size" => 2,
                "children" => [
                    "a" => "item_a",
                    "b" => "item_b"
                ],
                "inclusive_size" => 11
            ],
            "parent_item_b" => [
                "size" => 3,
                "children" => [
                    "c" => "item_c"
                ],
                "inclusive_size" => 9
            ],
            "item_a" => [
                "size" => 4,
                "inclusive_size" => 4
            ],
            "item_b" => [
                "size" => 5,
                "inclusive_size" => 5
            ],
            "item_c" => [
                "size" => 6,
                "inclusive_size" => 6
            ]
        ]);
    }

    function it_computes_inclusive_size_for_circular_referenced_items()
    {
        $this->beConstructedWith([
            "item_a" => [
                "size" => 1,
                "children" => [
                    "b" => "item_b",
                ]
            ],
            "item_b" => [
                "size" => 2,
                "children" => [
                    "a" => "item_a",
                ]
            ],
        ]);

        $this->calculateInclusiveSize()->shouldReturn([
            "item_a" => [
                "size" => 1,
                "children" => [
                    "b" => "item_b"
                ],
                "inclusive_size" => 3
            ],
            "item_b" => [
                "size" => 2,
                "children" => [
                    "a" => "item_a",
                ],
                "inclusive_size" => 3
            ],
        ]);
    }

    function it_computes_inclusive_size_for_indirectly_circular_referenced_items()
    {
        $this->beConstructedWith([
            "item_a" => [
                "size" => 1,
                "children" => [
                    "b" => "item_b",
                ]
            ],
            "item_b" => [
                "size" => 2,
                "children" => [
                    "c" => "item_c",
                ]
            ],
            "item_c" => [
                "size" => 3,
                "children" => [
                    "a" => "item_a",
                ]
            ]
        ]);

        $this->calculateInclusiveSize()->shouldReturn([
            "item_a" => [
                "size" => 1,
                "children" => [
                    "b" => "item_b",
                ],
                "inclusive_size" => 6
            ],
            "item_b" => [
                "size" => 2,
                "children" => [
                    "c" => "item_c",
                ],
                "inclusive_size" => 6
            ],
            "item_c" => [
                "size" => 3,
                "children" => [
                    "a" => "item_a",
                ],
                "inclusive_size" => 6
            ]
        ]);
    }

    function it_computes_inclusive_size_for_indirectly_circular_referenced_items_and_with_children_items()
    {
        $this->beConstructedWith([
            "item_a" => [
                "size" => 1,
                "children" => [
                    "b" => "item_b",
                ]
            ],
            "item_b" => [
                "size" => 2,
                "children" => [
                    "c" => "item_c",
                    "d" => "item_d"
                ]
            ],
            "item_c" => [
                "size" => 3,
                "children" => [
                    "a" => "item_a",
                ]
            ],
            "item_d" => [
                "size" => 4
            ]
        ]);

        $this->calculateInclusiveSize()->shouldReturn([
            "item_a" => [
                "size" => 1,
                "children" => [
                    "b" => "item_b",
                ],
                "inclusive_size" => 10
            ],
            "item_b" => [
                "size" => 2,
                "children" => [
                    "c" => "item_c",
                    "d" => "item_d"
                ],
                "inclusive_size" => 10
            ],
            "item_c" => [
                "size" => 3,
                "children" => [
                    "a" => "item_a",
                ],
                "inclusive_size" => 10
            ],
            "item_d" => [
                "size" => 4,
                "inclusive_size" => 4
            ]
        ]);
    }
}
