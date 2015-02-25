package com.mingspy.tokenizers;

import java.util.Map;

import com.senseidb.plugin.SenseiPluginFactory;
import com.senseidb.plugin.SenseiPluginRegistry;

public class MsegSenseiFactory  implements SenseiPluginFactory<JsegAnalyzer>
{

    @Override
    public JsegAnalyzer getBean(Map<String, String> arg0, String arg1,
                                SenseiPluginRegistry arg2)
    {
        return new JsegAnalyzer();
    }

}
