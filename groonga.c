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

#include <groonga/groonga.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_groonga.h"

/* If you declare any globals in php_groonga.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(groonga)
*/

/* True global resources - no need for thread safety here */
static int le_groonga;

/* Groonga初期化フラグ */
static int groonga_initialized = 0;

/* Groongaクラス構造体 */
zend_class_entry *groonga_ce;

/* {{{ groonga_functions[]
 *
 * Every user visible function must have an entry in groonga_functions[].
 */
const zend_function_entry groonga_functions[] = {
    PHP_ME(Groonga, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(Groonga, __destruct, NULL, ZEND_ACC_DTOR | ZEND_ACC_PUBLIC)
    PHP_ME(Groonga, connect, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Groonga, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Groonga, send, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Groonga, recv, NULL, ZEND_ACC_PUBLIC)

    PHP_ME(Groonga, query, NULL, ZEND_ACC_PUBLIC)

    PHP_FE_END    /* Must be the last line in groonga_functions[] */
};
/* }}} */

/* {{{ groonga_module_entry
 */
zend_module_entry groonga_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "groonga",
    groonga_functions,
    PHP_MINIT(groonga),
    PHP_MSHUTDOWN(groonga),
    PHP_RINIT(groonga),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(groonga),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(groonga),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_GROONGA_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_GROONGA
ZEND_GET_MODULE(groonga)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("groonga.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_groonga_globals, groonga_globals)
    STD_PHP_INI_ENTRY("groonga.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_groonga_globals, groonga_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_groonga_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_groonga_init_globals(zend_groonga_globals *groonga_globals)
{
    groonga_globals->global_value = 0;
    groonga_globals->global_string = NULL;
}
/* }}} */


/* {{{ php_groonga_free_storage
 */
static void php_groonga_free_storage(groonga_t *object TSRMLS_DC)
{
    zend_object_std_dtor(&object->std TSRMLS_CC);

    if (NULL != object->ctx) {
        grn_ctx_fin(object->ctx);
        efree(object->ctx);
    }
    object->ctx = NULL;

    efree(object);
}
/* }}} */


/* {{{ php_groonga_new
 */
zend_object_value php_groonga_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    groonga_t *object;
    zval *tmp;

    object = ecalloc(1, sizeof(*object));

    zend_object_std_init(&object->std, ce TSRMLS_CC);
    object_properties_init(&object->std, ce);
    rebuild_object_properties(&object->std);

    object->ctx = emalloc(sizeof(*object->ctx));
    object->connected = 0;

    retval.handle = zend_objects_store_put(
        object, 
        (zend_objects_store_dtor_t)zend_objects_destroy_object, 
        (zend_objects_free_object_storage_t)php_groonga_free_storage, 
        NULL TSRMLS_CC
    );
    retval.handlers = zend_get_std_object_handlers();

    return retval;
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(groonga)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Groonga", groonga_functions);
    groonga_ce = zend_register_internal_class(&ce TSRMLS_CC);
    groonga_ce->create_object = php_groonga_new;

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(groonga)
{
    if (groonga_initialized) {
        grn_fin();
    }
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(groonga)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(groonga)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(groonga)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Groonga Query Transfer Protocol support", "enabled");
    php_info_print_table_row(2, "Version", PHP_GROONGA_VERSION);
    php_info_print_table_end();
}
/* }}} */


/* {{{ proto Groonga Groonga::__construct()
    Public constructor */
PHP_METHOD(Groonga, __construct)
{
    groonga_t *object;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
        RETURN_FALSE;
    }

    /* groongaライブラリを初期化 */
    if (grn_init()) {
        RETURN_FALSE;
    }
    groonga_initialized = 1;

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
    object->connected = 0;

    /* grn_ctx構造体を初期化 */
    if (grn_ctx_init(object->ctx, 0)) {
        zend_throw_exception(NULL, "grn_ctx_init() failed!", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
}
/* }}} */


/* {{{ proto Groonga Groonga::__destruct()
    Public Destructor
 */
PHP_METHOD(Groonga, __destruct) 
{
    groonga_t *object;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
        RETURN_FALSE;
    }

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* groongaとの接続を切断 */
    if (object->connected) {
        grn_ctx_close(object->ctx);
    }

    /* grn_ctx構造体を開放 */
    if (grn_ctx_fin(object->ctx)) {
        zend_throw_exception(NULL, "grn_ctx_fin() failed!", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    object->ctx = NULL;
}
/* }}} */


/* {{{ proto boolean Groonga::connect(string host[, long port [, long flags]])
 */
PHP_METHOD(Groonga, connect)
{
    groonga_t *object;
    char  *host = "localhost";
    int host_len = 0;
    long port = 10041;
    long flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &host, &host_len, &port, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    if (grn_ctx_connect((grn_ctx *)object->ctx, host, port, flags) != GRN_SUCCESS) {
        zend_throw_exception(NULL, "Groonga server went away", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    
    RETURN_TRUE;
}
/* }}} */


/* {{{ proto boolean Groonga::close()
 */
PHP_METHOD(Groonga, close)
{
    groonga_t *object;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, groonga_ce) == FAILURE) {
        RETURN_FALSE;
    }

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* groongaとの接続を切断 */
    if (object->connected) {
        if (grn_ctx_close(object->ctx) != GRN_SUCCESS) {
            RETURN_FALSE;
        }
    }

    RETURN_TRUE;
}
/* }}} */


/* {{{ proto long Groonga::send(string query[, long flags])
 */
PHP_METHOD(Groonga, send)
{
    groonga_t *object;
    char *query = NULL;
    unsigned int query_len, qid;
    long flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &query, &query_len, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* groongaへコマンド送信 */
    qid = grn_ctx_send(object->ctx, query, query_len, flags);
    if (object->ctx->rc != GRN_SUCCESS) {
        RETURN_FALSE;
    }

    RETURN_LONG(qid)
}
/* }}} */


/* {{{ proto boolean Groonga::recv()
 */
PHP_METHOD(Groonga, recv)
{
    groonga_t *object;
    zval *ret = NULL;

    char *str = NULL;
    int flags = 0;
    unsigned int str_len, qid;

    MAKE_STD_ZVAL(ret);

    array_init(ret);
    array_init(return_value);

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* groongaからのメッセージ受信 */
    qid = grn_ctx_recv(object->ctx, &str, &str_len, &flags);
    if (object->ctx->rc != GRN_SUCCESS) {
        RETURN_FALSE;
    }

    add_next_index_long(ret, flags);
    add_next_index_stringl(ret, str, str_len, 1);

    add_index_zval(return_value, qid, ret);
}
/* }}} */


/* {{{ proto long Groonga::query(string query[, long flags])
 */
PHP_METHOD(Groonga, query)
{
    groonga_t *object;
    char *cmd = NULL, *res = NULL;
    unsigned int length;
    long flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &cmd, &length, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    object = (groonga_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    grn_ctx_send(object->ctx, cmd, length, flags);
    if (GRN_SUCCESS == object->ctx->rc) {
        grn_ctx_recv(object->ctx, &res, &length, &flags);
    }

    if (GRN_SUCCESS != object->ctx->rc) {
        RETURN_FALSE;
    }

    if (NULL == res) {
        RETURN_FALSE;
    }

    RETURN_STRINGL(res, length, 1);
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
