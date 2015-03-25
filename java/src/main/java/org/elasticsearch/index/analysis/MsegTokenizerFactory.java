package org.elasticsearch.index.analysis;

import java.io.IOException;
import java.io.Reader;
import java.util.logging.Logger;

import org.apache.lucene.analysis.Tokenizer;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.assistedinject.Assisted;
import org.elasticsearch.common.logging.ESLogger;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.Index;
import org.elasticsearch.index.settings.IndexSettings;

import com.mingspy.lucene.MsegAnalyzer;
import com.mingspy.lucene.MsegTokenizer;

public class MsegTokenizerFactory extends AbstractTokenizerFactory {
	private Environment environment;
	private Settings settings;
	private MsegAnalyzer analyzer;
	private static final ESLogger logger=Loggers.getLogger("mseg-analyzer");
	@Inject
	public MsegTokenizerFactory(Index index,
			@IndexSettings Settings indexSettings, Environment env,
			@Assisted String name, @Assisted Settings settings) {
		super(index, indexSettings, name, settings);
		this.environment = env;
		this.settings = settings;
		logger.info("MsegTokenizerFactory: new MsegAnalyzer()");
		analyzer=new MsegAnalyzer(env,settings);

	}

	public Tokenizer create(Reader reader) {
		
		try {
			logger.info("MsegTokenizerFactory: create tokenizer by calling analyzer.tokenStream.");
			return (Tokenizer) analyzer.tokenStream("word", reader);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		String method = this.settings.get("method","FULL").trim();
		MsegTokenizer tokenizer = new MsegTokenizer(reader);
		tokenizer.setSplitMethod(method);
		return tokenizer;
	}

}
