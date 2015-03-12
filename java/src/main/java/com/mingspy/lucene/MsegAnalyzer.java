package com.mingspy.lucene;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.Reader;

import org.apache.lucene.analysis.Analyzer;
import org.elasticsearch.common.settings.ImmutableSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;

import com.mingspy.mseg.MsegJNI;

public class MsegAnalyzer extends Analyzer
{
	private Settings settings=ImmutableSettings.EMPTY;
	private Environment env = new Environment();

	public MsegAnalyzer() {
	}

	public MsegAnalyzer(Environment env, Settings settings) {
		this.env = env;
		this.settings = settings;
		File config = env.configFile();
		try {
			MsegJNI.setDictRoot(new File(config, "mseg").toString());
			MsegJNI.init();
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
