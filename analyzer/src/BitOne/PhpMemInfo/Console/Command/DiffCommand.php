<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Loader;
use BitOne\PhpMemInfo\Analyzer\SummaryCreator;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Exception\InvalidArgumentException;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;


/**
 * Class DiffCommand
 *
 * @author  oliver de Cramer <oliverde8@gmail.com>
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

        $loader = new Loader();

        $firstItems = $loader->load($firstFilename);
        $secondItems = $loader->load($secondFilename);

        $firstSummary = new SummaryCreator($firstItems);
        $secondSummary = new SummaryCreator($secondItems);

        $firstSummary = $firstSummary->createSummary();
        $secondSummary = $secondSummary->createSummary();

        $table = new Table($output);
        $this->formatTable($firstSummary, $secondSummary, $table, $sort);

        $table->render();

        return 0;
    }

    /**
     * Format data into a detailed table.
     *
     * @param array  $firstSummary
     * @param array  $secondSummary
     * @param Table  $table
     * @param string $sort
     */
    protected function formatTable(array $firstSummary, array $secondSummary, Table $table, $sort)
    {
        $table->setHeaders(['Type', 'First Instances Count', 'First Cumulated Self Size (bytes)', 'Second Instances Diff', 'Second Size Diff (bytes)']);

        $rows = [];
        foreach($secondSummary as $type => $stats) {
            $countDiff = $stats['count'];
            $sizeDiff = $stats['self_size'];

            if (isset($firstSummary[$type])) {
                $countDiff = $stats['count'] - $firstSummary[$type]['count'];
                $sizeDiff = $stats['self_size'] - $firstSummary[$type]['self_size'];
            }

            $rows[] = [$type, $stats['count'], $stats['self_size'], $this->formatDiffValue($countDiff), $this->formatDiffValue($sizeDiff)];
        }

        // Let's not forget all elements completely removed from memory.
        foreach ($firstSummary as $type => $stats) {
            if (!isset($secondSummary[$type])) {
                $countDiff = -$stats['count'];
                $sizeDiff = -$stats['self_size'];
                $rows[] = [$type, $stats['count'], $stats['self_size'], $this->formatDiffValue($countDiff), $this->formatDiffValue($sizeDiff)];
            }
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
            return abs(str_replace('+', '', $item1[$sortIndex])) <= abs(str_replace('+', '', $item2[$sortIndex]));
        });
    }

    /**
     * Format diff value for display
     *
     * @param int $value
     *
     * @return string
     */
    protected function formatDiffValue($value)
    {
        if ($value > 0) {
            return '+' . $value;
        }
    }
}