# Setup script for branch: master
RELEASE=CMSSW_10_2_9
BRANCH=master

# If the release is already available using cmsenv, use it, otherwise set up a new one
if [[ $CMSSW_BASE == *$RELEASE ]] && [[ -d $CMSSW_BASE ]]; then
  echo "Setting up heavyNeutrino package in current release: $CMSSW_BASE"
  cd $CMSSW_BASE/src
else
  scram project CMSSW $RELEASE
  cd $RELEASE/src
  eval `scram runtime -sh`
  echo "Creating release for heavyNeutrino package: $CMSSW_BASE"
fi

# The git commands
gitUser=$(git config --get user.github)
remoteLs=$(git ls-remote https://github.com/$gitUser/heavyNeutrino 2>&1)
if [[ "$remoteLs" = *'Repository not found'* ]]; then
  echo "WARNING: You do not have a heavyNeutrino fork yet, checking out the GhentAnalysis remote instead"
  gitUser="GhentAnalysis"
fi
git cms-init
git clone https://github.com/$gitUser/heavyNeutrino
git cms-merge-topic cms-met:METFixEE2017_949_v2_backport_to_102X # for EE noise fix of 2017 MET
git cms-merge-topic cms-egamma:EgammaID_1023                     # for Fall17_V2 EGamma id's !!! can be removed once CMSSW_10_2_10 is available
git cms-merge-topic cms-egamma:EgammaPostRecoTools               # making the egamma postreco sequence available
cd $CMSSW_BASE/src/heavyNeutrino
git checkout --track origin/$BRANCH
if [[ "$gitUser" != "GhentAnalysis" ]]; then
  git remote add ghent https://github.com/GhentAnalysis/heavyNeutrino
  git fetch ghent
  git rebase ghent/$BRANCH
fi

# Compile and move into package
cd $CMSSW_BASE
scram b -j 10
cd $CMSSW_BASE/src/heavyNeutrino
echo "Setup finished"
