<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Analyzer\GraphBrowser;
use BitOne\PhpMemInfo\Loader;
use Fhaculty\Graph\Vertex;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

/**
 * Command to find reference path to a specific item.
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class ReferencePathCommand extends Command
{
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('ref-path')
            ->setDescription('Find reference paths to an item')
            ->addArgument(
                'item-id',
                InputArgument::REQUIRED,
                'Item Id in 0xaaaaaaaa format'
            )
            ->addArgument(
                'dump-file',
                InputArgument::REQUIRED,
                'PHP Meminfo Dump File in JSON format'
            );
    }

    /**
     * {@inheritdoc}
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $dumpFilename = $input->getArgument('dump-file');
        $itemId = $input->getArgument('item-id');

        $loader = new Loader();

        $items = $loader->load($dumpFilename);

        $graphBrowser = new GraphBrowser($items);

        $paths = $graphBrowser->findReferencePaths($itemId);

        $output->writeln(sprintf('<info>Found %d paths</info>', count($paths)));
        foreach ($paths as $vertexId => $path) {

            if ($input->hasOption('verbose') && $input->getOption('verbose')) {
                $output->writeln("<info>Path from $vertexId</info>");
                $this->renderDetailedPath($output, $path);
            } else {
                $output->writeln("<info>Path to $vertexId</info>");
                $this->renderSimplePath($output, $path);
            }
        }

        return 0;
    }

    /**
     * Convert raw filters provided by the user into structured filter.
     *
     * @param array $rawFilters
     *
     * @return array
     */
    protected function convertFilters(array $rawFilters)
    {
        $filters = [];

        foreach ($rawFilters as $rawFilter) {
            $filterParts = [];
            preg_match('/^([^=~]+)([=~])(.+)$/', $rawFilter, $filterParts);

            if (count($filterParts) !== 4) {
                throw new \InvalidArgumentException(
                    sprintf('Provided filter is invalid:%s', $rawFilter)
                );
            }

            $attribute = $filterParts[1];
            $operator = $filterParts[2];
            $value = $filterParts[3];

            $filters[$attribute] = ['operator' => $operator, 'value' => $value];
        }

        return $filters;
    }

    /**
     * Format a simple path and render it to the output.
     *
     * @param OutputInterface $output
     * @param Edge[]          $path
     */
    protected function renderSimplePath(OutputInterface $output, array $path)
    {
        $pathFromRoot = array_reverse($path);

        $indent = 0;

        foreach ($pathFromRoot as $edge) {

            $vertex = $edge->getVertexEnd();

            $data = $vertex->getAttribute('data');

            if ($data['is_root'] === true) {
                $frame = $data['frame'];

                $output->write(sprintf('(%s)', $frame));

                if ("CLASS_STATIC_MEMBER" !== $frame) {
                    $output->write('$');
                }
                $output->write($data['symbol_name']);
            }

            if ($data['type'] == 'object') {
                $output->writeln('');
                $output->write(str_repeat(' ', $indent));
                $output->write('->');
                $output->write($edge->getAttribute('name'));
            } elseif ($data['type'] == 'array') {
                $output->write(sprintf('["%s"]', $edge->getAttribute('name')));
            }
            $indent +=2;
        }
        $output->writeln('');

    }

    /**
     * Format a detailed path and render it to the output.
     *
     * @param OutputInterface $output
     * @param Edge[]          $path
     */
    protected function renderDetailedPath(OutputInterface $output, array $path)
    {
        $linkPadding = 20;

        foreach ($path as $edge) {
            $table = new Table($output);
            $vertexData = $this->prepareVertexData($edge->getVertexStart());
            $table->setRows($vertexData);
            $table->render();

            $output->writeln(str_pad('^', $linkPadding, ' ', STR_PAD_BOTH));
            $output->writeln(str_pad('|', $linkPadding, ' ', STR_PAD_BOTH));
            $output->writeln(str_pad($edge->getAttribute('name'), $linkPadding, ' ', STR_PAD_BOTH));
            $output->writeln(str_pad('|', $linkPadding, ' ', STR_PAD_BOTH));
            $output->writeln(str_pad('|', $linkPadding, ' ', STR_PAD_BOTH));
        }
        $table = new Table($output);
        $table->setRows($this->prepareVertexData($edge->getVertexEnd()));
        $table->render();
    }

    /**
     * Prepare an array with a vertex data.
     *
     * @param Vertex $vertex
     *
     * @return array
     */
    protected function prepareVertexData(Vertex $vertex)
    {
        $formatter = $this->getHelper('formatter');

        $vertexData = $vertex->getAttribute('data');
        $data = [];
        $data[] = ['Id: '.$vertex->getId()];
        $data[] = ['Type: '.$vertexData['type']];

        if ('object' === $vertexData['type']) {
            $data[] = ['Class: '.$vertexData['class']];
            $data[] = ['Object Handle: '.$vertexData['object_handle']];
        }

        $data[] = ['Size: '.$formatter->formatMemory($vertexData['size'])];

        if ($vertexData['is_root']) {
            $data[] = ['Is root: Yes'];
            $data[] = ['Execution Frame: '.$vertexData['frame']];
            $data[] = ['Symbol Name: '.$vertexData['symbol_name']];
        } else {
            $data[] = ['Is root: No'];
        }

        if (isset($vertexData['children'])) {
            $data[] = ['Children count: '.count($vertexData['children'])];
        }

        return $data;
    }

    /**
     * Get the widest string length from an array.
     *
     * @param array $input
     *
     * @return int
     */
    protected function getMaxStringLength(array $input)
    {
        return array_reduce($input, function ($result, $string) {
            return max($result, strlen($string[0]));
        });
    }
}
