package org.elasticsearch.index.analysis;

import java.io.File;
import java.io.FileNotFoundException;

import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.assistedinject.Assisted;
import org.elasticsearch.common.logging.ESLogger;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.Index;
import org.elasticsearch.index.settings.IndexSettings;

import com.mingspy.lucene.MsegAnalyzer;
import com.mingspy.mseg.MsegJNI;

public class MsegAnalyzerProvider extends AbstractIndexAnalyzerProvider<MsegAnalyzer> {
    private final MsegAnalyzer analyzer;

    private static final ESLogger logger=Loggers.getLogger("mseg-analyzer");
    @Inject
    public MsegAnalyzerProvider(Index index, @IndexSettings Settings indexSettings, Environment env, @Assisted String name, @Assisted Settings settings) {
        super(index, indexSettings, name, settings);
        logger.info("MsegAnalyzerProvider new MsegAnalyzer");
        analyzer=new MsegAnalyzer(env,settings);
        
    }

    public MsegAnalyzer get() {
        return this.analyzer;
    }
}
