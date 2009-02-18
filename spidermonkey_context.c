#include "php_spidermonkey.h"

static int le_jscontext_descriptor;

/**
 * JSContext embedding
 */
zend_class_entry *php_spidermonkey_jsc_entry;

/* {{{ proto public JSContext::__construct(JSRuntime $runtime)
   JSContext's constructor, expect a JSRuntime, you should use
   JSRuntime::createContext */
PHP_METHOD(JSContext, __construct)
{
	/* prevent creating this object */
	zend_throw_exception(zend_exception_get_default(TSRMLS_C), "JSContext can't be instancied directly, please call JSRuntime::createContext", 0 TSRMLS_CC);
}
/* }}} */

/* {{{ proto public void JSContext::__destruct()
   Destroy context and free up memory */
PHP_METHOD(JSContext, __destruct)
{
	php_jscontext_object *intern;

	/* retrieve this class from the store */
	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

#if ((PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION >= 3))
	Z_ADDREF_P(intern->rt_z);
#else
	ZVAL_DELREF(intern->rt_z);
#endif

	/* if a context is found ( which should be the case )
	 * destroy it
	 */
	if (intern->ct != (JSContext*)NULL)
		JS_DestroyContext(intern->ct);

	intern->rt = NULL;
}
/* }}} */

/* {{{ proto public bool JSContext::registerFunction(string name, callback function)
   Register a PHP function in a Javascript context allowing a script to call it*/
PHP_METHOD(JSContext, registerFunction)
{
	char					*name;
	int						name_len;
	php_callback			callback;
	php_jscontext_object	*intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sf", &name, &name_len, &callback.fci, &callback.fci_cache) == FAILURE) {
		RETURN_NULL();
	}

	/* retrieve this class from the store */
	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	Z_ADDREF_P(callback.fci.function_name);

	zend_hash_add(intern->ht, name, name_len, &callback, sizeof(callback), NULL);

	JS_DefineFunction(intern->ct, intern->obj, name, generic_call, 1, 0);

}
/* }}} */

/* {{{ proto public bool JSContext::registerFunction(string name, callback function)
   Register a PHP function in a Javascript context allowing a script to call it*/
PHP_METHOD(JSContext, assign)
{
	char					*name;
	int						name_len;
	zval					*val;
	php_jscontext_object	*intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &val) == FAILURE) {
		RETURN_NULL();
	}

	/* retrieve this class from the store */
	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	jsval ival;
	zval_to_jsval(val, intern->ct, &ival);

	JS_SetProperty(intern->ct, intern->obj, name, &ival);
}
/* }}} */

/* {{{ proto public mixed JSContext::evaluateScript(string $script)
   Evaluate script and return the last global object in scope to PHP.
   Objects are not returned for now. Any global variable declared in
   this script will be usable in any following call to evaluateScript
   in any JSObject in  the same context. */
PHP_METHOD(JSContext, evaluateScript)
{
	char *script;
	int script_len;
	php_jscontext_object *intern;
	jsval rval;

	/* retrieve script */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
						"s", &script, &script_len) == FAILURE) {
		RETURN_FALSE;
	}

	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (JS_EvaluateScript(intern->ct, intern->obj, script, script_len, NULL, 0, &rval) == JS_TRUE)
	{
		/* The script evaluated fine, convert the return value to PHP */
		jsval_to_zval(return_value, intern->ct, &rval);
	}
	else
	{
		RETURN_FALSE;
	}

}
/* }}} */

/* {{{ proto public mixed JSContext::setOptions(long $options)
   Set options for the current context, $options is a bitfield made of JSOPTION_ consts */
PHP_METHOD(JSContext, setOptions)
{
	php_jscontext_object	*intern;
	long					options;
	long					old_options;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
						 "l", &options) == FAILURE) {
		RETURN_NULL();
	}

	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	old_options = JS_SetOptions(intern->ct, options);
	
	if (JS_GetOptions(intern->ct) == options)
	{
		RETVAL_LONG(old_options);
	}
	else
	{
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public mixed JSContext::toggleOptions(long $options)
   Just toggle a set of options, same as doing setOptions 
   with JSContext::getOptions() ^ $options */
PHP_METHOD(JSContext, toggleOptions)
{
	php_jscontext_object	*intern;
	long					options;
	long					old_options;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
						 "l", &options) == FAILURE) {
		RETURN_NULL();
	}

	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	old_options = JS_ToggleOptions(intern->ct, options);
	
	if (JS_GetOptions(intern->ct) == (old_options ^ options))
	{
		RETVAL_LONG(old_options);
	}
	else
	{
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public long JSContext::getOptions()
   Return context's options */
PHP_METHOD(JSContext, getOptions)
{
	php_jscontext_object	*intern;

	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RETVAL_LONG(JS_GetOptions(intern->ct));
}
/* }}} */

/* {{{ proto public mixed JSContext::setVersion(long $version)
   Set Javascript version for this context, supported versions are
   dependant of the current spidermonkey library */
PHP_METHOD(JSContext, setVersion)
{
	php_jscontext_object	*intern;
	long					version;
	long					old_version;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
						 "l", &version) == FAILURE) {
		RETURN_NULL();
	}

	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	old_version = JS_SetVersion(intern->ct, version);
	
	if (JS_GetVersion(intern->ct) == version)
	{
		RETVAL_LONG(old_version);
	}
	else
	{
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public long JSContext::getVersion(long $options)
   Return current version */
PHP_METHOD(JSContext, getVersion)
{
	php_jscontext_object	*intern;

	intern = (php_jscontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RETVAL_LONG(JS_GetVersion(intern->ct));
}
/* }}} */

/* {{{ proto public string JSContext::getVersionString(long $version)
   Return the version name base on his number*/
PHP_METHOD(JSContext, getVersionString)
{
	const char *version_str;
	int l;
	long version;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
						 "l", &version) == FAILURE) {
		RETURN_NULL();
	}

	version_str = JS_VersionToString(version);
	l = strlen(version_str);

	RETVAL_STRINGL(estrndup(version_str, l), l, 0);
}
/* }}} */

/*******************************************
* Internal function for the script JS class
*******************************************/


JSBool generic_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSString				*str;
	JSFunction				*func;
	JSString				*jfunc_name;
	char					*func_name;
	zval					***params, *retval_ptr;
	php_callback			*callback;
	php_jscontext_object	*intern;
	HashTable				*ht;
	int						i;

	/* first retrieve function name */
	func = JS_ValueToFunction(cx, ((argv)[-2]));
	jfunc_name = JS_GetFunctionId(func);
	func_name = JS_GetStringBytes(jfunc_name);

	intern = (php_jscontext_object*)JS_GetContextPrivate(cx);

	if ((ht = (HashTable*)JS_GetInstancePrivate(cx, obj, &intern->script_class, NULL)) == 0)
	{
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Failed to retrieve function table", 0 TSRMLS_CC);
	}

	/* search for function callback */
	if (zend_hash_find(ht, func_name, strlen(func_name), (void**)&callback) == FAILURE) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Failed to retrieve function callback", 0 TSRMLS_CC);
	}

	/* ready parameters */
	params = emalloc(argc * sizeof(zval**));
	for (i = 0; i < argc; i++)
	{
		zval **val = emalloc(sizeof(zval*));
		MAKE_STD_ZVAL(*val);
		jsval_to_zval(*val, cx, &argv[i]);
		params[i] = val;
	}

	callback->fci.params			= params;
	callback->fci.param_count		= argc;
	callback->fci.retval_ptr_ptr	= &retval_ptr;

//	php_printf("Size: %d\nFunction table: %p\nFunction name: %s\n", callback->fci.size, callback->fci.function_table, callback->fci.function_name);

	zend_call_function(&callback->fci, NULL TSRMLS_CC);

	/* call ended, clean */
	for (i = 0; i < argc; i++)
	{
		zval **eval;
		eval = params[i];
		zval_ptr_dtor(eval);
		efree(eval);
	}

	if (retval_ptr)
	{
		zval_to_jsval(retval_ptr, cx, rval);
		zval_ptr_dtor(&retval_ptr);
	}
	else
	{
		*rval = JSVAL_VOID;
	}
	
	efree(params);

	return JS_TRUE;
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
