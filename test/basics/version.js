// This is separate from basics/chromess-object.js because it has to be
// updated with every release.
test(function () {
    assert_equals(chromess.version.major, 2);
    assert_equals(chromess.version.minor, 1);
    assert_equals(chromess.version.patch, 1);
}, "chromessJS version number is accurate");
