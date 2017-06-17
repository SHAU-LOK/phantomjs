test(function () {
    assert_type_of(chromess, 'object');
}, "chromess object");

test(function () {
    assert_own_property(chromess, 'libraryPath');
    assert_type_of(chromess.libraryPath, 'string');
    assert_greater_than(chromess.libraryPath.length, 0);
}, "chromess.libraryPath");

test(function () {
    assert_own_property(chromess, 'outputEncoding');
    assert_type_of(chromess.outputEncoding, 'string');
    assert_equals(chromess.outputEncoding.toLowerCase(), 'utf-8'); // default
}, "chromess.outputEncoding");

test(function () {
    assert_own_property(chromess, 'injectJs');
    assert_type_of(chromess.injectJs, 'function');
}, "chromess.injectJs");

test(function () {
    assert_own_property(chromess, 'exit');
    assert_type_of(chromess.exit, 'function');
}, "chromess.exit");

test(function () {
    assert_own_property(chromess, 'cookiesEnabled');
    assert_type_of(chromess.cookiesEnabled, 'boolean');
    assert_is_true(chromess.cookiesEnabled);
}, "chromess.cookiesEnabled");

test(function () {
    assert_own_property(chromess, 'version');
    assert_type_of(chromess.version, 'object');
    assert_type_of(chromess.version.major, 'number');
    assert_type_of(chromess.version.minor, 'number');
    assert_type_of(chromess.version.patch, 'number');
}, "chromess.version");
