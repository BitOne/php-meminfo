<?php

namespace BitOne\PhpMemInfo\Analyzer;

class TopSearcher
{
    /** @var array */
    protected $items;

    /**
     * @param array $items
     */
    public function __construct(array $items)
    {
        $this->items = $items;
    }

    /**
     * Create a list of objects with the largest number of children
     *
     * @var int $limit
     * @return array
     */
    public function createTopChildren($limit)
    {
        $index = $top = [];

        foreach ($this->items as $key => $item) {
            if (isset($item['children'])) {
                $index[$key] = count($item['children']);
            }
        }

        arsort($index);

        return array_slice($index, 0, $limit);
    }
}
