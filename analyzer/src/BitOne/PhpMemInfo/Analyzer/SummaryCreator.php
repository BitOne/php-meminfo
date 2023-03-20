<?php

namespace BitOne\PhpMemInfo\Analyzer;

/**
 * Analyzer to generate a summary of all items (object and primitives)
 * by type
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2017 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class SummaryCreator
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
     * aggregated by type/class and sorted by count.
     *
     * @param array $sorters
     *
     * @return array
     */
    public function createSummary($sorters)
    {
        $summary = [];

        foreach ($this->items as $item) {
            $type = $item['type'];
            if ('object' === $type) {
                $type = $item['class'];
            }
            if (!isset($summary[$type])) {
                $summary[$type] = ['count' => 0, 'self_size' => 0];
            }
            $summary[$type]['count']++;
            $summary[$type]['self_size']+= $item['size'];
        }

        uasort($summary, function ($a, $b) use ($sorters) {
            $aCount = $a[$sorters['column']];
            $bCount = $b[$sorters['column']];

            if ($a === $b) {
                return 0;
            }

            $comp = $a[$sorters['column']] > $b[$sorters['column']];
            if ($sorters['order'] === 'asc') {
                $comp = $a[$sorters['column']] < $b[$sorters['column']];
            }

            if ($comp) {
                return -1;
            } else {
                return 1;
            }
        }
        );

        return $summary;
    }
}
