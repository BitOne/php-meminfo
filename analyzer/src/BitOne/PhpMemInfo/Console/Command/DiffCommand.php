<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Analyzer\SummaryDiffer;
use BitOne\PhpMemInfo\Loader;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Exception\InvalidArgumentException;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;


/**
 * Command to list the differences between two memory dumps.
 *
 * @author  oliver de Cramer <oliverde8@gmail.com>
 * @author  Axel Ducret <axel.ducret@gmail.com>
 * @license   http://opensource.org/licenses/MIT MIT
 */
class DiffCommand extends Command
{
    const SORT_COUNT = 'c';
    const SORT_SIZE = 's';
    const SORT_DIFF_COUNT = 'dc';
    const SORT_DIFF_SIZE = 'ds';

    /** @var array List of available sorts and the keys to use for the sorting. */
    protected $sorts = [
        self::SORT_COUNT => 1,
        self::SORT_SIZE => 2,
        self::SORT_DIFF_COUNT => 3,
        self::SORT_DIFF_SIZE => 4,
    ];

    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $sortDescription = "Define sorting order when displaying diff. Available options are : \n";
        $sortDescription .= "- c : Sort by count\n";
        $sortDescription .= "- s : Sort by size\n";
        $sortDescription .= "- dc : Sort by the count differene\n";
        $sortDescription .= "- ds : Sort by the size difference\n";

        $this
            ->setName('diff')
            ->setDescription('Compare 2 dump files')
            ->addArgument(
                'first-file',
                InputArgument::REQUIRED,
                'PHP Meminfo Dump File in JSON format'
            )
            ->addArgument(
                'second-file',
                InputArgument::REQUIRED,
                'PHP Meminfo Dump File in JSON format to compare first file with'
            )
            ->addOption(
                'sort',
                's',
                InputOption::VALUE_OPTIONAL,
                $sortDescription,
                self::SORT_DIFF_COUNT
            )
            ->addOption(
                'only-diff',
                'd',
                InputOption::VALUE_NONE,
                'If set, only the rows with actual differences will be showed'
            );
    }

    /**
     * {@inheritdoc}
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $firstFilename = $input->getArgument('first-file');
        $secondFilename = $input->getArgument('second-file');
        $sort = $input->getOption('sort');
        $onlyDiff = $input->getOption('only-diff');

        $loader = new Loader();

        $firstItems = $loader->load($firstFilename);
        $secondItems = $loader->load($secondFilename);

        $summaryDiffer = new SummaryDiffer($firstItems, $secondItems);
        $rows = $summaryDiffer->generateDiff();

        $table = new Table($output);
        $this->formatTable($rows, $table, $sort, $onlyDiff);

        $table->render();

        return 0;
    }

    /**
     * Format data into a detailed table.
     *
     * @param array  $rows
     * @param Table  $table
     * @param string $sort
     * @param bool   $onlyDiff
     */
    protected function formatTable(array $rows, Table $table, $sort, $onlyDiff)
    {
        $table->setHeaders(['Type', 'First Instances Count', 'First Cumulated Self Size (bytes)', 'Second Instances Diff', 'Second Size Diff (bytes)']);

        if ($onlyDiff) {
            $rows = array_filter($rows, function ($row) {
                return $row[3] !== '0';
            });
        }

        $this->sortTable($rows, $sort);

        $table->setRows($rows);
    }

    /**
     * Sort data for display.
     *
     * @param array $rows
     * @param       $sort
     */
    protected function sortTable(array &$rows, $sort)
    {
        if (!isset($this->sorts[$sort])) {
            throw new InvalidArgumentException("'$sort' is not a know sort parameter, see help for possible sort options");
        }

        if ($this->sorts[$sort] < 0) {
            return;
        }

        $sortIndex = $this->sorts[$sort];

        usort($rows, function($item1, $item2) use ($sortIndex) {
            return str_replace('+', '', $item1[$sortIndex]) <= str_replace('+', '', $item2[$sortIndex]);
        });
    }
}