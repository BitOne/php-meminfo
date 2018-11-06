<?php

namespace BitOne\PhpMemInfo\Analyzer;

use Fhaculty\Graph\Graph;
use Graphp\Algorithms\Search\BreadthFirst;

/**
 * Analyzer to compute the inclusive size of items, i.e. the
 * sum of the size of the item itself and all its descendants
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2018 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class InclusiveSizeCalculator
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
     * Analyzer to calculate the inclusive size of items, i.e. the
     * sum of the size of the item itself and all its descendants
     *
     * @return array
     */
    public function calculateInclusiveSize()
    {
        $items = [];

        $items = $this->items;

        $graph = $this->buildGraph($this->items);

        foreach ($items as $itemId => $itemData) {
            $inclusiveSize = 0;

            if (isset($itemData['children'])) {

                $search = new BreadthFirst($graph->getVertex($itemId));
                $reachableVertices = $search->getVertices();

                foreach ($reachableVertices as $reachableVertex) {
                    $inclusiveSize += $reachableVertex->getAttribute('size');
                }
            } else {
                $inclusiveSize = $itemData['size'];
            }
            $items[$itemId]['inclusive_size'] = $inclusiveSize;
        }

        return $items;
    }

    /**
     * Build the graph from the items.
     */
    protected function buildGraph(array $items)
    {
        $graph = new Graph();
        $this->createVertices($graph, $items);
        $this->createEdges($graph, $items);

        return $graph;
    }

    /**
     * Create vertices on the graph from items.
     */
    protected function createVertices($graph, array $items)
    {
        foreach ($items as $itemId => $itemData) {
            $vertex = $graph->createVertex($itemId);
            $vertex->setAttribute('size', $itemData['size']);
        }
    }

    /**
     * Create edges on the graph between vertices.
     */
    protected function createEdges($graph, array $items)
    {
        foreach ($items as $itemId => $itemData) {
            if (isset($itemData['children'])) {
                $parentVertex = $graph->getVertex($itemId);
                $children = $itemData['children'];

                foreach ($children as $link => $child) {
                    $childVertex = $graph->getVertex($child);
                    $parentVertex->createEdgeTo($childVertex)->setAttribute('name', $link);
                }
            }
        }
    }

}
