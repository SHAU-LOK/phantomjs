var page = require('webpage').create();
// var url = "https://www.baidu.com";

var first_name = 'dongdongc';
var last_name = 'mays';
var email = '4782474242@qq.com';
var address = 'tiantg road';
var phone = '123992923';
var city_zip = '65000';
var city = 'beijing';
var password = 'Su2j2h3kdfQq';
var uid = '359';

var url = 'https://m.starwoodhotels.com/preferredguest/account/enroll/index.html?un_jtt_application_platform=android&language=en_US&localeCode=en_US';
page.onConsoleMessage = function (msg, lineNum, sourceId) {
    console.log('CONSOLE: ' + msg + ' (from line #' + lineNum + ' in "' + sourceId + '")');
};

page.customHeaders = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36',
    'Accept-Language': 'en-US,en;q=0.8,zh-CN;q=0.6,zh;q=0.4,zh-TW;q=0.2',
    'Accept_Encoding': 'gzip, deflate, sdch, br',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8'
};

page.open(url, function (status) {
    page.onConsoleMessage = function (msg, lineNum, sourceId) {
        console.log('CONSOLE: ' + msg + ' (from line #' + lineNum + ' in "' + sourceId + '")');
    };
    page.evaluate(function () {
        // document.getElementById("email").value = "email";
        // document.getElementById("pass").value = "password";
        // document.getElementById("u_0_1").click();
        // document.getElementById("kw").value = "今日关注";
        // document.getElementById("su").click();
        // location.href = "http://example.com";
        // page is redirecting.
        namePrefixDrop.value = 'Mr.';
        firstName.value = 'yang';
        lastName.value = 'zhengsu';

        countryCode.value = 'CN';
        streetAddress1.value = 'city one road ,tianjin';
        city.value = 'beijing';
        stateProvince.value = 'CN11';
        stateProvince_onChange(stateProvince);
        zipCode.value = '510000';


        phoneType0.value = 'home';
        phoneCountryCodeDd0.value = 'CN';
        smsTextPrefShow();
        asTypedPhoneNumber0.value = '18392768392';

        enrollStep1Email.value = '432823890@qq.com';

        userID.value = 'beifuewei';
        window['password-password'].value = '1qaz@WSX1qaX';
        window['password-password4'].value = '1qaz@WSX1qaX';
    });

    setTimeout(function () {
        page.evaluate(function () {
            console.log('haha');
        });
        page.render("page1.png");

        page.evaluate(function () {
            $("#guestInformationForm .buttonForm").click();
        })

        setTimeout(function () {
            page.evaluate(function () {
                console.log('haha2');
            });
            page.render("page2.png");
        }, 5000);

   }, 5000);
});