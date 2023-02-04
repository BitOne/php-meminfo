<?php

namespace BitOne\PhpMemInfo\Analyzer;

use BitOne\PhpMemInfo\Loader;
use Symfony\Component\Finder\Finder;
use Symfony\Component\Finder\SplFileInfo;

class TopRanges
{
    public function get(Finder $sortedDumpFiles, bool $showZeroRange = true)
    {
        list($min, $max) = $this->getMinMax($sortedDumpFiles);

        $allTypes = [];
        foreach (array_keys($min) as $type) {
            $allTypes[$type] = $type;
        }
        foreach (array_keys($max) as $type) {
            $allTypes[$type] = $type;
        }

        $ranges = [];
        foreach ($allTypes as $type) {
            $minVal = $min[$type] ?? 0;
            $maxVal = $max[$type] ?? 0;

            $rangeVal = $maxVal - $minVal;

            if (($showZeroRange && 0 === $rangeVal) || $rangeVal > 0) {
                $ranges[$type] = $rangeVal;
            }
        }

        arsort($ranges, SORT_NUMERIC);

        return $ranges;
    }

    private function getTypeCount(array $items)
    {
        $typeCount = [];

        foreach ($items as $item) {
            $type = ('object' === $item['type']) ? $item['class'] : $item['type'];

            if (!array_key_exists($type, $typeCount)) {
                $typeCount[$type] = 0;
            }

            ++$typeCount[$type];
        }

        return $typeCount;
    }

    private function getMinMax(Finder $sortedDumpFiles)
    {
        $loader = new Loader();
        $min = [];
        $max = [];

        /** @var SplFileInfo $sortedDumpFile */
        foreach ($sortedDumpFiles as $sortedDumpFile) {
            $items = $loader->load($sortedDumpFile->getPathname());

            $typeCount = $this->getTypeCount($items);

            foreach ($typeCount as $type => $count) {
                if (!array_key_exists($type, $min) || $count < $min[$type]) {
                    $min[$type] = $count;
                }

                if (!array_key_exists($type, $max) || $count > $max[$type]) {
                    $max[$type] = $count;
                }
            }
        }

        return [$min, $max];
    }
}
