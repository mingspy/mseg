package org.elasticsearch.index.analysis;


public class MsegAnalysisBinderProcessor extends AnalysisModule.AnalysisBinderProcessor {

    @Override public void processTokenFilters(TokenFiltersBindings tokenFiltersBindings) {

    }


    @Override public void processAnalyzers(AnalyzersBindings analyzersBindings) {
        analyzersBindings.processAnalyzer("mseg", MsegAnalyzerProvider.class);
        super.processAnalyzers(analyzersBindings);
    }


    @Override
    public void processTokenizers(TokenizersBindings tokenizersBindings) {
      tokenizersBindings.processTokenizer("mseg", MsegTokenizerFactory.class);
      super.processTokenizers(tokenizersBindings);
    }
}
