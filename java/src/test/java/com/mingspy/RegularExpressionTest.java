package com.mingspy;

import org.junit.Assert;
import org.junit.Test;

public class RegularExpressionTest
{
    @Test
    public void testRegular()
    {
        Assert.assertTrue("[中央/n".matches("\\[.+/[a-z]+"));
        Assert.assertTrue("中央/n]/nt".matches(".+/[a-z]+\\]/?[a-z]+"));
        Assert.assertTrue("中央/n]nt".matches(".+/[a-z]+\\]/?[a-z]+"));
        Assert.assertTrue(!"中央/n".matches(".+/[a-z]+\\]/?[a-z]+"));
        Assert.assertTrue(!"中央/n".matches("\\[.+/[a-z]+"));
        Assert.assertTrue("中央/n/b".matches(".+/[a-z]+"));
        Assert.assertTrue("中央/n".matches(".+(/[a-z]+)+"));
    }
}
