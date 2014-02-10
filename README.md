MEMINFO
=======

Compatibility
-------------
 - compiled and tested on PHP 5.4.4 (Debian)

Compilation instructions
------------------------
MemInfo can be compiled outside PHP itself.

You will need the phpize function. It can be installed on a Debian base system by:
apt-get install php5-dev

Once you have this function, follow this steps:

1. Go to the root of the MemInfo source files
2. phpize
3. ./configure --enable-meminfo
4. make
5. cp modules/meminfo.so /path/to/your/extensions/folder
6. add the following line to your php.ini or to a custom file inside /etc/php5/conf.d/ for Debian based system. extension=meminfo.sp
7. restart your webserver
8. check the PHP Info output and look for the MemInfo data. If you can find it, installation has been successful.
To see the PHP Info output, just create a page calling the phpinfo(); function, a load it from your browser, or call php -i from command line.

Usage

Todo


Credits
-------
Thanks to Derick Rethans on his inspiritionnal work on the essential XDebug. See http://www.xdebug.org/
