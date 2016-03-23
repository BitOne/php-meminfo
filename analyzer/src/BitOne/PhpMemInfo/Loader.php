<?php

namespace BitOne\PhpMemInfo;

use Symfony\Component\Serializer\Encoder\JsonEncoder;
use Symfony\Component\Filesystem\Filesystem;

/**
 * Load a dump file as an array.
 *
 * @author    Benoit Jacquemont <benoit.jacquemont@gmail.com>
 * @copyright 2016 Benoit Jacquemont
 * @license   http://opensource.org/licenses/MIT MIT
 */
class Loader
{
    /**
     * Load the items from a file and return them.
     *
     * @param string $filename
     *
     * @return array
     */
    public function load($filename)
    {
        $filesystem = new FileSystem();

        if (!$filesystem->exists($filename)) {
            throw new \InvalidArgumentException(
                sprintf('Provided filename is not accessible: %s', $filename)
            );
        }

        $jsonEncoder = new JsonEncoder();

        $allData = $jsonEncoder->decode(file_get_contents($filename), JsonEncoder::FORMAT);

        return $allData['items'];
    }
}
