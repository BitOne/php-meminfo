<?php

namespace BitOne\PhpMemInfo\Console\Command;

use BitOne\PhpMemInfo\Analyzer\TopRanges;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Helper\Table;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Finder\Finder;

class TopRangesCommand extends Command
{
    /**
     * {@inheritedDoc}.
     */
    protected function configure()
    {
        $this
            ->setName('top-ranges')
            ->setDescription('Given a directory of dump files sorted by name, this command displays the top ranges (difference between the largest and smallest values) of the dumped types.')
            ->addArgument(
                'dump-dir',
                InputArgument::REQUIRED,
                'PHP Meminfo Dump Files directory'
            )
            ->addOption('show-zero', null, InputOption::VALUE_NONE, 'Show zero ranges')
        ;
    }

    /**
     * {@inheritdoc}
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $dumpDir = $input->getArgument('dump-dir');

        if (!file_exists($dumpDir)) {
            throw new \InvalidArgumentException(sprintf('Directory %s does not exist.', $dumpDir));
        }

        $sortedDumpFiles = Finder::create()->files()->in($dumpDir)->name('*.json')->sortByName(true);
        $output->writeln(sprintf('<info>%d dump files found.</info>', $sortedDumpFiles->count()));

        $topRanges = new TopRanges();
        $topRangeByType = $topRanges->get($sortedDumpFiles, (bool) $input->getOption('show-zero'));

        $table = new Table($output);

        $table->setHeaders(['Type', 'Range']);

        foreach ($topRangeByType as $type => $range) {
            $table->addRow([$type, $range]);
        }

        $table->render();

        return Command::SUCCESS;
    }
}
