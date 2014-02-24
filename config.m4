dnl $Id$
dnl config.m4 for extension gqtpcli

PHP_ARG_ENABLE(gqtpcli, whether to enable gqtpcli support,
dnl Make sure that the comment is aligned:
[  --enable-gqtpcli           Enable gqtpcli support])

if test "$PHP_GQTPCLI" != "no"; then
  # --with-gqtpcli -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/groonga/groonga.h"  # you most likely want to change this
  if test -r $PHP_GQTPCLI/$SEARCH_FOR; then # path given as parameter
    GQTPCLI_DIR=$PHP_GQTPCLI
  else # search default path list
    AC_MSG_CHECKING([for gqtpcli files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        GQTPCLI_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$GQTPCLI_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the gqtpcli distribution])
  fi

  # --with-gqtpcli -> add include path
  PHP_ADD_INCLUDE($GQTPCLI_DIR/include/groonga)

  # --with-gqtpcli -> check for lib and symbol presence
  LIBNAME=groonga # you may want to change this
  LIBSYMBOL=grn_init # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $GQTPCLI_DIR/lib, GQTPCLI_SHARED_LIBADD)
    AC_DEFINE(HAVE_GQTPCLILIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong gqtpcli lib version or lib not found])
  ],[
    -L$GQTPCLI_DIR/lib -lm
  ])

  PHP_SUBST(GQTPCLI_SHARED_LIBADD)

  PHP_NEW_EXTENSION(gqtpcli, gqtpcli.c, $ext_shared)
fi
