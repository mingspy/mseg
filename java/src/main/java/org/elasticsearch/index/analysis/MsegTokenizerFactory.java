package org.elasticsearch.index.analysis;

import java.io.Reader;

import org.apache.lucene.analysis.Tokenizer;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.assistedinject.Assisted;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.Index;
import org.elasticsearch.index.settings.IndexSettings;

import com.mingspy.lucene.MsegTokenizer;

public class MsegTokenizerFactory extends AbstractTokenizerFactory {
  private Environment environment;
  private Settings settings;

  @Inject
  public MsegTokenizerFactory(Index index, @IndexSettings Settings indexSettings, Environment env, @Assisted String name, @Assisted Settings settings) {
	  super(index, indexSettings, name, settings);
	  this.environment = env;
	  this.settings = settings;
	  //Dictionary.initial(new Configuration(env));
  }

  public Tokenizer create(Reader reader) {
	  return new MsegTokenizer(reader);
  }

}
