<?php

namespace BitOne\PhpMemInfo\Analyzer;

/**
 * Analyzer to do simple query on the data.
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class QueryExecutor
{
    /** @var array */
    protected $items;

    /**
     * @param array $items
     */
    public function __construct(array $items)
    {
        $this->items = $items;

        foreach ($this->items as $itemid => &$itemData) {
            $itemData['id'] = $itemid;
        }
    }

    /**
     * Query the existing items.
     *
     * @param array $filters
     * @param int   $limit
     *
     * @return array
     */
    public function executeQuery(array $filters, $limit = 10)
    {
        $results = [];

        foreach ($this->items as $itemId => $itemData) {
            if ($this->matches($itemData, $filters)) {
                $results[$itemId] = $itemData;
            }

            if (count($results) >= $limit) {
                break;
            }
        }

        return $results;
    }

    /**
     * Apply filters on an item and return if it is a match or not.
     *
     * @param array $item
     * @param array $filters
     *
     * @return bool
     */
    public function matches(array $item, array $filters)
    {
        $matches = true;

        foreach ($filters as $attribute => $filter) {
            $operator = $filter['operator'];
            $value = $filter['value'];

            if (!isset($item[$attribute])) {
                $matches = false;
            } else {
                switch ($operator) {
                    case '=':
                        $matches = ($item[$attribute] == $value);
                        break;
                    case '~':
                        $pattern = sprintf('#%s#', $value);
                        $regexpResult = preg_match($pattern, $item[$attribute]);
                        if (false === $regexpResult) {
                            throw new \InvalidArgumentException("Provided regexp is invalid: $value");
                        }
                        $matches = (1 === $regexpResult);
                        break;
                }
            }

            if (!$matches) {
                break;
            }
        }

        return $matches;
    }
}
