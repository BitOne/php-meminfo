<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Analyzer\QueryExecutor;
use BitOne\PhpMemInfo\Loader;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Helper\TableSeparator;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

/**
 * Command to list items based on query.
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class QueryCommand extends Command
{
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('query')
            ->setDescription('Basic querying on memory dump')
            ->addArgument(
                'dump-file',
                InputArgument::REQUIRED,
                'PHP Meminfo Dump File in JSON format'
            )
            ->addOption(
                'filters',
                'f',
                InputOption::VALUE_REQUIRED | InputOption::VALUE_IS_ARRAY,
                'Filter on an attribute. Operators: =, ~. Example: class~User'
            )
            ->addOption(
                'limit',
                'l',
                InputOption::VALUE_REQUIRED,
                'Number of results limit (default 10).'
            );
    }

    /**
     * {@inheritdoc}
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $dumpFilename = $input->getArgument('dump-file');

        $filters = [];
        $limit = 10;

        if (null !== $input->getOption('filters')) {
            $rawFilters = $input->getOption('filters');
            $filters = $this->convertFilters($rawFilters);
        }

        if (null !== $input->getOption('limit')) {
            $limit = $input->getOption('limit');
        }

        $loader = new Loader();

        $items = $loader->load($dumpFilename);

        $queryExecutor = new QueryExecutor($items);

        $results = $queryExecutor->executeQuery($filters, $limit);

        $table = new Table($output);
        if ($input->hasOption('verbose') && $input->getOption('verbose')) {
            $this->formatDetailedTable($results, $table);
        } else {
            $this->formatSimpleTable($results, $table);
        }

        $table->render();

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
     * Format data into a simple table with only item ids.
     *
     * @param array $items
     * @param Table $table
     */
    protected function formatSimpleTable(array $items, Table $table)
    {
        $table->setHeaders(['Item id']);

        $rows = [];

        foreach (array_keys($items) as $itemId) {
            $rows[] = [$itemId];
        }

        $table->setRows($rows);
    }

    /**
     * Format data into a detailed table.
     *
     * @param array $items
     * @param Table $table
     */
    protected function formatDetailedTable(array $items, Table $table)
    {
        $formatter = $this->getHelper('formatter');

        $table->setHeaders(['Item ids', 'Item data', 'Children']);

        $rows = [];

        foreach ($items as $itemId => $itemData) {
            $data = 'Type: '.$itemData['type']."\n";
            $children = '';

            if ('object' === $itemData['type']) {
                $data .= 'Class: '.$itemData['class']."\n";
                $data .= 'Object Handle: '.$itemData['object_handle']."\n";
            }

            $data .= 'Size: '.$formatter->formatMemory($itemData['size'])."\n";

            if ($itemData['is_root']) {
                $data .= "Is root: Yes\n";
                $data .= 'Execution Frame: '.$itemData['frame']."\n";
                $data .= 'Symbol Name: '.$itemData['symbol_name'];
            } else {
                $data .= 'Is root: No';
            }

            if (isset($itemData['children'])) {
                $childrenData = [];
                foreach ($itemData['children'] as $attribute => $childId) {
                    $childrenData[] = $attribute.': '.$childId;
                }
                $children = implode("\n", $childrenData);
            }

            $rows[] = [$itemId, $data, $children];
            $rows[] = new TableSeparator();
        }

        array_pop($rows);

        $table->setRows($rows);
    }
}
