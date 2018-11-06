<?php

namespace BitOne\PhpMeminfo\Analyzer\Graph;

use Graphp\Algorithms\Search\Base;
use Fhaculty\Graph\Set\Vertices;

/**
 * From Graphp\Algorithms\Search\DepthFirst
 *
 * See https://github.com/graphp/algorithms/blob/master/src/Search/DepthFirst.php
 */
class DepthFirstForClusterSize extends Base
{
    /**
     * calculates an iterative depth-first search but don't go through vertex
     * where cluster size has already been calculated.
     *
     * @return Vertices
     */
    public function getVertices()
    {
        $visited = array();
        $todo = array($this->vertex);
        while ($vertex = array_shift($todo)) {
            if (!isset($visited[$vertex->getId()])) {
                $visited[$vertex->getId()] = $vertex;

                if (null === $vertex->getAttribute('inclusive_size')) {
                    foreach (array_reverse($this->getVerticesAdjacent($vertex)->getMap(), true) as $vid => $nextVertex) {
                        $todo[] = $nextVertex;
                    }
                }
            }
        }

        return new Vertices($visited);
    }
}
