package com.mingspy.tokenizers;

import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;

import javax.swing.text.AttributeSet.CharacterAttribute;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.tokenattributes.CharTermAttribute;

public class MsegAnalyzer extends Analyzer
{

    public static void main(String[] args)
    {
        JsegAnalyzer analyser = new JsegAnalyzer();
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

    @Override
    public TokenStream tokenStream(String arg0, Reader arg1)
    {
        return new MsegTokenizer(arg1);
    }
}
