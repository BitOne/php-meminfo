<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Loader;
use BitOne\PhpMemInfo\Analyzer\SummaryCreator;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Helper\TableSeparator;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

/**
 * Command to list a summary of items based on query.
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class SummaryCommand extends Command
{
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('summary')
            ->setDescription('Display a summary of items by type from a dump file')
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

        $loader = new Loader();

        $items = $loader->load($dumpFilename);

        $summaryCreator = new SummaryCreator($items);

        $summary = $summaryCreator->createSummary();

        $table = new Table($output);
        $this->formatTable($summary, $table);

        $table->render();

        return 0;
    }

    /**
     * Format data into a detailed table.
     *
     * @param array $summary
     * @param Table $table
     */
    protected function formatTable(array $summary, Table $table)
    {
        $formatter = $this->getHelper('formatter');

        $table->setHeaders(['Type', 'Instances Count', 'Cumulated Self Size (bytes)']);

        $rows = [];

        foreach($summary as $type => $stats) {
            $rows[] = [$type, $stats['count'], $stats['self_size']];
        }

        $table->setRows($rows);
    }
}
