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
#include "zend_exceptions.h"
#include "zend_API.h"
#include "php_gqtpcli.h"

/* If you declare any globals in php_gqtpcli.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(gqtpcli)
*/

/* True global resources - no need for thread safety here */
static int le_gqtpcli;

/* GQTPcli初期化フラグ */
static int gqtpcli_initialized = 0;

/* GQTPcliクラス構造体 */
zend_class_entry *gqtpcli_ce;

/* {{{ gqtpcli_class_methods[] */
const zend_function_entry gqtpcli_class_methods[] = {
    PHP_ME(GQTPcli, __construct,     NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, __destruct,      NULL, ZEND_ACC_DTOR | ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, connect,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, close,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, send,            NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, recv,            NULL, ZEND_ACC_PUBLIC)

    PHP_ME(GQTPcli, getFlags,        NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, getQueryID,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, getErrorMessage, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GQTPcli, query,           NULL, ZEND_ACC_PUBLIC)

    PHP_FE_END    /* Must be the last line in gqtpcli_class_methods[] */
};
/* }}} */

/* {{{ gqtpcli_module_entry
 */
zend_module_entry gqtpcli_module_entry = {
    STANDARD_MODULE_HEADER,
    "gqtpcli",
    NULL,
    PHP_MINIT(gqtpcli),
    PHP_MSHUTDOWN(gqtpcli),
    PHP_RINIT(gqtpcli),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(gqtpcli),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(gqtpcli),
    PHP_GQTPCLI_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_GQTPCLI
ZEND_GET_MODULE(gqtpcli)
#endif

/* {{{ php_gqtpcli_free_storage
 */
static void php_gqtpcli_free_storage(gqtpcli_t *object TSRMLS_DC)
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


/* {{{ php_gqtpcli_new
 */
zend_object_value php_gqtpcli_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    gqtpcli_t *object;
    zval *tmp;

    object = ecalloc(1, sizeof(*object));

    zend_object_std_init(&object->std, ce TSRMLS_CC);
    object_properties_init(&object->std, ce);
    rebuild_object_properties(&object->std);

    object->ctx = emalloc(sizeof(*object->ctx));
    object->connected = 0;
    object->flags = 0;

    retval.handle = zend_objects_store_put(
        object, 
        (zend_objects_store_dtor_t)zend_objects_destroy_object, 
        (zend_objects_free_object_storage_t)php_gqtpcli_free_storage, 
        NULL TSRMLS_CC
    );
    retval.handlers = zend_get_std_object_handlers();

    return retval;
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(gqtpcli)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "GQTPcli", gqtpcli_class_methods);
    gqtpcli_ce = zend_register_internal_class(&ce TSRMLS_CC);
    gqtpcli_ce->create_object = php_gqtpcli_new;

    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_USE_QL",     (long)GRN_CTX_USE_QL);      // (0x03)

    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_BATCH_MODE", (long)GRN_CTX_BATCH_MODE);  // (0x04)

    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_DEFAULT",    (long)GRN_ENC_DEFAULT);     // 0
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_NONE",       (long)GRN_ENC_NONE);        // 1
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_EUC_JP",     (long)GRN_ENC_EUC_JP);      // 2
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_UTF8",       (long)GRN_ENC_UTF8);        // 3
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_SJIS",       (long)GRN_ENC_SJIS);        // 4
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_LATIN1",     (long)GRN_ENC_LATIN1);      // 5
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_ENC_KOI8R",      (long)GRN_ENC_KOI8R);       // 6

    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_MORE",       (long)GRN_CTX_MORE);        // (0x01<<0)
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_TAIL",       (long)GRN_CTX_TAIL);        // (0x01<<1)
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_HEAD",       (long)GRN_CTX_HEAD);        // (0x01<<2)
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_QUIET",      (long)GRN_CTX_QUIET);       // (0x01<<3)
    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_QUIT",       (long)GRN_CTX_QUIT);        // (0x01<<4)

    REGISTER_CLASS_CONST_LONG(gqtpcli_ce, "GRN_CTX_FIN",        (long)GRN_CTX_FIN);         // (0xff)


    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(gqtpcli)
{
    if (gqtpcli_initialized) {
        grn_fin();
    }
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(gqtpcli)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(gqtpcli)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(gqtpcli)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Groonga Query Transfer Protocol support", "enabled");
    php_info_print_table_row(2, "Version", PHP_GQTPCLI_VERSION);
    php_info_print_table_end();
}
/* }}} */


/* {{{ proto GQTPcli GQTPcli::__construct()
    Public constructor */
PHP_METHOD(GQTPcli, __construct)
{
    gqtpcli_t *object;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
        RETURN_FALSE;
    }

    /* gqtpcliライブラリを初期化 */
    if (GRN_SUCCESS != grn_init()) {
        RETURN_FALSE;
    }
    gqtpcli_initialized = 1;

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
    object->connected = 0;

    /* grn_ctx構造体を初期化 */
    if (grn_ctx_init(object->ctx, 0)) {
        zend_throw_exception(NULL, "Unable to  finish context.", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
}
/* }}} */


/* {{{ proto GQTPcli GQTPcli::__destruct()
    Public Destructor
 */
PHP_METHOD(GQTPcli, __destruct) 
{
    gqtpcli_t *object;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
        RETURN_FALSE;
    }

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* gqtpcliとの接続を切断 */
    if (object->connected) {
        grn_ctx_close(object->ctx);
    }

    /* grn_ctx構造体を開放 */
    if (grn_ctx_fin(object->ctx)) {
        zend_throw_exception(NULL, "Unable to  finish context.", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    object->ctx = NULL;
}
/* }}} */


/* {{{ proto boolean GQTPcli::connect(string host[, long port [, long flags]])
 */
PHP_METHOD(GQTPcli, connect)
{
    gqtpcli_t *object;
    char errmsg[64];
    char  *host = "localhost";
    int host_len = 0;
    long port = 10041;
    long flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &host, &host_len, &port, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    if (grn_ctx_connect((grn_ctx *)object->ctx, host, port, flags) != GRN_SUCCESS) {
        sprintf(errmsg, "Unable to connect to Groonga server.[%s:%d]", host, port);
        zend_throw_exception(NULL, errmsg, 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    
    RETURN_TRUE;
}
/* }}} */


/* {{{ proto boolean GQTPcli::close()
 */
PHP_METHOD(GQTPcli, close)
{
    gqtpcli_t *object;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, gqtpcli_ce) == FAILURE) {
        RETURN_FALSE;
    }

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* gqtpcliとの接続を切断 */
    if (object->connected) {
        if (grn_ctx_close(object->ctx) != GRN_SUCCESS) {
            RETURN_FALSE;
        }
    }

    RETURN_TRUE;
}
/* }}} */


/* {{{ proto boolean GQTPcli::send(string query[, long flags])
 */
PHP_METHOD(GQTPcli, send)
{
    gqtpcli_t *object;
    char *query = NULL;
    unsigned int query_len, qid;
    long flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &query, &query_len, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* gqtpcliへコマンド送信 */
    object->qid = grn_ctx_send(object->ctx, query, query_len, flags);
    if (object->ctx->rc != GRN_SUCCESS) {
        zend_throw_exception(NULL, object->ctx->errbuf, 0 TSRMLS_CC);

        RETURN_FALSE;
    }

    RETURN_TRUE;
}
/* }}} */


/* {{{ proto string GQTPcli::recv()
 */
PHP_METHOD(GQTPcli, recv)
{
    gqtpcli_t *object;
    char *res = NULL;
    unsigned int length;
    int flags;

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    /* gqtpcliからのメッセージ受信 */
    object->qid = grn_ctx_recv(object->ctx, &res, &length, &flags);
    if (object->ctx->rc != GRN_SUCCESS) {
        RETURN_FALSE;
    }
    object->flags = flags;

    RETURN_STRINGL(res, length, 1);
}
/* }}} */


/* {{{ proto long GQTPcli::getFlags()
 */
PHP_METHOD(GQTPcli, getFlags)
{
    gqtpcli_t *object;

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    RETURN_LONG(object->flags);
}
/* }}} */


/* {{{ proto long GQTPcli::getQueryID()
 */
PHP_METHOD(GQTPcli, getQueryID)
{
    gqtpcli_t *object;

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    RETURN_LONG(object->qid);
}
/* }}} */


/* {{{ proto long GQTPcli::getErrorMessage()
 */
PHP_METHOD(GQTPcli, getErrorMessage)
{
    gqtpcli_t *object;
    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    RETURN_STRING(object->ctx->errbuf, 1);
}
/* }}} */


/* {{{ proto string GQTPcli::query(string query[, long flags])
 */
PHP_METHOD(GQTPcli, query)
{
    gqtpcli_t *object;
    char *cmd = NULL, *res = NULL;
    unsigned int length;
    long flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &cmd, &length, &flags) == FAILURE) {
        RETURN_FALSE;
    }

    object = (gqtpcli_t *) zend_object_store_get_object(getThis() TSRMLS_CC);

    object->qid = grn_ctx_send(object->ctx, cmd, length, flags);
    if (GRN_SUCCESS == object->ctx->rc) {
        grn_ctx_recv(object->ctx, &res, &length, &object->flags);
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
