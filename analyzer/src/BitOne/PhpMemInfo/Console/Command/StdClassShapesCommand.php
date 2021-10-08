<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Analyzer\StdClassShapesCollector;
use BitOne\PhpMemInfo\Loader;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

class StdClassShapesCommand extends Command {
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('stdclass-shapes')
            ->setDescription('Show statistics on stdClass property names')
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
        $shapes = (new StdClassShapesCollector($items))->collect();
        $table = new Table($output);
        $this->formatTable($shapes, $table);
        $table->render();
        return 0;
    }

    /**
     * Format data into a detailed table.
     *
     * @param array $shapes
     * @param Table $table
     */
    protected function formatTable(array $shapes, Table $table)
    {
        $table->setHeaders(['Members', 'Instances Count', 'Cumulated Self Size (bytes)']);

        $rows = [];

        foreach($shapes as $members => $stats) {
            $rows[] = [$members, $stats['count'], $stats['self_size']];
        }

        $table->setRows($rows);
    }

}
