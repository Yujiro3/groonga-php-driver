/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2011-2014 sheeps.me
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @package         groonga-php-driver
 * @copyright       Copyright (c) 2011-2014 sheeps.me
 * @author          Yujiro Takahashi <yujiro3@gmail.com>
 * @filesource
 */

#ifndef PHP_GROONGA_H
#define PHP_GROONGA_H

extern zend_module_entry groonga_module_entry;
#define phpext_groonga_ptr &groonga_module_entry

#define PHP_GROONGA_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#   define PHP_GROONGA_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_GROONGA_API __attribute__ ((visibility("default")))
#else
#   define PHP_GROONGA_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(groonga);
PHP_MSHUTDOWN_FUNCTION(groonga);
PHP_RINIT_FUNCTION(groonga);
PHP_RSHUTDOWN_FUNCTION(groonga);
PHP_MINFO_FUNCTION(groonga);

/**
 * Groongaクラス::メンバー変数定義
 */
typedef struct {
    zend_object std;
    grn_ctx *ctx;
    int connected;
} groonga_t;

/**
 * Groongaクラス::メンバー関数定義
 */
PHP_METHOD(Groonga, __construct);
PHP_METHOD(Groonga, __destruct);
PHP_METHOD(Groonga, connect);
PHP_METHOD(Groonga, close);
PHP_METHOD(Groonga, send);
PHP_METHOD(Groonga, recv);

PHP_METHOD(Groonga, query);


/* 
    Declare any global variables you may need between the BEGIN
    and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(groonga)
    long  global_value;
    char *global_string;
ZEND_END_MODULE_GLOBALS(groonga)
*/

/* In every utility function you add that needs to use variables 
   in php_groonga_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as GROONGA_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define GROONGA_G(v) TSRMG(groonga_globals_id, zend_groonga_globals *, v)
#else
#define GROONGA_G(v) (groonga_globals.v)
#endif

#endif    /* PHP_GROONGA_H */

