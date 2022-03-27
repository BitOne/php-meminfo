<?php

namespace BitOne\PhpMemInfo\Analyzer;

class StdClassShapesCollector
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
     * Create a summary from the existing items,
     * aggregated by member list and sorted by count.
     *
     * @return array
     */
    public function collect()
    {
        $summary = [];

        foreach ($this->items as $item) {
            if ( $item['type'] !== 'object' || $item['class'] !== 'stdClass') {
                continue;
            }
            $shape = implode(',', array_keys($item['children'] ?? []));
            if (!isset($summary[$shape])) {
                $summary[$shape] = ['count' => 0, 'self_size' => 0];
            }
            $summary[$shape]['count']++;
            $summary[$shape]['self_size']+= $item['size'];
        }

        uasort($summary, function ($a, $b) {
            if ($a === $b) {
                return 0;
            }
            if ($a['count'] > $b['count']) {
                return -1;
            } else {
                return 1;
            }
        }
        );

        return $summary;
    }
}
