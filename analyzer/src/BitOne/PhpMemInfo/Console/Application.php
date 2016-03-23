<?php

namespace BitOne\PhpMemInfo\Console;

use BitOne\PhpMemInfo\Console\Command\QueryCommand;
use BitOne\PhpMemInfo\Console\Command\ReferencePathCommand;
use Symfony\Component\Console\Application as BaseApplication;

/**
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class Application extends BaseApplication
{
    public function __construct()
    {
        parent::__construct('PHP Meminfo Dump Analyzer', '0.1');

        $this->add(new QueryCommand());
        $this->add(new ReferencePathCommand());
    }
}
