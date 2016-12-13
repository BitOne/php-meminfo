PHP_ARG_ENABLE(meminfo, whether to enable Memory Information support,
[ --enable-meminfo   Enable Meminfo support])

if test "$PHP_MEMINFO" = "yes"; then
  AC_DEFINE(HAVE_MEMINFO, 1, [Whether you have Memory Info])
  PHP_NEW_EXTENSION(meminfo, meminfo.c, $ext_shared)
fi
