package com.mingspy.lucene;

import java.io.Reader;

import org.apache.lucene.analysis.Analyzer;

public class MsegAnalyzer extends Analyzer
{
	@Override
	protected TokenStreamComponents createComponents(String fieldName,
			Reader reader) {
		MsegTokenizer tokenizer = new MsegTokenizer(reader);
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
