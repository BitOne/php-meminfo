<?php

namespace BitOne\PhpMemInfo\Analyzer;

use Fhaculty\Graph\Graph;
use Graphp\Algorithms\ShortestPath\BreadthFirst;

/**
 * Analyzer to load the data as a graph and analyze the graph.
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class GraphBrowser
{
    /** @var array */
    protected $items;

    /** @var Graph */
    protected $graph;

    /**
     * @param array $items
     */
    public function __construct(array $items)
    {
        $this->items = $items;
    }

    /**
     * Find all references coming from roots.
     *
     * @param string $itemid
     *
     * @return array
     */
    public function findReferencePaths($itemId)
    {
        $graph = $this->getGraph();

        $to = $graph->getVertex($itemId);

        $paths = [];

        $mapSearch = new BreadthFirst($to);

        $map = $mapSearch->getEdgesMap();

        foreach ($map as $endVertexId => $path) {
            $endVertex = $graph->getVertex($endVertexId);
            $endVertexData = $endVertex->getAttribute('data');

            if ($endVertexData['is_root']) {
                $paths[$endVertexId] = $path;
            }
        }

        if ($to->getAttribute('data')['is_root']) {
            $edge = $to->createEdgeTo($to);
            $edge->setAttribute('name', '<self>');
            $paths[$to->getId()] = [$edge];
        }

        return $paths;
    }

    /**
     * Build the graph if necessary and return it.
     *
     * @param Graph
     */
    protected function getGraph()
    {
        if (null === $this->graph) {
            $this->buildGraph();
        }

        return $this->graph;
    }

    /**
     * Build the graph from the items.
     */
    protected function buildGraph()
    {
        $this->graph = new Graph();
        $this->createVertices();
        $this->createEdges();
    }

    /**
     * Create vertices on the graph from items.
     */
    protected function createVertices()
    {
        foreach ($this->items as $itemId => $itemData) {
            $vertex = $this->graph->createVertex($itemId);
            $vertex->setAttribute('data', $itemData);
        }
    }

    /**
     * Create edges on the graph between vertices.
     */
    protected function createEdges()
    {
        foreach ($this->items as $itemId => $itemData) {
            if (isset($itemData['children'])) {
                $children = $itemData['children'];
                $vertex = $this->graph->getVertex($itemId);

                foreach ($children as $link => $child) {
                    $childVertex = $this->graph->getVertex($child);
                    $childVertex->createEdgeTo($vertex)->setAttribute('name', $link);
                }
            }
        }
    }
}
