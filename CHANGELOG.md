# 1.1.1 PHP Meminfo v1.1.1 (2021-02-17)
Fixes segfault on symbol table not properly rebuilt.

# 1.1 PHP Meminfo v1.1 (2021-02-15)
This version drops support for PHP 5.5 and adds official supports for PHP 7.4.

Thanks a lot to @tony2001, @WyriHaximus, @vudaltsov and @ahocquard for their contibutions on making PHP Meminfo compatible with PHP .7.4!

# 1.0.4 Ygramul (2018-08-13)
 - [GITHUB-75](https://github.com/BitOne/php-meminfo/issues/75) Now goes through static members of classes as well the usual frame execution browsing.

# 1.0.3 Ygramul (2018-08-13)
 - [GITHUB-76](https://github.com/BitOne/php-meminfo/issues/76) Fixes wrong non matching item reference from children, introduced in v1.0.2

# 1.0.2 Ygramul (2018-08-08)

## Bug fixes
 - [GITHUB-70](https://github.com/BitOne/php-meminfo/issues/70) Fixes duplication of scalars in PHP 7 when multiple references are used
 - [GITHUB-68](https://github.com/BitOne/php-meminfo/issues/68) Fixes duplication of objects in PHP 7 when multiple references are used
 - [GITHUB-67](https://github.com/BitOne/php-meminfo/issues/67) Fixes wrong starting frame in PHP 7, by @mathieuk
 - [GITHUB-61](https://github.com/BitOne/php-meminfo/issues/61) Removes now obsolete Brew installation instructions

## Improvement
 - [GITHUB-73](https://github.com/BitOne/php-meminfo/issues/73) Adds explanation on memory usage difference, between what is used by objects and what is reported

# 1.0.1 Ygramul (2017-11-24)

## Bug fixes
 - [GITHUB-49](https://github.com/BitOne/php-meminfo/issues/49) Properly escape control character from array index

## Improvements
 - [GITHUB-48](https://github.com/BitOne/php-meminfo/issues/48) Adds information about Brew installation on MacOS X, by @jdecool
 - [GITHUB-47](https://github.com/BitOne/php-meminfo/issues/47) Fixes on version support, by @jdecool

# 1.0.0 Ygramul (2017-11-16)

## Improvements
 - [GITHUB-45](https://github.com/BitOne/php-meminfo/issues/45) Adds PHP 7 support
 - keeps only the dump method in the extension. From the data gathered by it, all analysis can be done.
 - summary is now done as a analyzer
 - doc separation between "usage" and "how to hunt down memory leaks"
 - tests + travis

# 0.3.0 (2016-03-23)

## Improvements
 - Remove UI
 - Adds frame and symbol information
 - Adds analyzers to find path that kept references

# 0.2.0 (2015-04-30)

## Improvements
 - Adds an feature to display instances count summary by class.

# 0.1.0 (2014-02-15)

## Improvements
 - Migrating from Google Code to GitHub
 - ADDS ABility to display a list of object with Class name and handle
