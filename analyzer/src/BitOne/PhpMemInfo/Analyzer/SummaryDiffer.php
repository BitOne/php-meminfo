<?php

namespace BitOne\PhpMemInfo\Analyzer;

/**
 * Analyzer to generate the differences between two memory dumps
 *
 * @author  oliver de Cramer <oliverde8@gmail.com>
 * @author  Axel Ducret <axel.ducret@gmail.com>
 * @license   http://opensource.org/licenses/MIT MIT
 */
class SummaryDiffer
{
    /** @var array */
    protected $firstItems;

    /** @var array */
    protected $secondItems;

    /**
     * @param array $firstItems
     * @param array $secondItems
     */
    public function __construct(array $firstItems, array $secondItems)
    {
        $this->firstItems = $firstItems;
        $this->secondItems = $secondItems;
    }

    /**
     * @return array
     */
    public function generateDiff()
    {
        $firstSummary = $this->getSummaryForItems($this->firstItems);
        $secondSummary = $this->getSummaryForItems($this->secondItems);
        $rows = [];

        foreach($secondSummary as $type => $stats) {
            $firstSummaryCount = 0;
            $firstSummarySize = 0;

            if (isset($firstSummary[$type])) {
                $firstSummaryCount = $firstSummary[$type]['count'];
                $firstSummarySize = $firstSummary[$type]['self_size'];
                unset($firstSummary[$type]);
            }

            $countDiff = $stats['count'] - $firstSummaryCount;
            $sizeDiff = $stats['self_size'] - $firstSummarySize;

            $rows[] = [
                $type,
                $firstSummaryCount,
                $firstSummarySize,
                $this->formatDiffValue($countDiff),
                $this->formatDiffValue($sizeDiff)
            ];
        }

        // Let's not forget all elements completely removed from memory.
        foreach ($firstSummary as $type => $stats) {
            $countDiff = -$stats['count'];
            $sizeDiff = -$stats['self_size'];

            $rows[] = [
                $type,
                $stats['count'],
                $stats['self_size'],
                $this->formatDiffValue($countDiff),
                $this->formatDiffValue($sizeDiff)
            ];
        }

        return $rows;
    }

    /**
     * @param array $items
     * @return array
     */
    protected function getSummaryForItems(array $items)
    {
        $summaryCreator = new SummaryCreator($items);

        return $summaryCreator->createSummary();
    }

    /**
     * Format diff value for display
     *
     * @param int $value
     *
     * @return string
     */
    protected function formatDiffValue($value)
    {
        if ($value > 0) {
            return '+' . $value;
        }

        return strval($value);
    }
}
