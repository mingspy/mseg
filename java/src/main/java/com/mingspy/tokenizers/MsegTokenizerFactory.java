package com.mingspy.tokenizers;

import java.io.IOException;
import java.io.Reader;
import java.util.Map;

import org.apache.lucene.analysis.Tokenizer;
import org.apache.lucene.analysis.util.TokenizerFactory;
import org.apache.lucene.util.AttributeSource.AttributeFactory;

public class MsegTokenizerFactory extends TokenizerFactory
{
    protected MsegTokenizerFactory(Map<String, String> args)
    {
        super(args);
    }

    private ThreadLocal<MsegTokenizer> tokenizerLocal = new ThreadLocal<MsegTokenizer>();


    private MsegTokenizer newTokenizer(Reader input)
    {
        MsegTokenizer tokenizer = new MsegTokenizer(input);
        tokenizerLocal.set(tokenizer);
        return tokenizer;
    }


    @Override
    public Tokenizer create(AttributeFactory arg0, Reader input)
    {
        MsegTokenizer tokenizer = newTokenizer(input);
        return tokenizer;
    }
}
