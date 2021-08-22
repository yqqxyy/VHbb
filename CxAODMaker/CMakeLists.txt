################################################################################
# Build configuration for CxAODMaker
################################################################################

# Declare the name of the package:
atlas_subdir( CxAODMaker )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC 
   PhysicsAnalysis/D3PDTools/EventLoop 
   PhysicsAnalysis/AnalysisCommon/AssociationUtils/AssociationUtils
   CxAODTools
   Event/xAOD/xAODBase
   Control/xAODRootAccess
   Event/xAOD/xAODEventInfo 
   Event/xAOD/xAODBTagging
   PhysicsAnalysis/AnalysisCommon/PATInterfaces/PATInterfaces
   Tools/PathResolver
   DataQuality/GoodRunsLists
   Event/xAOD/xAODJet
   Event/xAOD/xAODMuon
   Event/xAOD/xAODEgamma
   Event/xAOD/xAODTracking
   PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonEfficiencyCorrections
   PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonMomentumCorrections
   PhysicsAnalysis/MuonID/MuonSelectorTools
   Reconstruction/Jet/JetCalibTools
   PhysicsAnalysis/JetMissingEtID/JetSelectorTools
   Reconstruction/Jet/JetResolution
   Reconstruction/Jet/JetUncertainties
   Control/AthContainers
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonFourMomentumCorrection
   PhysicsAnalysis/ElectronPhotonID/ElectronEfficiencyCorrection
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonSelectorTools
   PhysicsAnalysis/AnalysisCommon/IsolationSelection
   Reconstruction/MET/METUtilities
   Reconstruction/MET/METInterface
   Control/AthToolSupport/AsgTools
   Event/xAOD/xAODMissingET
   PhysicsAnalysis/TauID/TauAnalysisTools
   Event/xAOD/xAODTau
   PhysicsAnalysis/AnalysisCommon/PileupReweighting
   Trigger/TrigConfiguration/TrigConfxAOD
   Trigger/TrigAnalysis/TrigDecisionTool
   Reconstruction/Jet/JetRec
   Reconstruction/Jet/JetEDM
   PhysicsAnalysis/ElectronPhotonID/PhotonEfficiencyCorrection
   Event/xAOD/xAODCutFlow
   Event/xAOD/xAODTrigMissingET
   Event/xAOD/xAODTrigEgamma
   Event/xAOD/xAODTrigCalo
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonShowerShapeFudgeTool
   Event/xAOD/xAODTrigMuon
   Reconstruction/Jet/JetMomentTools
   Reconstruction/Jet/JetSubStructureUtils
   Trigger/TrigAnalysis/TrigTauMatching
   PhysicsAnalysis/ElectronPhotonID/IsolationCorrections
   Reconstruction/Jet/JetJvtEfficiency
   Trigger/TrigAnalysis/TriggerMatchingTool
   Trigger/TrigAnalysis/TrigBunchCrossingTool
   #External/Asg_MCUtils
   Reconstruction/Jet/JetAnalysisTools/JetTileCorrection
   Reconstruction/Jet/BoostedJetTaggers
   PhysicsAnalysis/TrackingID/InDetTrackSystematicsTools
   )

find_package( ROOT COMPONENTS Core RIO Hist Tree )

# Build a dictionary for the library:
atlas_add_root_dictionary( CxAODMaker _dictionarySource
   ROOT_HEADERS Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

# Build a shared library:
atlas_add_library( CxAODMaker
   CxAODMaker/*.h Root/*.h Root/*.cxx ${_dictionarySource}
   PUBLIC_HEADERS CxAODMaker
   LINK_LIBRARIES EventLoop AssociationUtilsLib CxAODTools    xAODBase    xAODRootAccess    xAODEventInfo xAODBTagging    PATInterfaces    PathResolver    GoodRunsListsLib    xAODJet    xAODMuon    xAODEgamma    xAODTracking    MuonEfficiencyCorrectionsLib    MuonMomentumCorrectionsLib    MuonSelectorToolsLib    JetCalibToolsLib    JetSelectorToolsLib    JetResolutionLib    JetUncertaintiesLib    AthContainers    ElectronPhotonFourMomentumCorrectionLib    ElectronEfficiencyCorrectionLib    ElectronPhotonSelectorToolsLib    IsolationSelectionLib    METUtilitiesLib    METInterface    AsgTools    xAODMissingET    TauAnalysisToolsLib    xAODTau    PileupReweightingLib    TrigConfxAODLib    TrigDecisionToolLib    JetRecLib    JetEDM    PhotonEfficiencyCorrectionLib    xAODCutFlow    xAODTrigMissingET    xAODTrigEgamma    xAODTrigCalo    ElectronPhotonShowerShapeFudgeToolLib    xAODTrigMuon    JetMomentToolsLib    JetSubStructureUtils    TrigTauMatchingLib    IsolationCorrectionsLib    JetJvtEfficiencyLib    TriggerMatchingToolLib  JetTileCorrectionLib    BoostedJetTaggersLib   InDetTrackSystematicsToolsLib TrigBunchCrossingTool)

atlas_install_generic( data/*
   DESTINATION data PKGNAME_SUBDIR )
