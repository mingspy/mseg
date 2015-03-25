package com.mingspy.lucene;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.Reader;

import org.apache.lucene.analysis.Analyzer;
import org.elasticsearch.common.logging.ESLogger;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.settings.ImmutableSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;

import com.mingspy.mseg.MsegJNI;

public class MsegAnalyzer extends Analyzer
{
	private Settings settings=ImmutableSettings.EMPTY;
	private Environment env = new Environment();
	private static final ESLogger logger=Loggers.getLogger("mseg-analyzer");
	public MsegAnalyzer() {
		logger.info("MsegAnalyzer() is called, and not inited.");
	}

	public MsegAnalyzer(Environment env, Settings settings) {
		this.env = env;
		this.settings = settings;
		File config = env.configFile();
		try {
			MsegJNI.setDictRoot(new File(config, "mseg").toString());
			MsegJNI.isLoadInverseDict(true);
			MsegJNI.init();
			logger.info("MsegAnalyzer(env,settings) called, and init JNI in:" + new File(config, "mseg").toString());
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	@Override
	protected TokenStreamComponents createComponents(String fieldName,
			Reader reader) {
		MsegTokenizer tokenizer = new MsegTokenizer(reader);
		String method = this.settings.get("method","FULL").trim();
		tokenizer.setSplitMethod(method);
		logger.info("MsegAnalyzer.createComponents() called, tokenizer.method is:" + method);
		return new TokenStreamComponents(tokenizer);
	}
	
	/*
    public static void main(String[] args)
    {
        MsegAnalyzer analyser = new MsegAnalyzer();
        try {
            TokenStream ts = analyser.tokenStream("words", new StringReader("他说的确实在理"));
            ts.addAttribute(CharTermAttribute.class);
            while(ts.incrementToken()) {
                System.out.println(ts.getAttribute(CharTermAttribute.class));
            }
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    */
}
