
// A SyntaxError leaks to chromess.onError, despite the try-catch.
setup({ allow_uncaught_exception: true });

test(function () {
    var helperFile = "../fixtures/parse-error-helper.js";
    try {
        chromess.injectJs(helperFile);
    } catch (e) {
        assert_equals(e.stack[0].file, helperFile);
        assert_equals(e.stack[0].line, 2);
    }
}, "stack trace from syntax error in injected file");
