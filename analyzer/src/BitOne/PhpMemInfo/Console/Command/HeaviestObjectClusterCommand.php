<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Loader;
use BitOne\PhpMemInfo\Analyzer\InclusiveSizeCalculator;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Helper\TableSeparator;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

/**
 * Command to list the root of the heaviest object cluster
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2018 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class HeaviestObjectClusterCommand extends Command
{
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('heaviest-cluster')
            ->setDescription('Display a list of the root of the heaviest cluster')
            ->addOption(
                'limit',
                'l',
                InputOption::VALUE_REQUIRED,
                'Number of results limit (default 10).'
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

        $limit = 10;

        if (null !== $input->getOption('limit')) {
            $limit = $input->getOption('limit');
        }

        $loader = new Loader();

        $items = $loader->load($dumpFilename);

        $calculator = new InclusiveSizeCalculator($items);

        $itemsWithSize = $calculator->calculateInclusiveSize();

        $topItems = $itemsWithSize;

        uasort($topItems, function ($a, $b) {
                $aInclusiveSize = $a['inclusive_size'];
                $bInclusiveSize = $b['inclusive_size'];

                if ($aInclusiveSize === $bInclusiveSize) {
                    return 0;
                }
                if ($aInclusiveSize > $bInclusiveSize) {
                    return -1;
                } else {
                    return 1;
                }
            }
        );

        $topItems = array_slice($topItems, 0, $limit);

        $table = new Table($output);
        $this->formatDetailedTable($topItems, $table);

        $table->render();

        return 0;
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
            $data .= 'Cluster Size: '.$formatter->formatMemory($itemData['inclusive_size'])."\n";

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
