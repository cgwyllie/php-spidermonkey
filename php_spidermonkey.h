#ifndef PHP_SPIDERMONKEY_H
/* Prevent double inclusion */
#define PHP_SPIDERMONKEY_H

/* Define Extension Properties */
#define PHP_SPIDERMONKEY_EXTNAME    "spidermonkey"
#define PHP_SPIDERMONKEY_EXTVER     "0.2"

/* Import configure options
   when building outside of
   the PHP source tree */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Include PHP Standard Header */
#include "php.h"
#define XP_UNIX
/* Include JSAPI Header */
#include "jsapi.h"

#define PHP_SPIDERMONKEY_JSR_NAME           "JSRuntime"
#define PHP_JSRUNTIME_DESCRIPTOR_RES_NAME   "Javascript Runtime"
#define PHP_JSRUNTIME_GC_MEMORY_THRESHOLD   8L * 1024L * 1024L

#define PHP_SPIDERMONKEY_JSC_NAME           "JSContext"
#define PHP_JSCONTEXT_DESCRIPTOR_RES_NAME   "Javascript Context"

#define PHP_SPIDERMONKEY_JSO_NAME           "JSObject"
#define PHP_JSOBJECT_DESCRIPTOR_RES_NAME    "Javascript Object"

/* Structure for JSRuntime object. */
typedef struct _php_jsruntime_object  {
	zend_object             zo;
	JSRuntime               *rt;
} php_jsruntime_object;

/* Structure for JSContext object. */
typedef struct _php_jscontext_object  {
	zend_object             zo;
	php_jsruntime_object    *rt;
	JSContext               *ct;
} php_jscontext_object;

/* Structure for JSObject object. */
typedef struct _php_jsobject_object  {
	zend_object             zo;
	php_jscontext_object    *ct;
	JSObject                *obj;
} php_jsobject_object;

extern zend_class_entry *php_spidermonkey_jsr_entry;
extern zend_class_entry *php_spidermonkey_jsc_entry;
extern zend_class_entry *php_spidermonkey_jso_entry;

// functions
PHP_MINIT_FUNCTION(spidermonkey);
PHP_MSHUTDOWN_FUNCTION(spidermonkey);
PHP_METHOD(JSRuntime, __construct);
PHP_METHOD(JSContext, __construct);
PHP_METHOD(JSContext, __destruct);
PHP_METHOD(JSObject, __construct);
PHP_METHOD(JSObject, __destruct);
PHP_METHOD(JSObject, evaluateScript);

/* Define the entry point symbol
 * Zend will use when loading this module
 */
extern zend_module_entry spidermonkey_module_entry;
#define phpext_spidermonkey_ptr &spidermonkey_module_entry

#endif /* PHP_SPIDERMONKEY_H */
