#!/usr/bin/env bash
set -e

cd analyzer

# Prepare buld dir
rm -rf .build
mkdir .build

# Install app deps
rm -rf vendor
composer install --no-dev

# Install Box
curl -L -o .build/box.phar https://github.com/box-project/box/releases/download/3.9.0/box.phar

# Compile Phar
php --define phar.read_only=0 .build/box.phar build

# Create signature
sha256sum .build/meminfo.phar > .build/meminfo.phar.sha256
