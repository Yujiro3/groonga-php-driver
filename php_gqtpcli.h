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

#ifndef PHP_GQTPCLI_H
#define PHP_GQTPCLI_H

#define REGISTER_CLASS_CONST_LONG(pce, const_name, value) zend_declare_class_constant_long(pce, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);
#define REGISTER_CLASS_CONST_STRING(pce, const_name, value) zend_declare_class_constant_stringl(pce, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);

extern zend_module_entry gqtpcli_module_entry;
#define phpext_gqtpcli_ptr &gqtpcli_module_entry

#define PHP_GQTPCLI_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#   define PHP_GQTPCLI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_GQTPCLI_API __attribute__ ((visibility("default")))
#else
#   define PHP_GQTPCLI_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(gqtpcli);
PHP_MSHUTDOWN_FUNCTION(gqtpcli);
PHP_RINIT_FUNCTION(gqtpcli);
PHP_RSHUTDOWN_FUNCTION(gqtpcli);
PHP_MINFO_FUNCTION(gqtpcli);

/**
 * GQTPcliクラス::メンバー変数定義
 */
typedef struct {
    zend_object std;
    grn_ctx *ctx;
    int connected;
    long flags;
    unsigned int qid;
} gqtpcli_t;

/**
 * GQTPcliクラス::メンバー関数定義
 */
PHP_METHOD(GQTPcli, __construct);
PHP_METHOD(GQTPcli, __destruct);
PHP_METHOD(GQTPcli, connect);
PHP_METHOD(GQTPcli, close);
PHP_METHOD(GQTPcli, send);
PHP_METHOD(GQTPcli, recv);

PHP_METHOD(GQTPcli, getFlags);
PHP_METHOD(GQTPcli, getQueryID);
PHP_METHOD(GQTPcli, getErrorMessage);
PHP_METHOD(GQTPcli, query);


#ifdef ZTS
#define GQTPCLI_G(v) TSRMG(gqtpcli_globals_id, zend_gqtpcli_globals *, v)
#else
#define GQTPCLI_G(v) (gqtpcli_globals.v)
#endif

#endif    /* PHP_GQTPCLI_H */

