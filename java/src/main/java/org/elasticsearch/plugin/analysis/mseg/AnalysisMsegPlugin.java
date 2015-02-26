package org.elasticsearch.plugin.analysis.mseg;

import org.elasticsearch.common.inject.Module;
import org.elasticsearch.index.analysis.AnalysisModule;
import org.elasticsearch.index.analysis.MsegAnalysisBinderProcessor;
import org.elasticsearch.plugins.AbstractPlugin;

public class AnalysisMsegPlugin extends AbstractPlugin {

	public String name() {
		return "analysis-mseg";
	}


	public String description() {
		return "mseg analysis";
	}

	@Override
	public void processModule(Module module) {
		if (module instanceof AnalysisModule) {
			AnalysisModule analysisModule = (AnalysisModule) module;
			analysisModule.addProcessor(new MsegAnalysisBinderProcessor());
		}
	}
}
