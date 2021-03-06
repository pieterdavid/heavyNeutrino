#include "heavyNeutrino/multilep/plugins/multilep.h"


multilep::multilep(const edm::ParameterSet& iConfig):
    vtxToken(                         consumes<std::vector<reco::Vertex>>(        iConfig.getParameter<edm::InputTag>("vertices"))),
    genEventInfoToken(                consumes<GenEventInfoProduct>(              iConfig.getParameter<edm::InputTag>("genEventInfo"))),
    genLumiInfoToken(                 consumes<GenLumiInfoHeader, edm::InLumi>(   iConfig.getParameter<edm::InputTag>("genEventInfo"))),
    lheEventInfoToken(                consumes<LHEEventProduct>(                  iConfig.getParameter<edm::InputTag>("lheEventInfo"))),
    pileUpToken(                      consumes<std::vector<PileupSummaryInfo>>(   iConfig.getParameter<edm::InputTag>("pileUpInfo"))),
    genParticleToken(                 consumes<reco::GenParticleCollection>(      iConfig.getParameter<edm::InputTag>("genParticles"))),
    muonToken(                        consumes<std::vector<pat::Muon>>(           iConfig.getParameter<edm::InputTag>("muons"))),
    eleToken(                         consumes<std::vector<pat::Electron>>(       iConfig.getParameter<edm::InputTag>("electrons"))),
    tauToken(                         consumes<std::vector<pat::Tau>>(            iConfig.getParameter<edm::InputTag>("taus"))),
    photonToken(                      consumes<std::vector<pat::Photon>>(         iConfig.getParameter<edm::InputTag>("photons"))),
    packedCandidatesToken(            consumes<std::vector<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("packedCandidates"))),
    rhoToken(                         consumes<double>(                           iConfig.getParameter<edm::InputTag>("rho"))),
    metToken(                         consumes<std::vector<pat::MET>>(            iConfig.getParameter<edm::InputTag>("met"))),
    jetToken(                         consumes<std::vector<pat::Jet>>(            iConfig.getParameter<edm::InputTag>("jets"))),
    jetSmearedToken(                  consumes<std::vector<pat::Jet>>(            iConfig.getParameter<edm::InputTag>("jetsSmeared"))),
    jetSmearedUpToken(                consumes<std::vector<pat::Jet>>(            iConfig.getParameter<edm::InputTag>("jetsSmearedUp"))),
    jetSmearedDownToken(              consumes<std::vector<pat::Jet>>(            iConfig.getParameter<edm::InputTag>("jetsSmearedDown"))),
    recoResultsPrimaryToken(          consumes<edm::TriggerResults>(              iConfig.getParameter<edm::InputTag>("recoResultsPrimary"))),
    recoResultsSecondaryToken(        consumes<edm::TriggerResults>(              iConfig.getParameter<edm::InputTag>("recoResultsSecondary"))),
    triggerToken(                     consumes<edm::TriggerResults>(              iConfig.getParameter<edm::InputTag>("triggers"))),
    prescalesToken(                   consumes<pat::PackedTriggerPrescales>(      iConfig.getParameter<edm::InputTag>("prescales"))),
    skim(                                                                         iConfig.getUntrackedParameter<std::string>("skim")),
    isData(                                                                       iConfig.getUntrackedParameter<bool>("isData")),
    is2017(                                                                       iConfig.getUntrackedParameter<bool>("is2017")),
    is2018(                                                                       iConfig.getUntrackedParameter<bool>("is2018")),
    isSUSY(                                                                       iConfig.getUntrackedParameter<bool>("isSUSY")),
    storeLheParticles(                                                            iConfig.getUntrackedParameter<bool>("storeLheParticles"))
{
    if(is2017 or is2018) ecalBadCalibFilterToken = consumes<bool>(edm::InputTag("ecalBadCalibReducedMINIAODFilter"));
    triggerAnalyzer = new TriggerAnalyzer(iConfig, this);
    leptonAnalyzer  = new LeptonAnalyzer(iConfig, this);
    photonAnalyzer  = new PhotonAnalyzer(iConfig, this);
    jetAnalyzer     = new JetAnalyzer(iConfig, this);
    genAnalyzer     = new GenAnalyzer(iConfig, this);
    lheAnalyzer     = new LheAnalyzer(iConfig, this);
    susyMassAnalyzer= new SUSYMassAnalyzer(iConfig, this, lheAnalyzer);
}

multilep::~multilep(){
    delete triggerAnalyzer;
    delete leptonAnalyzer;
    delete photonAnalyzer;
    delete jetAnalyzer;
    delete genAnalyzer;
    delete lheAnalyzer;
    delete susyMassAnalyzer;
}

// ------------ method called once each job just before starting event loop  ------------
void multilep::beginJob(){

    //Initialize tree with event info
    outputTree = fs->make<TTree>("blackJackAndHookersTree", "blackJackAndHookersTree");
    nVertices  = fs->make<TH1D>("nVertices", "Number of vertices", 120, 0, 120);

    //Set all branches of the outputTree
    outputTree->Branch("_runNb",                        &_runNb,                        "_runNb/l");
    outputTree->Branch("_lumiBlock",                    &_lumiBlock,                    "_lumiBlock/l");
    outputTree->Branch("_eventNb",                      &_eventNb,                      "_eventNb/l");
    outputTree->Branch("_nVertex",                      &_nVertex,                      "_nVertex/b");

    if(!isData) lheAnalyzer->beginJob(outputTree, fs);
    if(isSUSY)  susyMassAnalyzer->beginJob(outputTree, fs);
    if(!isData) genAnalyzer->beginJob(outputTree);
    triggerAnalyzer->beginJob(outputTree);
    leptonAnalyzer->beginJob(outputTree);
    photonAnalyzer->beginJob(outputTree);
    jetAnalyzer->beginJob(outputTree);

    _runNb = 0;
}

// ------------ method called for each lumi block ---------
void multilep::beginLuminosityBlock(const edm::LuminosityBlock& iLumi, const edm::EventSetup& iSetup){
    if(isSUSY) susyMassAnalyzer->beginLuminosityBlock(iLumi, iSetup);
    _lumiBlock = (unsigned long) iLumi.id().luminosityBlock();
}

//------------- method called for each run -------------
void multilep::beginRun(const edm::Run& iRun, edm::EventSetup const& iSetup){
    _runNb = (unsigned long) iRun.id().run();
    triggerAnalyzer->reIndex = true;                                   // HLT results could have different size/order in new run, so look up again the index positions
}

// ------------ method called for each event  ------------
void multilep::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){
    edm::Handle<std::vector<reco::Vertex>> vertices; iEvent.getByToken(vtxToken, vertices);
    if(!isData) lheAnalyzer->analyze(iEvent);                          // needs to be run before selection to get correct uncertainties on MC xsection
    if(isSUSY) susyMassAnalyzer->analyze(iEvent);                      // needs to be run after LheAnalyzer, but before all other models

    //extract number of vertices 
    _nVertex = vertices->size();
    nVertices->Fill(_nVertex, lheAnalyzer->getWeight()); 
    if(_nVertex == 0) return;                                          //Don't consider 0 vertex events

    if(!leptonAnalyzer->analyze(iEvent, *(vertices->begin()))) return; // returns false if doesn't pass skim condition, so skip event in such case
    if(!photonAnalyzer->analyze(iEvent))                       return;
    if(!jetAnalyzer->analyze(iEvent))                          return;
    if(!isData) genAnalyzer->analyze(iEvent);
    triggerAnalyzer->analyze(iEvent);

    _eventNb   = (unsigned long) iEvent.id().event();                  //determine event number run number and luminosity block
    outputTree->Fill();                                                //store calculated event info in root tree
}

//define this as a plug-in
DEFINE_FWK_MODULE(multilep);
