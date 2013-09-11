
# Spidermonkey 17 Build

Build the Spidermonkey 17 engine from source following [the source build instructions](https://developer.mozilla.org/en-US/docs/SpiderMonkey/Build_Documentation). This can be configured with a `--prefix` option if you don't want to globally install the library.

If all goes well, checkout the `mozjs17-build-cpp` branch of the extension and `phpize` & `./configure`. If `libmozjs17` is installed in a prefix, use the `--with-spidermonkey` config argument.

Once configured, edit the makefile to set the compiler to `g++` rather than `cc`, `make` and `make test`. 8/10 cases should pass, failing on closure passing.

# Issues

- Use of C++ JSAPI/compiler seemed necessary to resolve strange linking bugs at runtime, but perhaps isn't
- Makefile edit for compiler isn't portable/sane, not sure how to resolve
- Spidermonkey17 seems much stricter at function calls than previous versions, and this results in failing test cases (see gist for simple reproducing case and thoughts on resolution)

# Gists

Simple closure failing case: https://gist.github.com/cgwyllie/6521189 (run with `php -f test.php` with spidermonkey loaded into `php-cli`). This will cause a type error because the JS runtime thinks the closure is not a function. Possible fixes:

- Use the `JSClass.call` hook to do a generic call and test at runtime if the `zval` is callable
- Use `JS_NewFunction` to define closures in `zval_to_jsval`, triggered by the property getter if the `zval` passes `zend_is_callable`

Problems with those approaches seem to be determining the original zval that was acted upon, as the `this` object from JS is resolved to the `window` in the gist, not the propety. The callee doesn't seem to have the property name attached either(?). If we could extract the property name from the callee, perhaps the `JSClass.call` hook would be the best way to go.

## Notes/thoughts

- The sources are messy just now because we've left a lot of debugging statements lying around
- We commented out some of the things that weren't important to script execution that didn't compile with C++ yet (we can port later)
- Some of the support for older JSAPIs with `#ifdef` has been taken out, we planned to remove <JS185 support in this branch as it's so old now
- This may not even be compatible with JS185 because of the C++ API introduction. However, mozjs24 is due at the end of the year and will officially deprecate the entire C API (apprently) so a transition to the C++ API may be on the cards in the future anyway
- Perhaps best maintained as a seprate branch rather than merged on master?