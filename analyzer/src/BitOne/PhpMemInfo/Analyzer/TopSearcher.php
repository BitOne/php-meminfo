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
        $top = [];
        for($i = 0; $i < $limit; $i ++) {
            $top[] = 0;
        }
        foreach ($this->items as $key => $item) {
            if (isset($item['children'])) {
                $links = count($item['children']);
                for ($i = 0; $i < $limit + 1; $i++) {
                    if ($i === $limit || $top[$i]['links'] >= $links) {
                        if ($i !== 0) {
                            $top[$i - 1] = [
                                'links' => $links,
                                'ref' => $key
                            ];
                        }
                        break;
                    } elseif ($i !== 0) {
                        $top[$i - 1] = $top[$i];
                    }
                }
            }
        }

        return $top;
    }

    /**
     * Create a list of the largest objects
     *
     * @var int $limit
     * @return array
     */
    public function createTopSize($limit)
    {
        $top = [];
        for($i = 0; $i < $limit; $i ++) {
            $top[] = 0;
        }
        foreach ($this->items as $key => $item) {
            if (isset($item['size'])) {
                for ($i = 0; $i < $limit + 1; $i++) {
                    if ($i === $limit || $top[$i]['size'] >= $item['size']) {
                        if ($i !== 0) {
                            $top[$i - 1] = [
                                'size' => $item['size'],
                                'ref' => $key
                            ];
                        }
                        break;
                    } elseif ($i !== 0) {
                        $top[$i - 1] = $top[$i];
                    }
                }
            }
        }

        return $top;
    }
}