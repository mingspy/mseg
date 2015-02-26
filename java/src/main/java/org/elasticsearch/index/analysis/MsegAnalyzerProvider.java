package org.elasticsearch.index.analysis;

import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.assistedinject.Assisted;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.Index;
import org.elasticsearch.index.settings.IndexSettings;

import com.mingspy.lucene.MsegAnalyzer;

public class MsegAnalyzerProvider extends AbstractIndexAnalyzerProvider<MsegAnalyzer> {
    private final MsegAnalyzer analyzer;

    @Inject
    public MsegAnalyzerProvider(Index index, @IndexSettings Settings indexSettings, Environment env, @Assisted String name, @Assisted Settings settings) {
        super(index, indexSettings, name, settings);
        //Dictionary.initial(new Configuration(env));
        //analyzer=new MsegAnalyzer(indexSettings, settings, env);
        analyzer=new MsegAnalyzer();
    }

    public MsegAnalyzer get() {
        return this.analyzer;
    }
}
