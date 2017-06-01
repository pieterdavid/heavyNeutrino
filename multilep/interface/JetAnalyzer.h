#ifndef JET_ANALYZER_H
#define JET_ANALYZER_H
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"

#include "heavyNeutrino/multilep/plugins/multilep.h"

#include "TTree.h"

class multilep;

class JetAnalyzer {
  private:
    JetCorrectionUncertainty jecUnc;

    static const unsigned nJets_max = 20;

    unsigned _nJets;
    double   _jetPt[nJets_max];
    double   _jetPt_JECUp[nJets_max];
    double   _jetPt_JECDown[nJets_max];
    double   _jetPt_JERUp[nJets_max];
    double   _jetPt_JERDown[nJets_max];
    double   _jetEta[nJets_max];
    double   _jetPhi[nJets_max];
    double   _jetE[nJets_max];
    double   _jetBTaggingCSV[nJets_max];
    double   _jetHadronFlavour[nJets_max];
    unsigned _jetId[nJets_max];

    multilep* multilepAnalyzer;

    bool jetId(const pat::Jet& j, bool tight);

  public:
    JetAnalyzer(const edm::ParameterSet& iConfig, multilep* vars);
    ~JetAnalyzer();

    void beginJob(TTree* outputTree);
    void analyze(const edm::Event&);
};

#endif
