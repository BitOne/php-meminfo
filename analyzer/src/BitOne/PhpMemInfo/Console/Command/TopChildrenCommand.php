<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Loader;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Input\InputOption;
use \BitOne\PhpMemInfo\Analyzer\TopSearcher;

/**
 * Command to list of objects with the largest number of children
 *
 * @author    Petr Petrenko <mail@avallac.ru>
 * @copyright 2019 Petr Petrenko
 * @license   http://opensource.org/licenses/MIT MIT
 */
class TopChildrenCommand extends Command
{
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('top-children')
            ->setDescription('Display a list of objects with the largest number of children')
            ->addArgument(
                'dump-file',
                InputArgument::REQUIRED,
                'PHP Meminfo Dump File in JSON format'
            )
            ->addOption(
                'limit',
                'l',
                InputOption::VALUE_OPTIONAL,
                'limit',
                5
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

        $topArrayChildrenSearcher = new TopSearcher($items);

        $top = $topArrayChildrenSearcher->createTopChildren($input->getOption('limit'));

        $table = new Table($output);
        $this->formatTable($top, $table);

        $table->render();

        return 0;
    }

    /**
     * Format data into a detailed table.
     *
     * @param array $top
     * @param Table $table
     */
    protected function formatTable(array $top, Table $table)
    {
        $table->setHeaders(['Num', 'Item ids', 'Children']);

        $rows = [];
        $num = 1;

        foreach($top as $ref => $children) {
            $rows[] = [$num ++, $ref, $children];
        }

        $table->setRows($rows);
    }
}
