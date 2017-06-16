package ghostdriver;

import org.junit.Test;
import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.chromessjs.chromessJSDriver;

import static junit.framework.TestCase.assertEquals;

public class chromessJSCommandTest extends BaseTest {
    @Test
    public void executechromessJS() {
        WebDriver d = getDriver();
        if (!(d instanceof chromessJSDriver)) {
            // Skip this test if not using chromessJS.
            // The command under test is only available when using chromessJS
            return;
        }

        chromessJSDriver chromess = (chromessJSDriver)d;

        // Do we get results back?
        Object result = chromess.executechromessJS("return 1 + 1");
        assertEquals(new Long(2), (Long)result);

        // Can we read arguments?
        result = chromess.executechromessJS("return arguments[0] + arguments[0]", new Long(1));
        assertEquals(new Long(2), (Long)result);

        // Can we override some browser JavaScript functions in the page context?
        result = chromess.executechromessJS("var page = this;" +
           "page.onInitialized = function () { " +
                "page.evaluate(function () { " +
                    "Math.random = function() { return 42 / 100 } " +
                "})" +
            "}");

        chromess.get("http://ariya.github.com/js/random/");

        WebElement numbers = chromess.findElement(By.id("numbers"));
        boolean foundAtLeastOne = false;
        for(String number : numbers.getText().split(" ")) {
            foundAtLeastOne = true;
            assertEquals("42", number);
        }
        assert(foundAtLeastOne);
    }
}
