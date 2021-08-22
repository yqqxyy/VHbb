#include <regex>

#include "xAODRootAccess/TEvent.h"

// CxAODTools includes
#include "CxAODTools/Utilities.h"
#include "CxAODTools/BTaggingTool.h"
#include "CxAODTools/CommonProperties.h"

// Infra structure (can be omitted - also included through BTaggingEfficiencyTool.h)
#include "AsgTools/StatusCode.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/SystematicCode.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicSet.h"

#include "PathResolver/PathResolver.h"

#include "EventLoop/StatusCode.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODTools/XSectionProvider.h"

// ROOT includes
#include "TError.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TKey.h"
#include "TRandom3.h"

// STD includes
#include <utility>
#include <vector>
#include <set>
#include <algorithm>

using std::map;
using std::vector;
using std::set;
using std::string;
using CP::CorrectionCode;
using CP::SystematicCode;
using CP::SystematicSet;


StatusCode BTaggingTool::initialize (const Config_t &config, const bool &use2DbTagCut,
                                     const bool &uncorrelate_run1_to_run2,
                                     bool UseQuantile, int maxTruthTag,
                                     const std::string &filename)
{






  auto fileNameCDI = PathResolverFindCalibFile("xAODBTaggingEfficiency/13TeV/" + filename + ".root");




  // NOTE: 2D tagging
  if (use2DbTagCut) fileNameCDI = "$WorkDir_DIR/data/xAODBTaggingEfficiency/AntiKt2TrackJets_20160615.root";

  // determine CDI file type
  //ttbar uncertaintites based on r21 samples, MC/MC SFs, first pseudo-cont. calibration to test
  if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-2018-02-09_v1") != string::npos) m_reference_file_type = CDI_FileType::OFFICIAL_MORIOND_2018;
  if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-2018-05-04_v1") != string::npos) m_reference_file_type = CDI_FileType::OFFICIAL_MAY_2018;
  if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-2018-06-24_v1") != string::npos) m_reference_file_type = CDI_FileType::OFFICIAL_JUN_2018;
  if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-2018-06-29_v1") != string::npos) m_reference_file_type = CDI_FileType::OFFICIAL_JUN_2018;
  if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-2018-10-19_v1") != string::npos) m_reference_file_type = CDI_FileType::OFFICIAL_OCT_2018;
  if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-2019-07-30_v1") != string::npos) m_reference_file_type = CDI_FileType::OFFICIAL_JUL_2019;
  
  //in case the path resolver does not find the file, assume the user gave the full path to a local CDI
  if(fileNameCDI==""){
   fileNameCDI = filename;
   //assume that if the user is using a local CDI it is similar to the latest official CDI
     m_reference_file_type = CDI_FileType::OFFICIAL_OCT_2018;
   //then check some specific options which have custom settings for efficiency maps
   if (fileNameCDI.find("2018-02-09") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_MORIOND_2018;}
   if (fileNameCDI.find("2018-05-04") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_MAY_2018;}
   if (fileNameCDI.find("2018-06-24") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_JUN_2018;}
   if (fileNameCDI.find("2018-06-29") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_JUN_2018;}
   if (fileNameCDI.find("2018-10-19") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_OCT_2018;}
   if (fileNameCDI.find("2019-07-30") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_JUL_2019;}

   if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-August_17_2018")!=string::npos){ m_reference_file_type = CDI_FileType::VHcc_Custom_CDI; }
   if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-May_04_2019.root")!=string::npos){ m_reference_file_type = CDI_FileType::VHcc_Custom_CDI; }
   if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-July10_2019.root")!=string::npos){ m_reference_file_type = CDI_FileType::VHcc_Custom_CDI; }
   if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-October3_2019.root")!=string::npos){ m_reference_file_type = CDI_FileType::VHcc_Custom_CDI; }
   if (fileNameCDI.find("2017-21-13TeV-MC16-CDI-November4-2019.root")!=string::npos){ m_reference_file_type = CDI_FileType::VHcc_Custom_CDI; }

   if (fileNameCDI.find("VHbbVjetsMaps") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_MAY_2018_CustomVHbbMaps;}
   if (fileNameCDI.find("Wjets") != string::npos){m_reference_file_type = CDI_FileType::OFFICIAL_MAY_2018_CustomVHbbMaps;}

  }


  m_uncorrelate_run1_to_run2 = uncorrelate_run1_to_run2;

  // determine which tagger, jet author, and working point combinations to initialize
  auto fileCDI = TFile::Open(fileNameCDI.c_str(), "read");

  for (auto i : *fileCDI->GetListOfKeys()) {
    auto key = static_cast<TKey*>(i);
    TString class_name(key->GetClassName());
    TString tagger_name(key->GetName());

    if ((tagger_name != "VersionInfo") && (class_name == "TDirectoryFile")) {
      auto jet_author_dir = static_cast<TDirectoryFile*>(fileCDI->GetObjectUnchecked(tagger_name.Data()));

      for (auto j : *jet_author_dir->GetListOfKeys()) {
        key        = static_cast<TKey*>(j);
        class_name = key->GetClassName();
        TString jet_author_name(key->GetName());

        if (class_name == "TDirectoryFile") {
          auto working_point_dir = static_cast<TDirectoryFile*>(jet_author_dir->GetObjectUnchecked(jet_author_name.Data()));

          for (auto k : *working_point_dir->GetListOfKeys()) {
            key        = static_cast<TKey*>(k);
            class_name = key->GetClassName();
            TString working_point_name(key->GetName());

            // check validity (directories with few entries are likely not valid)
            if (class_name == "TDirectoryFile") {
              auto flavor_dir = static_cast<TDirectoryFile*>(working_point_dir->GetObjectUnchecked(working_point_name.Data()));

              if (flavor_dir->GetListOfKeys()->GetEntries() < 1) continue;

              for (auto l : *flavor_dir->GetListOfKeys()) {
                key = static_cast<TKey*>(l);
                class_name = key->GetClassName();
                TString flavor_name(key->GetName());

                if (class_name == "TDirectoryFile") {
                  auto calib_dir = static_cast<TDirectoryFile*>(flavor_dir->GetObjectUnchecked(flavor_name.Data()));

                  for (auto m : *calib_dir->GetListOfKeys()) {
                    key = static_cast<TKey*>(m);
                    class_name = key->GetClassName();
                    TString calib_name(key->GetName());

                    if (class_name == "TDirectoryFile") continue;

                    string calib(calib_name.Data());
                    if (calib.find("_Eff") != string::npos &&
                        calib.find("_CalibrationBinned_") == string::npos &&
                        std::count(calib.begin(), calib.end(), '_') == 1) {
                      auto pos = std::find(calib.begin(), calib.end(), '_');
                      calib.erase(pos, calib.end());
                      if (Analysis::is_numeric(calib) && m_flavorDSID[flavor_name.Data()].find(calib) == string::npos) {
                        if(m_flavorDSID[flavor_name.Data()].size() == 0) m_flavorDSID[flavor_name.Data()] += calib;
                        else m_flavorDSID[flavor_name.Data()] += ";" + calib;
                      }
                    }
                  }
                }
              }
            } else continue;

            // efficiency tool
              Info("BTaggingTool::initialize", "Found efficiency tool settings: %s, %s, %s", tagger_name.Data(), jet_author_name.Data(), working_point_name.Data());
              if (m_efficiencyTools.count(tagger_name.Data()) == 0) {
                std::unordered_map < std::string, std::shared_ptr < BTaggingEfficiencyTool >>
                inner { { working_point_name.Data(), nullptr } };
                std::unordered_map < std::string,
                std::unordered_map < std::string, std::shared_ptr<BTaggingEfficiencyTool >>>
                mid { { jet_author_name.Data(), inner } };
                m_efficiencyTools[tagger_name.Data()] = mid;
              }
              else if (m_efficiencyTools[tagger_name.Data()].count(jet_author_name.Data()) == 0) {
                std::unordered_map < std::string, std::shared_ptr < BTaggingEfficiencyTool >>
                inner { { working_point_name.Data(), nullptr } };
                m_efficiencyTools[tagger_name.Data()][jet_author_name.Data()] = inner;
              }
              else {
                m_efficiencyTools[tagger_name.Data()][jet_author_name.Data()][working_point_name.Data()] =
                  nullptr;
              }

              if(working_point_name.Contains("FixedCutBEff") || working_point_name.Contains("CTag") || working_point_name.Contains("Continuous")){
                Info("BTaggingTool::initialize", "Found TruthTaggingTool settings: %s, %s, %s", tagger_name.Data(), jet_author_name.Data(), working_point_name.Data());
                if (m_TruthTaggingTools.count(tagger_name.Data()) == 0) {
                  std::unordered_map < std::string, std::shared_ptr < BTaggingTruthTaggingTool >>
                  inner { { working_point_name.Data(), nullptr } };
                  std::unordered_map < std::string,
                  std::unordered_map < std::string, std::shared_ptr<BTaggingTruthTaggingTool >>>
                  mid { { jet_author_name.Data(), inner } };
                  m_TruthTaggingTools[tagger_name.Data()] = mid;
                }
                else if (m_TruthTaggingTools[tagger_name.Data()].count(jet_author_name.Data()) == 0) {
                  std::unordered_map < std::string, std::shared_ptr < BTaggingTruthTaggingTool >>
                  inner { { working_point_name.Data(), nullptr } };
                  m_TruthTaggingTools[tagger_name.Data()][jet_author_name.Data()] = inner;
                }
                else {
                  m_TruthTaggingTools[tagger_name.Data()][jet_author_name.Data()][working_point_name.Data()] =
                    nullptr;
                }
              }

            // selection tool
            Info("BTaggingTool::initialize", "Found selection tool settings: %s, %s, %s", tagger_name.Data(), jet_author_name.Data(), working_point_name.Data());
            if (m_selectionTools.count(tagger_name.Data()) == 0) {
              std::unordered_map < std::string, std::shared_ptr < BTaggingSelectionTool >>
              inner { { working_point_name.Data(), nullptr } };
              std::unordered_map < std::string,
              std::unordered_map < std::string, std::shared_ptr<BTaggingSelectionTool >>>
              mid { { jet_author_name.Data(), inner } };
              m_selectionTools[tagger_name.Data()] = mid;
            }
            else if (m_selectionTools[tagger_name.Data()].count(jet_author_name.Data()) == 0) {
              std::unordered_map < std::string, std::shared_ptr < BTaggingSelectionTool >>
              inner { { working_point_name.Data(), nullptr } };
              m_selectionTools[tagger_name.Data()][jet_author_name.Data()] = inner;
            }
            else {
              m_selectionTools[tagger_name.Data()][jet_author_name.Data()][working_point_name.Data()] =
                nullptr;
            }
          }
        }
      }
    }
  }
  fileCDI->Close();

  //overwrite m_flavorDSID
  if(m_reference_file_type == CDI_FileType::OFFICIAL_MORIOND_2018){
    //pythia8, sherpa 2.2.1
    m_flavorDSID["B"] = "410501;410250";
    m_flavorDSID["C"] = "410501;410250";
    m_flavorDSID["Light"] = "410501;410250";
    m_flavorDSID["T"] = "410501;410250";
  }
  else if(m_reference_file_type == CDI_FileType::OFFICIAL_MAY_2018 || m_reference_file_type == CDI_FileType::OFFICIAL_JUN_2018){
    //pythia8, sherpa 2.2.1, herwig7
    m_flavorDSID["B"] = "410501;410250;410558";
    m_flavorDSID["C"] = "410501;410250;410558";
    m_flavorDSID["Light"] = "410501;410250;410558";
    m_flavorDSID["T"] = "410501;410250;410558";
  }
  else if(m_reference_file_type == CDI_FileType::OFFICIAL_MAY_2018_CustomVHbbMaps){
    //pythia8, V+jets based sherpa2.2.1, herwig7
    m_flavorDSID["B"] = "410501;364156;410558";
    m_flavorDSID["C"] = "410501;364156;410558";
    m_flavorDSID["Light"] = "410501;364156;410558";
    m_flavorDSID["T"] = "410501;364156;410558";
  }
  else if(m_reference_file_type == CDI_FileType::OFFICIAL_OCT_2018){
    //pythia8, sherpa 2.2.1, herwig7
    m_flavorDSID["B"] = "410470;410250;410558";
    m_flavorDSID["C"] = "410470;410250;410558";
    m_flavorDSID["Light"] = "410470;410250;410558";
    m_flavorDSID["T"] = "410470;410250;410558";
  }
  else if(m_reference_file_type == CDI_FileType::OFFICIAL_JUL_2019){
    //pythia8, sherpa 2.2.1, herwig7
    m_flavorDSID["B"] = "410470;410250;410558";
    m_flavorDSID["C"] = "410470;410250;410558";
    m_flavorDSID["Light"] = "410470;410250;410558";
    m_flavorDSID["T"] = "410470;410250;410558";
  }
  else if(m_reference_file_type == CDI_FileType::VHcc_Custom_CDI){
    
    m_flavorDSID["B"] = "410470;410250;410558;W_jets_a;ttbar_a;s_top_a;Diboson_a;W_jets_d;ttbar_d;s_top_d;Diboson_d;W_jets_e;ttbar_e;s_top_e;Diboson_e";
    m_flavorDSID["C"] = "410470;410250;410558;W_jets_a;ttbar_a;s_top_a;Diboson_a;W_jets_d;ttbar_d;s_top_d;Diboson_d;W_jets_e;ttbar_e;s_top_e;Diboson_e";
    m_flavorDSID["Light"] = "410470;410250;410558;W_jets_a;ttbar_a;s_top_a;Diboson_a;W_jets_d;ttbar_d;s_top_d;Diboson_d;W_jets_e;ttbar_e;s_top_e;Diboson_e";
    m_flavorDSID["T"] = "410470;410250;410558;W_jets_a;ttbar_a;s_top_a;Diboson_a;W_jets_d;ttbar_d;s_top_d;Diboson_d;W_jets_e;ttbar_e;s_top_e;Diboson_e";
  }

  // set default values
  m_tagger      = config.at("TaggerName");
  m_jetAuthor   = config.at("JetAuthor");
  m_btagcuteff  = config.at("OperatingPoint");
  m_scheme      = config.at("Scheme");
  m_rScheme     = config.at("rScheme");
  m_fileNameCDI = fileNameCDI;

  std::cout << " m_btagcuteff " << m_btagcuteff << std::endl;
  std::cout << " m_scheme " << m_scheme << std::endl;

  m_state_change= true;
  m_UseQuantile = UseQuantile;
  m_maxTruthTag = maxTruthTag;
  if(m_scheme.find("Continuous") != string::npos)
    m_useContinuous = true;
  std::cout <<"m_scheme "  <<m_scheme <<std::endl;
  std::cout <<"m_useContinuous " <<m_useContinuous <<std::endl;
  
  // //"veto" working points mean you require the jet to pass the nominal tagger, but fail the "veto-tagger"
  m_taggerName_Veto = "";
  m_OP_Veto = "";
  if( m_btagcuteff.find("Veto") != string::npos ){
    //op string should follow the format nominal_WP_Veto_vetotagger_vetoWP
    //for example, CTag_Loose_Veto_MV2c10_FixedCutBEff_70
    TString vetotaggerstring = m_scheme+"_"+m_btagcuteff;
    TObjArray* wptokens = vetotaggerstring.Tokenize("_");
    if( wptokens->GetEntries() != 6 ){
       	Error("BTaggingTool: improperly formatted WP defintion: ",vetotaggerstring);
        return StatusCode::FAILURE;
    }
    m_taggerName_Veto = (std::string)(((TObjString *)(wptokens->At(3)))->String());
    m_OP_Veto = (std::string)(((TObjString *)(wptokens->At(4)))->String());
    m_OP_Veto = m_OP_Veto+"_"+(std::string)(((TObjString *)(wptokens->At(5)))->String());
  }
 //std::cout << " m_taggerName_Veto " << m_taggerName_Veto << "  m_OP_Veto " << m_OP_Veto << std::endl;



  setTaggerType();

  FillVHccIndexMap();

  return StatusCode::SUCCESS;
} // initialize

BTaggingTool& BTaggingTool::setExcludedBTagEV(const std::string &list)
{
  if (m_useReducedSetOfBTagEV == true) {
    return *this;
  }
  
  m_useReducedSetOfBTagEV = true;
  m_ExcludeEV = list;
  m_state_change= true;
  return *this;
}

BTaggingTool& BTaggingTool::setTagger (const std::string &tagger)
{
  if (m_tagger == tagger) {
    return *this;
  }
  m_tagger = tagger;
  setTaggerType();
  m_state_change= true;
  return *this;
}

BTaggingTool& BTaggingTool::setJetAuthor (const std::string &author)
{
  if (m_jetAuthor == author) {
    return *this;
  }
  m_jetAuthor = author;
  m_state_change= true;
  return *this;
}

BTaggingTool& BTaggingTool::setTaggingScheme (const std::string &scheme)
{
  if (m_scheme == scheme) {
    return *this;
  }
  m_scheme = scheme;
  m_state_change= true;
  return *this;
}

BTaggingTool& BTaggingTool::setTaggerEfficiency (const int &eff)
{
  if (std::stoi(m_btagcuteff) == eff) {
    return *this;
  }
  m_btagcuteff = std::to_string(eff);
  m_state_change= true;
  return *this;

} // setTaggerEfficiency


BTaggingTool& BTaggingTool::setMCIndex (const char *fileName, XSectionProvider &xSectionProvider)
{
  for (auto &p : m_flavorDSID) {
    auto itoken = 0;
    auto tokens = Analysis::tokenize(p.second, ';');
    for (auto &token : tokens) {
      auto index = this->indexMCEfficiencyFromChannel(std::stoul(token), xSectionProvider);
      if (index != m_defaultMCIndex) m_validMCIndicies.insert(index);
      m_actualMC2MCIndex[index] = itoken;
      ++itoken;
    }
  }

  auto index = this->indexMCEfficiencyFromFileName(fileName);

  if (m_validMCIndicies.count(index) == 0)
    Warning("BTaggingTool::setMCIndex", "unable to locate corresponding MC index %u"
        " - will likely cause a segmentation violation", index);
  m_MCIndex = m_actualMC2MCIndex[index];
  return *this;
}

BTaggingTool& BTaggingTool::setMCIndex (const int &mcChannel, XSectionProvider &xSectionProvider)
{

  if(m_reference_file_type == CDI_FileType::OFFICIAL_MORIOND_2018 ||
     m_reference_file_type == CDI_FileType::OFFICIAL_MAY_2018 ||
     m_reference_file_type == CDI_FileType::VHcc_Custom_CDI ||
     m_reference_file_type == CDI_FileType::OFFICIAL_MAY_2018_CustomVHbbMaps || 
     m_reference_file_type == CDI_FileType::OFFICIAL_JUN_2018 ||
     m_reference_file_type == CDI_FileType::OFFICIAL_OCT_2018 || 
     m_reference_file_type == CDI_FileType::OFFICIAL_JUL_2019){
      if (!xSectionProvider.hasMCchannel(mcChannel)){
        m_MCIndex = 0;
      }else{
        auto description = xSectionProvider.getSampleDetailName(mcChannel);
        m_MCIndex = get_MC_index_from_string(description, m_defaultMCIndex);
      }
  }else{
      ///////////////// old way of setting the eff map index //////////////////////
      for (auto &p : m_flavorDSID) {
        auto itoken = 0;
        auto tokens = Analysis::tokenize(p.second, ';');
        for (auto &token : tokens) {
          auto index = this->indexMCEfficiencyFromChannel(std::stoul(token), xSectionProvider);
          if (index != m_defaultMCIndex) m_validMCIndicies.insert(index);
          m_actualMC2MCIndex[index] = itoken;
          ++itoken;
        }
      }

      auto index = this->indexMCEfficiencyFromChannel(mcChannel, xSectionProvider);

      if (m_validMCIndicies.count(index) == 0)
        Warning("BTaggingTool::setMCIndex", "unable to locate corresponding MC index %u"
            " - will likely cause a segmentation violation", index);
      m_MCIndex = m_actualMC2MCIndex[index];
      /////////////////////////////////////////////////////////////////////////
  }
  return *this;
}

float BTaggingTool::getEfficiency (const xAOD::Jet &jet)
{
  if (m_state_change){
    selectTools();}

  // check if tool is initialized
  if (auto tool = m_currentEfficiencyTool.lock()) {
    if (m_MCIndex == 9999 || !tool->setMapIndex(getFlavorLabel(jet), m_MCIndex)) {
      Error("BTaggingTool::getEfficiency",
          "Couldn't set MC/MC index properly. Results will be biasd. Flavor: %s, Index: %u",
          getFlavorLabel(jet).c_str(), m_MCIndex);
    }

    // check eta of jet
    // the tool will initiate a core dump (!) if we feed a jet with
    // |eta| > 2.5 to it... it's a design choice, like it or not...
    if (fabs(jet.eta()) > 2.5) return 0.0;

    // get efficiency
    float eff;
    auto status = CorrectionCode::OutOfValidityRange;
    if(m_useContinuous){
      Analysis::CalibrationDataVariables m_variables;
      m_variables.jetAuthor = m_jetAuthor;
      m_variables.jetPt = jet.pt(); // in MeV
      m_variables.jetEta = jet.eta();
      float tagger_weight;
      Props::MV2c10.get(&jet, tagger_weight);
      m_variables.jetTagWeight = tagger_weight; // here you specify the MV1 or MV1c value of the jet

      status = tool->getEfficiency(getFlavor(jet), m_variables, eff);
      //      std::cout <<Form("author %s pt %f, eta %f, flavorLabel %d, efficiency is: %f", m_variables.jetAuthor.c_str(), m_variables.jetPt, m_variables.jetEta, getFlavor(jet), eff) <<std::endl;
    }
    else
      status = tool->getEfficiency(jet, eff);

    // check return code
    if (status == CorrectionCode::OutOfValidityRange && m_printOutOfRangeWarnings) {
      // out of extrapolation range of tool (high jet pt) - result should not be used
      Warning("BTaggingTool::getEfficiency", "Jet is out of the validity range of the BTaggingEfficiencyTool! "
              "Please exercise caution using this efficiency. Contact expert if unsure how to proceed. "
              "jet details: eta %f, pt %f, eff %f", jet.eta(), jet.pt(), eff);
    }
    else if (status == CorrectionCode::Error) {
      // something went wrong - print error message
      Error("BTaggingTool::getEfficiency()", "tool returned CP::CorrectionCode::Error");
      return 0.;
    }

    // All is okay
    return eff;
  }
  else {
      Error("BTaggingTool::getEfficiency()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return 0.0;
  }
} // getEfficiency

float BTaggingTool::getInefficiency (const xAOD::Jet &jet)
{
  if (m_state_change){
    selectTools();}

  // check if tool is initialized
  if (auto tool = m_currentEfficiencyTool.lock()) {
    // if (m_MCIndex != 9999)
    //   tool->setMapIndex(getFlavorLabel(jet), m_MCIndex);
    if (m_MCIndex == 9999 || !tool->setMapIndex(getFlavorLabel(jet), m_MCIndex)) {
      Error("BTaggingTool::getInefficiency",
          "Couldn't set MC/MC index properly. Results will be biasd. Flavor: %s, Index: %u",
          getFlavorLabel(jet).c_str(), m_MCIndex);
    }

    // check eta of jet
    // the tool will initiate a core dump (!) if we feed a jet with
    // |eta| > 2.5 to it... it's a design choice, like it or not...
    if (fabs(jet.eta()) > 2.5) return 0.0;

    // get efficiency
    float eff;
    auto  status = tool->getInefficiency(jet, eff);

    // check return code
    if (status == CorrectionCode::OutOfValidityRange && m_printOutOfRangeWarnings) {
      // out of extrapolation range of tool (high jet pt) - result should not be used
      Warning("BTaggingTool::getInefficiency()", "Jet is out of the validity range of the BTaggingEfficiencyTool! "
              "Please exercise caution using this efficiency. Contact expert if unsure how to proceed. "
              "jet details: eta %f, pt %f, eff %f", jet.eta(), jet.pt(), eff);
    }
    else if (status == CorrectionCode::Error) {
      // something went wrong - print error message
      Error("BTaggingTool::getInefficiency()", "tool returned CP::CorrectionCode::Error");
      return 0.;
    }

    // All is okay
    return eff;
  }
  else {
      Error("BTaggingTool::getInefficiency()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return 0.0;
  }
} // getInefficiency

float BTaggingTool::getMCEfficiency (const xAOD::Jet &jet)
{
  if (m_state_change){
    selectTools();}

  // check if tool is initialized
  if (auto tool = m_currentEfficiencyTool.lock()) {
    // if (m_MCIndex != 9999)
    //   tool->setMapIndex(getFlavorLabel(jet), m_MCIndex);
    if (m_MCIndex == 9999 || !tool->setMapIndex(getFlavorLabel(jet), m_MCIndex)) {
      Error("BTaggingTool::getMCEfficiency",
          "Couldn't set MC/MC index properly. Results will be biasd. Flavor: %s, Index: %u",
          getFlavorLabel(jet).c_str(), m_MCIndex);
    }

    // check eta of jet
    // the tool will initiate a core dump (!) if we feed a jet with
    // |eta| > 2.5 to it... it's a design choice, like it or not...
    if (fabs(jet.eta()) > 2.5) return 0.0;

    // get efficiency
    float eff;
    auto  status = tool->getMCEfficiency(jet, eff);

    // check return code
    if (status == CorrectionCode::OutOfValidityRange && m_printOutOfRangeWarnings) {
      // out of extrapolation range of tool (high jet pt) - result should not be used
      Warning("BTaggingTool::getMCEfficiency()", "Jet is out of the validity range of the BTaggingEfficiencyTool! "
              "Please exercise caution using this efficiency. Contact expert if unsure how to proceed. "
              "jet details: eta %f, pt %f, eff %f", jet.eta(), jet.pt(), eff);
    }
    else if (status == CorrectionCode::Error) {
      // something went wrong - print error message
      Error("BTaggingTool::getMCEfficiency()", "tool returned CP::CorrectionCode::Error");
      return 0.;
    }

    // All is okay
    return eff;
  }
  else {
      Error("BTaggingTool::getMCEfficiency()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return 0.0;
  }
} // getMCEfficiency

float BTaggingTool::getScaleFactor (const xAOD::Jet &jet)
{
  if (m_state_change){
    selectTools();}

  if (auto tool = m_currentEfficiencyTool.lock()) {
    // if (m_MCIndex != 9999)
    //   tool->setMapIndex(getFlavorLabel(jet), m_MCIndex);
    if (m_MCIndex == 9999 || !tool->setMapIndex(getFlavorLabel(jet), m_MCIndex)) {
      Error("BTaggingTool::getScaleFactor",
          "Couldn't set MC/MC index properly. Results will be biasd. Flavor: %s, Index: %u",
          getFlavorLabel(jet).c_str(), m_MCIndex);
    }

    // check eta of jet
    // the tool will initiate a core dump (!) if we feed a jet with
    // |eta| > 2.5 to it... it's a design choice, like it not...
    if (fabs(jet.eta()) > 2.5) return -1.0;

    // get scale factor
    float sf;
    auto status = CorrectionCode::OutOfValidityRange;
    if(m_useContinuous){
      Analysis::CalibrationDataVariables m_variables;
      m_variables.jetAuthor = m_jetAuthor;
      m_variables.jetPt = jet.pt(); // in MeV
      m_variables.jetEta = jet.eta();
      float tagger_weight;
      Props::MV2c10.get(&jet, tagger_weight);
      m_variables.jetTagWeight = tagger_weight; // here you specify the MV1 or MV1c value of the jet
      status = tool->getScaleFactor(getFlavor(jet), m_variables, sf);
      //      Info("BTaggingTool::getScaleFactor", "scale factor is: %f WP is: %s",  sf, m_workingPoint.c_str());
    }
    else
      status = tool->getScaleFactor(jet, sf);

    // check return code
    if (status == CorrectionCode::OutOfValidityRange && m_printOutOfRangeWarnings) {
      // out of extrapolation range of tool (high jet pt) - result should not be used
      Warning("BTaggingTool::getScaleFactor()", "Jet is out of the validity range of the BTaggingEfficiencyTool! "
              "Please exercise caution using this scale factor. Contact expert if unsure how to proceed. "
              "jet details: eta %f, pt %f, sf %f", jet.eta(), jet.pt(), sf);
    }
    else if (status == CP::CorrectionCode::Error) {
      // something went wrong - print error message
      Error("BTaggingTool::getScaleFactor()", "tool returned CP::CorrectionCode::Error");
      return 0.0;
    }

    // All is okay
    return sf;
  }
  else {
      Error("BTaggingTool::getScaleFactor()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return 0.0;
  }
} // getScaleFactor

float BTaggingTool::getInefficiencyScaleFactor (const xAOD::Jet &jet)
{
  if (m_state_change){
    selectTools();}

  if (auto tool = m_currentEfficiencyTool.lock()) {
    // if (m_MCIndex != 9999)
    //   tool->setMapIndex(getFlavorLabel(jet), m_MCIndex);
    if (m_MCIndex == 9999 || !tool->setMapIndex(getFlavorLabel(jet), m_MCIndex)) {
      Error("BTaggingTool::getInefficiencyScaleFactor",
          "Couldn't set MC/MC index properly. Results will be biasd. Flavor: %s, Index: %u",
          getFlavorLabel(jet).c_str(), m_MCIndex);
    }

    // check eta of jet
    // the tool will initiate a core dump (!) if we feed a jet with
    // |eta| > 2.5 to it... it's a design choice, like it not...
    if (fabs(jet.eta()) > 2.5) return -1.0;

    // get scale factor
    float sf;
    auto status = CorrectionCode::OutOfValidityRange;
    if(m_useContinuous){
      Analysis::CalibrationDataVariables m_variables;
      //      m_variables.flavour = getFlavourLabel(jet);
      m_variables.jetAuthor = m_jetAuthor;
      m_variables.jetPt = jet.pt(); // in MeV
      m_variables.jetEta = jet.eta();
      float tagger_weight;
      Props::MV2c10.get(&jet, tagger_weight);
      m_variables.jetTagWeight = tagger_weight; // here you specify the MV1 or MV1c value of the jet
      status = tool->getScaleFactor(getFlavor(jet), m_variables, sf); //The Continuous SF are modified, no need to use IneffSF
      //       Info("BTaggingTool::getInefficiencyScaleFactor", "IneffScaleFactor is: %f",  sf);
    }
    else    
  status = tool->getInefficiencyScaleFactor(jet, sf);

    // check return code
    if (status == CorrectionCode::OutOfValidityRange && m_printOutOfRangeWarnings) {
      // out of extrapolation range of tool (high jet pt) - result should not be used
      Warning("BTaggingTool::getInefficiencyScaleFactor()", "Jet is out of the validity range of the BTaggingEfficiencyTool! "
              "Please exercise caution using this scale factor. Contact expert if unsure how to proceed. "
              "jet details: eta %f, pt %f, sf %f", jet.eta(), jet.pt(), sf);
    }
    else if (status == CP::CorrectionCode::Error) {
      // something went wrong - print error message
      Error("BTaggingTool::getInefficiencyScaleFactor()", "tool returned CP::CorrectionCode::Error");
      return 0.0;
    }

    // All is okay
    return sf;
  }
  else {
      Error("BTaggingTool::getInefficiencyScaleFactor()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return 0.0;
  }
} // getInefficiencyScaleFactor

float BTaggingTool::getFullEfficiency (const xAOD::Jet &jet, bool tagged)
{
  float eff = 0.0;

  if (tagged) getEfficiency(jet, eff);
  else getInefficiency(jet, eff);
  return eff;
} // getFullEfficiency

float BTaggingTool::getFullScaleFactor (const xAOD::Jet &jet, bool tagged)
{
  float sf = 1.0;

  if (tagged) getScaleFactor(jet, sf);
  else getInefficiencyScaleFactor(jet, sf);
  return sf;
} // getFullScaleFactor

void BTaggingTool::getScaleFactor (const xAOD::JetContainer &jets)
{
  // loop over jets and attach scale factor as decoration
  for (const xAOD::Jet *jet : jets) {
    // attach scale factor
    BTagProps::SF.set(jet, getScaleFactor(*jet));
  }
} // getScaleFactor

int BTaggingTool::getQuantile (const xAOD::Jet &jet)
{
  if (m_state_change){
    selectTools();}
    //////////////////////
    // Cheatsheet:
    // returns 5 if between 60% and 0%
    // returns 4 if between 70% and 60%
    // returns 3 if between 77% and 70%
    // returns 2 if between 85% and 77%
    // returns 1 if between 100% and 85%
    //////////////////////

  if (auto tool = m_currentSelectionTool.lock()) {

    float tagweight = getTaggerWeight(jet);

    if(m_scheme.find("FixedCut")!= string::npos ){

      for(unsigned int i=0; i< m_binEdges.size(); i++){
        if(m_binEdges[i] > tagweight){
            return i+1;
        }
      }
        //if we got here, the jet is tagged in the 60% WP
        return 5;

    }else if(m_scheme.find("Continuous")!= string::npos){

      return tool->getQuantile(jet.pt(), jet.eta(), tagweight);

    }else{
      //only fixed cut and continuous WP supported in getQuantile
      return -1;
    }
  }


  Error("BTaggingTool::getQuantile()", "selection tool is not initialized for "
          "%s (tagger), %s (jet author), and %s (working point)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str());
  return -1;


}

float BTaggingTool::getTaggerWeight (const xAOD::Jet &jet){
  //this function is here to be a non-const interface for an external user of the tool (since it uses selectTools())
  //the private getJetTaggerWeight must be const, since its used inside other const functions.
  if (m_state_change){
    selectTools();}

  return getJetTaggerWeight(jet);

}

float BTaggingTool::getJetTaggerWeight (const xAOD::Jet &jet, bool Get_vetoTagger) const{

   TaggerType taggerEnum = TaggerType::UNKNOWN;
   
   if(!Get_vetoTagger){
   	taggerEnum = m_taggerEnum;
   }else{
   	taggerEnum = m_Veto_taggerEnum;
   }

  if (auto tool = m_currentSelectionTool.lock()) {

        float tagger_weight = -1;

        switch(taggerEnum){

          case TaggerType::UNKNOWN:
	    {
              Error("BTaggingTool::getJetTaggerWeight","tagger type unknown");
              break;
	    }
          case TaggerType::MV2c10:
	    {
	      tagger_weight=Props::MV2c10.get(&jet);
	      break;
	    }
          case TaggerType::DL1:
	    {
	      float tagger_pb = Props::DL1_pb.get(&jet);
	      float tagger_pc = Props::DL1_pc.get(&jet);
	      float tagger_pu = Props::DL1_pu.get(&jet);
	      
	      double tagweight =-99;
	      
	      CorrectionCode code = tool->getTaggerWeight(tagger_pb, tagger_pc, tagger_pu,tagweight);
	      if(code!=CorrectionCode::Ok){
		Error("BTaggingTool::getJetTaggerWeight","failed to get DL1 tag weight");
		return -99;
	      }
	      tagger_weight = (float)tagweight;
	      break;
	    }
	  case TaggerType::DL1r:
	    {
	      float tagger_r_pb = Props::DL1r_pb.get(&jet);
	      float tagger_r_pc = Props::DL1r_pc.get(&jet);
	      float tagger_r_pu = Props::DL1r_pu.get(&jet);
	      
	      double tagrweight =-99;
	      
	      CorrectionCode code_r = tool->getTaggerWeight(tagger_r_pb, tagger_r_pc, tagger_r_pu,tagrweight);
	      if(code_r!=CorrectionCode::Ok){
		Error("BTaggingTool::getJetTaggerWeight","failed to get DL1r tag weight");
		return -99;
	      }
	      tagger_weight = (float)tagrweight;
	      break;
	    }   
	    
        }

        return tagger_weight;

    }

    Error("BTaggingTool::getJetTaggerWeight", "selection tool is not initialized for "
          "%s (tagger), %s (jet author), and %s (working point)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str());
    return -99;

}

bool BTaggingTool::isTagged(const xAOD::Jet &jet){

  if (m_state_change){
    selectTools();}

  return isJetTagged(jet);

}

bool BTaggingTool::isJetTagged (const xAOD::Jet &jet) const
{

  if (auto tool = m_currentSelectionTool.lock()) {

  
    bool useVeto = m_taggerName_Veto!="";

    if (!useVeto) {

		float tagger_weight = getJetTaggerWeight(jet);

		if(!m_useContinuous){     
		  //Info("isTagged()", "pt %f, eta %f", jet.pt(), jet.eta());
	          //Info("isTagged()", "tagger %s, %s, %s,  tagger weight: %f, isTagged? %d", m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), tagger_weight, isTag);
		  return static_cast<bool>(tool->accept(jet.pt(), jet.eta(), tagger_weight));
		}
		else{
		  int effBin = -99;
		  getBinFromEfficiency(effBin);	
		  return static_cast<bool> (tool->getQuantile(jet.pt(), jet.eta(), tagger_weight) >= effBin ? true : false);
		}
      
    }

    if(useVeto){
    	float tagger_weight_nominal = getJetTaggerWeight(jet);
    	float tagger_weight_veto = getJetTaggerWeight(jet,true);
    	return static_cast<bool>(tool->accept(jet.pt(), jet.eta(), tagger_weight_nominal,tagger_weight_veto));
    }
  
  }

    Error("BTaggingTool::isTagged()", "selection tool is not initialized for "
          "%s (tagger), %s (jet author), and %s (working point)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str());
    return false;
     
} // isTagged
  
SystematicSet BTaggingTool::affectingSystematics ()
{
  if (m_state_change)
    selectTools();

  if (auto tool = m_currentEfficiencyTool.lock()) {
    return tool->affectingSystematics();
  }
  else {
      Error("BTaggingTool::affectingSystematics()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme) - returning dummy!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return SystematicSet();
  }
} // affectingSystematics

bool BTaggingTool::applySystematicVariation (const SystematicSet &sysSet)
{
  if (m_state_change)
    selectTools();

  if (auto tool = m_currentEfficiencyTool.lock()) {
    // apply systematic variation
    auto status = tool->applySystematicVariation(sysSet);

    if (status != SystematicCode::Ok) {
      Error("BTaggingTool::applySystematic()", "Could not apply systematic variation!");
      return false;
    }

    // all okay
    return true;
  }
  else {
      Error("BTaggingTool::applySystematic()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return false;
  }
} // applySystematicVariation

bool BTaggingTool::applyNominal ()
{
  if (m_state_change)
    selectTools();

  if (auto tool = m_currentEfficiencyTool.lock()) {
    // switch off systematics
    SystematicSet defaultSet;
    auto status = tool->applySystematicVariation(defaultSet);

    if (status != SystematicCode::Ok) {
      Error("BTaggingTool::applyNominal()", "Could not apply nominal!");
      return false;
    }

    // all okay
    return true;
  }
  else {
      Error("BTaggingTool::applyNominal()", "efficiency tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
    return false;
  }
} // applyNominal


map<string, float> BTaggingTool::computeEventWeight (const vector<const xAOD::Jet*> &xaodjets)
{
  map<string, float> btageffweights;

  // Add nominal
  btageffweights["Nominal"] = 1.0;

  if (m_state_change){
    selectTools();}

  if (auto tool = m_currentEfficiencyTool.lock()) {
    // always compute "Nominal" SF
    float nominal_sf = 1.0;

    for (auto jet : xaodjets) {
      float sf(1.0);

      bool tagged = BTagProps::isTagged.exists(jet) ?
                    static_cast<bool>(BTagProps::isTagged.get(jet)) :
                    isTagged(*jet);

      sf = getFullScaleFactor(*jet, tagged);

      if (sf == -1.0) continue;
      nominal_sf*= sf;
    }

    btageffweights["Nominal"] = nominal_sf;



    // optionally do systematics
    if (this->doWeightVar()) {
     
      for (auto& var : m_affectingSystematics) {
	if(m_useReducedSetOfBTagEV){
	  if(var.name().find("__1down") !=string::npos) continue;
	  if(m_ExcludeEV.find(var.name())!=string::npos) continue;
	}

        auto name = Analysis::replace_all(var.name(), " ", "_");
        SystematicSet set;

        set.insert(var);
        auto sresult = tool->applySystematicVariation(set);
        if (sresult != SystematicCode::Ok) continue;

        // Set all weights to 1
        btageffweights[name] = 1.0;

        float tmp_sf = 1.0;

        for (auto jet : xaodjets) {
          float sf(1.0);

          bool tagged = BTagProps::isTagged.exists(jet) ?
            static_cast<bool>(BTagProps::isTagged.get(jet)) :
            isTagged(*jet);

          sf = getFullScaleFactor(*jet, tagged);
          if (sf == -1.0) continue;
          tmp_sf *= sf;
        }
        btageffweights[name] = tmp_sf;
      }
      // revert to nominal
      applyNominal();
    }
  }
  else {
    Error("BTaggingTool::computeEventWeight()", "efficiency tool is not initialized for "
        "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
        m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
  }
  return btageffweights;
} // computeEventWeight

map<string, float> BTaggingTool::computeEventWeight (xAOD::JetContainer &xaodjets)
{
  std::vector<const xAOD::Jet*> temp;

  for (auto jet : xaodjets) temp.push_back(jet);

  return computeEventWeight(temp);
} // computeEventWeight




std::map<std::string, float> BTaggingTool::computeEventWeight_truthTag (ConfigStore *config){

  if (m_state_change){
    selectTools();}

  bool exclusive = true; //exactly n-required tags, or inclusive, n or more tags.
  bool doHybridTruthTagging = false; //in hybrid mode b-jets are kept "as they are" without applying truth tagging.

  config->getif<bool>("exclusiveTruthTagging", exclusive);
  config->getif<bool>("doHybridTruthTagging", doHybridTruthTagging);


  map<string, float> btageffweights;
  map<string, float> DirectTagEffweights; //for use with hybrid truth tagging

  if(m_hybrid_jets[m_jetAuthor].size() > 0){ // remove if(doHybridTruthTagging) for boosted TT
    DirectTagEffweights = computeEventWeight(m_hybrid_jets[m_jetAuthor]);
  }

  // Add nominal
  btageffweights["Nominal"] = m_truthtag_jets[m_jetAuthor].getEventWeight(exclusive,"Nominal")*m_truthtag_jets_groupB[m_jetAuthor].getEventWeight(exclusive,"Nominal");

  if(m_hybrid_jets[m_jetAuthor].size() > 0){ 
    btageffweights["Nominal"] = btageffweights["Nominal"]*DirectTagEffweights["Nominal"];
  }

  if (m_state_change){
    selectTools();}

  if (auto tool = m_currentEfficiencyTool.lock()) {
    // optionally do systematics
    if (this->doWeightVar()) {
    
      for (auto& var : m_affectingSystematics) {

	if(m_useReducedSetOfBTagEV){
	  if(var.name().find("__1down") !=string::npos) continue;
          if(m_ExcludeEV.find(var.name())!=string::npos) continue;
	}

        //fetch the systematic
        SystematicSet set;
        auto name = Analysis::replace_all(var.name(), " ", "_");
        set.insert(var);
        auto sresult = tool->applySystematicVariation(set);

          //if not retrieved properly, set the weight to Nominal (an okay default.....)
          //remember, this is both event weight and scale factors
          if (sresult != SystematicCode::Ok){
            btageffweights[name] = btageffweights["Nominal"];
            continue;
          }
        btageffweights[name] = m_truthtag_jets[m_jetAuthor].getEventWeight(exclusive,name)*m_truthtag_jets_groupB[m_jetAuthor].getEventWeight(exclusive,name);
        if(m_hybrid_jets[m_jetAuthor].size() > 0){ 
          btageffweights[name] = btageffweights[name]*DirectTagEffweights[name];
        }

      }
      // revert to nominal
      applyNominal();
    }
  }
  else {
    Error("BTaggingTool::computeEventWeight_truthTag()", "efficiency tool is not initialized for "
        "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
        m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
  }
  return btageffweights;
} // computeEventWeight_truthTag



// local helper function that returns an (arbitrary) integer to represent an MC
// generator given a string description
unsigned BTaggingTool::get_MC_index_from_string (const std::string &name,
                                   const unsigned &defaultMCIndex)
{

  if( m_reference_file_type == CDI_FileType::OFFICIAL_MORIOND_2018 ){

    //the avaliable maps are 410501;410250

    // Pythia 8
    if ((name.find("Pythia8_") != string::npos) ||
        (name.find("Pythia8B_") != string::npos) ||
        (name.find("MGPy8EG_") != string::npos) ||
        (name.find("MadGraphPythia8_") != string::npos) ||
        (name.find("MadGraphPythia8EvtGen_") != string::npos) ||
        (name.find("PowhegPythia8EvtGen_CT10_") != string::npos) ||
        (name.find("PowhegPythia8EvtGen_") != string::npos) ||
        (name.find("Pythia8EvtGen_") != string::npos) ||
        (name.find("Pythia8EG_") != string::npos) ||
        (name.find("PwPy8EG_CT10_") != string::npos) ||
        (name.find("PowhegPythia8_") != string::npos) ||
        (name.find("PowhegPythia8EG_") != string::npos) ||
        (name.find("PhPy8EG_CT10_") != string::npos) ||
        (name.find("Py8EG_") != string::npos) ||
        (name.find("Py8") != string::npos)) { return 0; }


    // Sherpa 2.2
    if ((name.find("Sh_22") != string::npos) ||
        (name.find("Sherpa_NNPDF30NNLO") != string::npos) ||
        (name.find("Sherpa_221_NNPDF30NNLO") != string::npos)){ return 1; }


    //for all other samples, use the default map (pythia8).
    return 0;
  }

  if( m_reference_file_type == CDI_FileType::OFFICIAL_MAY_2018 ||  m_reference_file_type == CDI_FileType::VHcc_Custom_CDI || m_reference_file_type == CDI_FileType::OFFICIAL_MAY_2018_CustomVHbbMaps || m_reference_file_type == CDI_FileType::OFFICIAL_JUN_2018 || m_reference_file_type == CDI_FileType::OFFICIAL_OCT_2018 || m_reference_file_type == CDI_FileType::OFFICIAL_JUL_2019 ){ 

    //the avaliable maps are 410501;410250;410558 or 410501;364156;410558

    // Pythia 8
    if ((name.find("Pythia8_") != string::npos) ||
        (name.find("Pythia8B_") != string::npos) ||
        (name.find("MGPy8EG_") != string::npos) ||
        (name.find("MadGraphPythia8_") != string::npos) ||
        (name.find("MadGraphPythia8EvtGen_") != string::npos) ||
        (name.find("PowhegPythia8EvtGen_CT10_") != string::npos) ||
        (name.find("PowhegPythia8EvtGen_") != string::npos) ||
        (name.find("Pythia8EvtGen_") != string::npos) ||
        (name.find("Pythia8EG_") != string::npos) ||
        (name.find("PwPy8EG_CT10_") != string::npos) ||
        (name.find("PowhegPythia8_") != string::npos) ||
        (name.find("PowhegPythia8EG_") != string::npos) ||
        (name.find("PhPy8EG_CT10_") != string::npos) ||
        (name.find("Py8EG_") != string::npos) ||
        (name.find("Py8") != string::npos)) { return 0; }

    // Sherpa 2.2
    if ((name.find("Sh_22") != string::npos) ||
        (name.find("Sherpa_NNPDF30NNLO") != string::npos) ||
        (name.find("Sherpa_221_NNPDF30NNLO") != string::npos)){ return 1; }


    //herwig7
    if ( (name.find("Herwig") != string::npos) ){ return 2; }

    //for all other samples, use the default map (pythia8).
    return 0;
  }


  // Pythia 6
  if ((name.find("PowhegPythia_") != string::npos) ||
      (name.find("PowhegPythiaEvtGen_") != string::npos) ||
      (name.find("AlpgenPythiaEvtGen_") != string::npos) ||
      (name.find("PhPyEG_CT10_") != string::npos) ||
      (name.find("PwPyEG_") != string::npos) ||
      (name.find("Pythia6") != string::npos) ||
      (name.find("Py6") != string::npos)) return 0;

  // Herwig
  if ((name.find("Herwig_") != string::npos) ||
      (name.find("HerwigEG_") != string::npos)) return 1;

  // Sherpa 2.2 (must be checked before Sherpa 2.1)
  if ((name.find("Sh_22") != string::npos) ||
      (name.find("Sherpa_NNPDF30NNLO") != string::npos) ||
      (name.find("Sherpa_221_NNPDF30NNLO") != string::npos)) return 5;

  // Sherpa 2.1 (must be checked after Sherpa 2.2)
  if ((name.find("Sh_CT10_") != string::npos) ||
      (name.find("Sh_") != string::npos) ||
      (name.find("Sherpa_CT10_") != string::npos) ||
      (name.find("Sherpa") != string::npos)) return 2;

  // Pythia 8
  if ((name.find("Pythia8_") != string::npos) ||
      (name.find("Pythia8B_") != string::npos) ||
      (name.find("MGPy8EG_") != string::npos) ||
      (name.find("MadGraphPythia8_") != string::npos) ||
      (name.find("MadGraphPythia8EvtGen_") != string::npos) ||
      (name.find("PowhegPythia8EvtGen_CT10_") != string::npos) ||
      (name.find("PowhegPythia8EvtGen_") != string::npos) ||
      (name.find("Pythia8EvtGen_") != string::npos) ||
      (name.find("Pythia8EG_") != string::npos) ||
      (name.find("PwPy8EG_CT10_") != string::npos) ||
      (name.find("PowhegPythia8_") != string::npos) ||
      (name.find("PowhegPythia8EG_") != string::npos) ||
      (name.find("PhPy8EG_CT10_") != string::npos) ||
      (name.find("Py8EG_") != string::npos) ||
      (name.find("Py8") != string::npos)) return 3;

  // Herwig++
  if ((name.find("aMcAtNLOHerwigppEG_") != string::npos) ||
      (name.find("Herwigpp") != string::npos) ||
      (name.find("HerwigppEG_") != string::npos) ||
      (name.find("PowhegHerwigppEG_") != string::npos)) return 4;

  return defaultMCIndex;
}

unsigned BTaggingTool::indexMCEfficiencyFromFileName (const std::string &name)
{
  return get_MC_index_from_string(name, m_defaultMCIndex);
} // indexMCEfficiencyFromFileName

unsigned BTaggingTool::indexMCEfficiencyFromChannel (const int &mcChannel, XSectionProvider &xSectionProvider)
{
  if (!xSectionProvider.hasMCchannel(mcChannel)) return m_defaultMCIndex;
  auto description = xSectionProvider.getSampleDetailName(mcChannel);

  return get_MC_index_from_string(description, m_defaultMCIndex);
}

void BTaggingTool::truth_tag_jets(unsigned long long eventNumber,const xAOD::JetContainer &signalJets,ConfigStore *m_config, const xAOD::JetContainer &signalJetsDT){
  std::vector<const xAOD::Jet*> temp;
  std::vector<const xAOD::Jet*> tempDT; // for boosted TT
  for (auto jet : signalJets) temp.push_back(jet);
  for (auto jet : signalJetsDT) tempDT.push_back(jet);   
  return truth_tag_jets(eventNumber,temp,m_config,tempDT); 
}

//mimics the functionality of AnalysisReader_VHbb::compute_btagging()
void BTaggingTool::truth_tag_jets(unsigned long long eventNumber,const std::vector<const xAOD::Jet*> &signalJets,ConfigStore *config, const std::vector<const xAOD::Jet*> &signalJetsDT){
  //mimics the functionality of AnalysisReader_VHQQ::compute_btagging()
  if (m_state_change){
    selectTools();}





  int n_RequiredTruthTags=2;
  bool exclusive = true; //exactly n-required tags, or inclusive, n or more tags.
  std::string strategy="AllSignalJets";
  bool doHybridTruthTagging = false; //in hybrid mode b-jets are kept "as they are" without applying truth tagging.
  std::string analysisStrategy = "Resolved";

  config->getif<string>("analysisStrategy", analysisStrategy);
  config->getif<int>("nRequiredTTaggedJets", n_RequiredTruthTags);
  if(analysisStrategy == "Merged"){ 
    config->getif<std::string>("boostedTagStrategy", strategy);  // AllSignalJets,Leading2SignalJets,LeadingSignalJets
  }
  else{
    config->getif<std::string>("tagStrategy", strategy);  // AllSignalJets,Leading2SignalJets,LeadingSignalJets
  }
  config->getif<bool>("exclusiveTruthTagging", exclusive);
  config->getif<bool>("doHybridTruthTagging", doHybridTruthTagging);

  if (auto tool = m_currentTruthTaggingTool.lock()) {
    if (m_MCIndex == 9999 || !tool->setEffMapIndex("B", m_MCIndex) || !tool->setEffMapIndex("C", m_MCIndex)
      || !tool->setEffMapIndex("Light", m_MCIndex) || !tool->setEffMapIndex("T", m_MCIndex)) {
      Error("BTaggingTool::truth_tag_jets",
          "Couldn't set MC/MC index properly. Results will be biasd. Index: %u", m_MCIndex);
    }


  if (this->doWeightVar()) {
    tool->setUseSystematics(true);
  }else{
    tool->setUseSystematics(false);
  }

  //Set the jets to be used in the tool.
  m_truthtag_jets[m_jetAuthor].clear();
  m_truthtag_jets_groupB[m_jetAuthor].clear();


  SelectJetsForTruthTagging(n_RequiredTruthTags,
    m_truthtag_jets[m_jetAuthor], m_truthtag_jets_groupB[m_jetAuthor],
    signalJets,strategy,doHybridTruthTagging,exclusive);

  int randomseed = (int)eventNumber;

  StatusCode status = tool->CalculateResults( m_truthtag_jets[m_jetAuthor].pt,m_truthtag_jets[m_jetAuthor].eta,
    m_truthtag_jets[m_jetAuthor].flav,m_truthtag_jets[m_jetAuthor].tagw, m_truthtag_jets[m_jetAuthor].tt_results,randomseed);

  if(status != StatusCode::SUCCESS){
    Error("BTaggingTool::truth_tag_jets","failed to compute truth tag results");
  }

  if(m_truthtag_jets_groupB[m_jetAuthor].size() > 0){
    status = tool->CalculateResults( m_truthtag_jets_groupB[m_jetAuthor].pt,m_truthtag_jets_groupB[m_jetAuthor].eta,
    m_truthtag_jets_groupB[m_jetAuthor].flav,m_truthtag_jets_groupB[m_jetAuthor].tagw, m_truthtag_jets_groupB[m_jetAuthor].tt_results,randomseed);

    if(status != StatusCode::SUCCESS){
    Error("BTaggingTool::truth_tag_jets","failed to compute truth tag results");
    }
  }

  //decorate the jets with the results of the chosen permutations and randomly generated tag weights
  m_hybrid_jets[m_jetAuthor].clear();
  for(unsigned int ijet=0; ijet< signalJets.size();ijet++){
    BTagProps::eff.set(signalJets[ijet],getEfficiency(*signalJets[ijet]));

    bool hybridjet = false;
    bool istagged = false;
    float tagweight = -99;
    int quantile = -1;
    if(m_truthtag_jets[m_jetAuthor].hasJetIndex(ijet)){
      istagged = m_truthtag_jets[m_jetAuthor].istagged(ijet,exclusive);
      if(m_UseQuantile){
        tagweight = m_truthtag_jets[m_jetAuthor].tagweight(ijet,exclusive);
        quantile = m_truthtag_jets[m_jetAuthor].quantile(ijet,exclusive);
      }

    }else if(m_truthtag_jets_groupB[m_jetAuthor].hasJetIndex(ijet)){
      istagged = m_truthtag_jets_groupB[m_jetAuthor].istagged(ijet,exclusive);
      if(m_UseQuantile){
        tagweight = m_truthtag_jets_groupB[m_jetAuthor].tagweight(ijet,exclusive);
        quantile = m_truthtag_jets_groupB[m_jetAuthor].quantile(ijet,exclusive);
      }
    }else{
      //if we got here, these must be b-jets and we must be in hybrid mode
      //so just give the jets their "real" properties
      istagged = isJetTagged(*signalJets[ijet]);
      tagweight = getJetTaggerWeight(*signalJets[ijet]);
      quantile = getQuantile(*signalJets[ijet]);
      hybridjet = true;
    }

    BTagProps::isTagged.set(signalJets[ijet],istagged);
    BTagProps::tagWeight.set(signalJets[ijet],tagweight);
    BTagProps::Quantile.set(signalJets[ijet],quantile);

    if(hybridjet){  m_hybrid_jets[m_jetAuthor].push_back(signalJets[ijet]); }

  }

  // here add to hybrid jets the jets on which I want to apply DT (e.g. in VHbb boosted analysis, the track jets outside the fat jet. This is not the same thing as hybrid tagging, but same technical treatment. 
  for(unsigned int ijet=0; ijet< signalJetsDT.size();ijet++){
    BTagProps::eff.set(signalJetsDT[ijet],getEfficiency(*signalJetsDT[ijet]));

    bool istagged = false;
    float tagweight = -99;
    int quantile = -1;
    istagged = isJetTagged(*signalJetsDT[ijet]);
    tagweight = getJetTaggerWeight(*signalJetsDT[ijet]);
    quantile = getQuantile(*signalJetsDT[ijet]);

    BTagProps::isTagged.set(signalJetsDT[ijet],istagged);
    BTagProps::tagWeight.set(signalJetsDT[ijet],tagweight);
    BTagProps::Quantile.set(signalJetsDT[ijet],quantile);

    m_hybrid_jets[m_jetAuthor].push_back(signalJetsDT[ijet]);
  }

  }else {
      Error("BTaggingTool::truth_tag_jets()", "truth tag tool is not initialized for "
          "%s (tagger), %s (jet author), %s (working point), and %s (efficiency scheme)!",
          m_tagger.c_str(), m_jetAuthor.c_str(), m_workingPoint.c_str(), m_scheme.c_str());
  }
}

void BTaggingTool::SelectJetsForTruthTagging(int n_RequiredTruthTags,
  TruthTagJets& tt_jets, TruthTagJets& tt_jets_groupB,
  const std::vector<const xAOD::Jet*> &signalJets, std::string strategy,bool doHybridTruthTagging, bool exclusive) const{

  if(strategy=="AllSignalJets"){
    int n_tagged_bjets=0;

    for(unsigned int ijet=0; ijet< signalJets.size();ijet++){
      int label = getFlavor(*signalJets[ijet]);

      if( doHybridTruthTagging && fabs(label)==5 ){
        if(isJetTagged(*signalJets[ijet])){ n_tagged_bjets++; }
        continue;
      }

      tt_jets.pt.push_back(signalJets[ijet]->pt());
      tt_jets.eta.push_back(signalJets[ijet]->eta());
      tt_jets.flav.push_back(label);
      tt_jets.tagw.push_back(getJetTaggerWeight(*signalJets[ijet])); //the tag weight is not needed for truth tagging, just for direct tagging.
      tt_jets.signaljet_index.push_back(ijet);
    }

    if(!doHybridTruthTagging){
      tt_jets.n_requiredTags =   n_RequiredTruthTags<= tt_jets.size() ? n_RequiredTruthTags : tt_jets.size();
    }

    if(doHybridTruthTagging){
      int required = n_RequiredTruthTags-n_tagged_bjets;
      if( required < 0 ){ required = 0; }
      tt_jets.n_requiredTags =   required<= tt_jets.size() ? required : tt_jets.size();
    }
  } //AllSignalJets


  if(strategy=="LeadingSignalJets"){
    //deal with the leading pt jet
    bool leadJetIsTagged = false;

    if( !(doHybridTruthTagging && getFlavorLabel(*signalJets[0])=="B") ){
      int label = getFlavor(*signalJets[0]);

      tt_jets.pt.push_back(signalJets[0]->pt());
      tt_jets.eta.push_back(signalJets[0]->eta());
      tt_jets.flav.push_back(label);
      tt_jets.tagw.push_back(getJetTaggerWeight(*signalJets[0])); //the tag weight is not needed for truth tagging, just for direct tagging.
      tt_jets.signaljet_index.push_back(0);
      if(n_RequiredTruthTags==0){
        tt_jets.n_requiredTags = 0;
      }else{
        tt_jets.n_requiredTags = 1;
        leadJetIsTagged = true;
      }
    }else{
      leadJetIsTagged = isJetTagged(*signalJets[0]);
    }

    //now deal with the rest
    int n_tagged_bjets=0;


    for(unsigned int ijet=1; ijet< signalJets.size();ijet++){
      int label = getFlavor(*signalJets[ijet]);

      if( doHybridTruthTagging && fabs(label)==5 ){

        if(isJetTagged(*signalJets[ijet])){ n_tagged_bjets++; }
        continue;
      }

      tt_jets_groupB.pt.push_back(signalJets[ijet]->pt());
      tt_jets_groupB.eta.push_back(signalJets[ijet]->eta());
      tt_jets_groupB.flav.push_back(label);
      tt_jets_groupB.tagw.push_back(getJetTaggerWeight(*signalJets[ijet])); //the tag weight is not needed for truth tagging, just for direct tagging.
      tt_jets_groupB.signaljet_index.push_back(ijet);
    }

    int nrequired = n_RequiredTruthTags;
    if(leadJetIsTagged){ nrequired = nrequired -1; }
    if(doHybridTruthTagging){ nrequired = nrequired - n_tagged_bjets; }
    if(nrequired < 0){ nrequired = 0; }

    tt_jets_groupB.n_requiredTags =   nrequired<= tt_jets_groupB.size() ? nrequired : tt_jets_groupB.size();

  } //LeadingSignalJets

  if(strategy=="Leading2SignalJets"){
    //deal with the 2 leading pt jets
    int ntags_leading2 = 0;

    for (unsigned int i = 0; i < signalJets.size(); i++)
    {
      if(i > 1){ break; }
      if( !(doHybridTruthTagging && getFlavorLabel(*signalJets[i])=="B") ){
        int label = getFlavor(*signalJets[i]);
        tt_jets.pt.push_back(signalJets[i]->pt());
        tt_jets.eta.push_back(signalJets[i]->eta());
        tt_jets.flav.push_back(label);
        tt_jets.tagw.push_back(getJetTaggerWeight(*signalJets[i])); //the tag weight is not needed for truth tagging, just for direct tagging.
        tt_jets.signaljet_index.push_back(i);
       }else{
        if( isJetTagged(*signalJets[i]) ){ ntags_leading2++; }
       }
    }

    int required = n_RequiredTruthTags;
    if(required > 2){ required =2; }
    required = required - ntags_leading2;
    if(required < 0){ required =0; }

    tt_jets.n_requiredTags =   required<= tt_jets.size() ? required : tt_jets.size();
    //if we are in the inclusive case, we don't care whats up with the rest of the jets, we can leave them as they are.
    if(!exclusive){ return; }
    if(signalJets.size() <=2 ){ return; }

    ntags_leading2 = ntags_leading2+ tt_jets.n_requiredTags;
    //in the exclusive case, we require the rest of the jets to have n_RequiredTruthTags-ntags_leading2
    //in hybrid mode, require those tags - n_tagged_bjets

    int n_tagged_bjets=0;

    for(unsigned int ijet=2; ijet< signalJets.size();ijet++){
      int label = getFlavor(*signalJets[ijet]);

      if( doHybridTruthTagging && fabs(label)==5 ){

        if(isJetTagged(*signalJets[ijet])){ n_tagged_bjets++; }
        continue;
      }

      tt_jets_groupB.pt.push_back(signalJets[ijet]->pt());
      tt_jets_groupB.eta.push_back(signalJets[ijet]->eta());
      tt_jets_groupB.flav.push_back(label);
      tt_jets_groupB.tagw.push_back(getJetTaggerWeight(*signalJets[ijet])); //the tag weight is not needed for truth tagging, just for direct tagging.
      tt_jets_groupB.signaljet_index.push_back(ijet);

    }

    required = n_RequiredTruthTags-ntags_leading2;
    if(doHybridTruthTagging){ required = required - n_tagged_bjets; }
    if(required < 0){ required = 0; }

    tt_jets_groupB.n_requiredTags =   required<= tt_jets_groupB.size() ? required : tt_jets_groupB.size();

  } //Leading2SignalJets


}



StatusCode BTaggingTool::initializeEffTool (std::shared_ptr<BTaggingEfficiencyTool> &eff_ptr,
                                            const string &alg, const string &author, const string &wp)
{
  // TODO CDI ignores this
  MSG::Level m_msgLevel = MSG::WARNING;

  eff_ptr = std::make_shared<BTaggingEfficiencyTool>(
    Form("BTaggingEfficiencyTool.%s.%s.%s", alg.c_str(), author.c_str(), wp.c_str()));

  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("TaggerName", alg.c_str()));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("OperatingPoint", wp.c_str()));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("JetAuthor", author.c_str()));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("ScaleFactorFileName", m_fileNameCDI.c_str()));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("ConeFlavourLabel", true));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("OldConeFlavourLabel", false));
  if (m_flavorDSID.count("B") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EfficiencyBCalibrations", m_flavorDSID["B"]));
  if (m_flavorDSID.count("C") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EfficiencyCCalibrations", m_flavorDSID["C"]));
  if (m_flavorDSID.count("T") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EfficiencyTCalibrations", m_flavorDSID["T"]));
  if (m_flavorDSID.count("Light") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EfficiencyLightCalibrations", m_flavorDSID["Light"]));

  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EigenvectorReductionB", m_rScheme));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EigenvectorReductionC", m_rScheme));
  TOOL_CHECK("BTaggingTool::initializeEffTool()", eff_ptr->setProperty("EigenvectorReductionLight", m_rScheme));
  
  Info("BTaggingTool::initializeSelectTool", "Initializing efficiency tool: %s, %s, %s", alg.c_str(), author.c_str(), wp.c_str());
  

  eff_ptr->msg().setLevel(m_msgLevel);
  EL_CHECK("BTaggingTool::initialize()", eff_ptr->initialize());

  return StatusCode::SUCCESS;
} // initializeEffTool

StatusCode BTaggingTool::initializeTruthTagTool (std::shared_ptr<BTaggingTruthTaggingTool> &tt_ptr,
                                            const string &alg, const string &author, const string &wp)
{
  // TODO CDI ignores this
  MSG::Level m_msgLevel = MSG::WARNING;

  tt_ptr = std::make_shared<BTaggingTruthTaggingTool>(
    Form("BTaggingTruthTaggingTool.%s.%s.%s", alg.c_str(), author.c_str(), wp.c_str()));

  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("TaggerName", alg.c_str()));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("OperatingPoint", wp.c_str()));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("JetAuthor", author.c_str()));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("ScaleFactorFileName", m_fileNameCDI.c_str()));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("ConeFlavourLabel", true));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("OldConeFlavourLabel", false));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("EigenvectorReductionB", m_rScheme));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("EigenvectorReductionC", m_rScheme));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("EigenvectorReductionLight", m_rScheme));
  //TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("EigenvectorReductionLight", m_rScheme));

  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("IgnoreScaleFactors",false));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("UsePermutations",true));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("UseQuantile", m_UseQuantile ));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("UseSystematics", true ));
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("MaxNtagged", m_maxTruthTag ) );
  TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("ExcludeSpecificEigens", m_ExcludeEV));
  
  if(m_useContinuous){ //working point for the selection
    TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("CutBenchmark", Form("FixedCutBEff_%s", m_btagcuteff.c_str())));
    TOOL_CHECK("BTaggingTool::initializeTruthTagTool()", tt_ptr->setProperty("StoreOnlyUpVariations", m_useReducedSetOfBTagEV));
    std::cout <<"CutBenchmark " <<Form("FixedCutBEff_%s", m_btagcuteff.c_str()) <<std::endl;
  }

  if (m_flavorDSID.count("B") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", tt_ptr->setProperty("EfficiencyBCalibrations", m_flavorDSID["B"]));
  if (m_flavorDSID.count("C") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", tt_ptr->setProperty("EfficiencyCCalibrations", m_flavorDSID["C"]));
  if (m_flavorDSID.count("T") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", tt_ptr->setProperty("EfficiencyTCalibrations", m_flavorDSID["T"]));
  if (m_flavorDSID.count("Light") != 0) TOOL_CHECK("BTaggingTool::initializeEffTool()", tt_ptr->setProperty("EfficiencyLightCalibrations", m_flavorDSID["Light"]));

  Info("BTaggingTool::initializeSelectTool", "Initializing truthtagging tool: %s, %s, %s", alg.c_str(), author.c_str(), wp.c_str());

  tt_ptr->msg().setLevel(m_msgLevel);
  EL_CHECK("BTaggingTool::initialize()", tt_ptr->initialize());

  return StatusCode::SUCCESS;
} // initializeTruthTagTool



StatusCode BTaggingTool::initializeSelectTool (std::shared_ptr<BTaggingSelectionTool> &select_ptr,
                                               const std::string &alg, const std::string &author, const std::string &wp)
{
  // TODO CDI ignores this
  MSG::Level m_msgLevel = MSG::WARNING;

  select_ptr = std::make_shared<BTaggingSelectionTool>(
    Form("BTaggingSelectionTool.%s.%s.%s", alg.c_str(), author.c_str(), wp.c_str()));

  // auto GeV = 1000.0;

    TOOL_CHECK("BTaggingTool::initializeSelectTool()", select_ptr->setProperty("TaggerName", alg.c_str()));
    TOOL_CHECK("BTaggingTool::initializeSelectTool()", select_ptr->setProperty("OperatingPoint", wp.c_str()));
    TOOL_CHECK("BTaggingTool::initializeSelectTool()", select_ptr->setProperty("JetAuthor", author.c_str()));
    TOOL_CHECK("BTaggingTool::initializeSelectTool()", select_ptr->setProperty("FlvTagCutDefinitionsFileName", m_fileNameCDI.c_str()));
    Info("BTaggingTool::initializeSelectTool", "Initializing selection tool: %s, %s, %s", alg.c_str(), author.c_str(), wp.c_str());

    if(wp.find("FixedCut") != string::npos ){
      //for getting the quantile, extract the cut values from the CDI

      TFile* inf = TFile::Open(m_fileNameCDI.c_str(), "read");
      std::vector<std::string> availableOP_fixCut= {"FixedCutBEff_85", "FixedCutBEff_77","FixedCutBEff_70","FixedCutBEff_60"};

      m_binEdges.clear();
      for(unsigned int iop=0; iop< availableOP_fixCut.size(); iop++){
          TString cutname = m_tagger+"/"+m_jetAuthor+"/"+availableOP_fixCut.at(iop)+"/cutvalue";
          float cutval = ((TVector*) inf->Get(cutname))[0](0);
          m_binEdges.push_back(cutval);
      }

      inf->Close();

    }

  select_ptr->msg().setLevel(m_msgLevel);
  EL_CHECK("BTaggingTool::initializeSelectTool()", select_ptr->initialize());

  return StatusCode::SUCCESS;
} // initializeSelectTool


int BTaggingTool::getFlavor (const xAOD::Jet &jet) const
{
  int label = -1;

  if (Props::HadronConeExclTruthLabelID.exists(&jet)) Props::HadronConeExclTruthLabelID.get(&jet, label);
  else if (Props::TruthLabelID.exists(&jet)) Props::TruthLabelID.get(&jet, label);
  else if (jet.getAttribute("HadronConeExclTruthLabelID", label)) {}
  else if (jet.getAttribute("TruthLabelID", label)) {}

  return label;
} // getFlavor


string BTaggingTool::getFlavorLabel (const xAOD::Jet &jet) const
{
  int label = getFlavor(jet);

  if ((label == 5) || (label == -5)) return "B";

  if ((label == 4) || (label == -4)) return "C";

  if ((label == 15) || (label == -15)) return "T";
  else return "Light";
} // getFlavorLabel

void BTaggingTool::setTaggerType ()
{
  
  if (m_tagger == "MV2c10") m_taggerEnum = TaggerType::MV2c10;
  else if (m_tagger == "DL1") m_taggerEnum = TaggerType::DL1;
  else if (m_tagger == "DL1r") m_taggerEnum = TaggerType::DL1r;
  else m_taggerEnum = TaggerType::UNKNOWN;

  if(m_taggerName_Veto!=""){
	  if (m_taggerName_Veto == "MV2c10") m_Veto_taggerEnum = TaggerType::MV2c10;
	  else if (m_taggerName_Veto == "DL1") m_Veto_taggerEnum = TaggerType::DL1;
	  else if (m_taggerName_Veto == "DL1r") m_Veto_taggerEnum = TaggerType::DL1r;
	  else m_Veto_taggerEnum = TaggerType::UNKNOWN;
  }
} // setTaggerType

void BTaggingTool::selectTools ()
{
  m_currentEfficiencyTool.reset();
  m_currentSelectionTool.reset();
  m_currentTruthTaggingTool.reset();

  if(m_useContinuous)
    m_workingPoint = "Continuous";
  else
    m_workingPoint = m_btagcuteff;


  for (auto &outer : m_efficiencyTools) {
    if (outer.first != m_tagger) continue;
    for (auto &mid : outer.second) {
      if (mid.first != m_jetAuthor) continue;
      for (auto &inner : mid.second) {

          if ((!m_useContinuous && inner.first.find(m_scheme) != string::npos &&
	       (inner.first.find(m_btagcuteff) != string::npos)) ||
               (m_useContinuous && inner.first.find("Continuous")!= string::npos)) {
	    if (!inner.second) {
              auto status = initializeEffTool(inner.second,
                                              outer.first, mid.first, inner.first);
              
              if (status != StatusCode::SUCCESS) {
                Error("selectTools", "couldn't initialize requested efficiency tool");
                PrintConfiguration();
              }
            }
            m_currentEfficiencyTool = inner.second;
            auto tool = m_currentEfficiencyTool.lock();
            m_affectingSystematics = tool->affectingSystematics();
          }
        
      }
    }
  }

  if (m_currentEfficiencyTool.expired())
              Error("selectTools",
          "couldn't find suitable b-tagging efficiency tool - efficiencies unavailable");

  for (auto &outer : m_selectionTools) {
    if (outer.first != m_tagger) continue;
    for (auto &mid : outer.second) {
      if (mid.first != m_jetAuthor) continue;
      for (auto &inner : mid.second) {
        if ((!m_useContinuous && inner.first.find(m_scheme) != string::npos &&
	     (inner.first.find(m_btagcuteff) != string::npos)) ||
	    (m_useContinuous && inner.first == "Continuous")) {
          if (!inner.second) {
            auto status = initializeSelectTool(inner.second,
                                               outer.first, mid.first, inner.first);

            if (status != StatusCode::SUCCESS) {
              Error("selectTools", "couldn't initialize requested selection tool");
              PrintConfiguration();
            }
          }
          m_currentSelectionTool = inner.second;
        }
      }
    }
  }

  if (m_currentSelectionTool.expired())
    Error("selectTools",
          "couldn't find suitable b-tagging selection tool - tagging unavailable");

  //truth tagging tools
  for (auto &outer : m_TruthTaggingTools) {
    if (outer.first != m_tagger) continue;
    for (auto &mid : outer.second) {
      if (mid.first != m_jetAuthor) continue;
      for (auto &inner : mid.second) {
	if ((!m_useContinuous && (inner.first.find(m_scheme) != string::npos) &&
             (inner.first.find(m_btagcuteff) != string::npos)) || (m_useContinuous && inner.first == "Continuous") ){
            if (!inner.second) {  
            auto status = initializeTruthTagTool(inner.second,
                                               outer.first, mid.first, inner.first);

            if (status != StatusCode::SUCCESS) {
              Error("selectTools", "couldn't initialize requested truthtagging tool");
              PrintConfiguration();
            }  
          }
          m_currentTruthTaggingTool = inner.second;
        }
      }
    }
  }

  if (m_currentTruthTaggingTool.expired())
    Error("TruthTaggingTool",
          "couldn't find suitable b-tagging TruthTaggingTool - unavailable");

  m_state_change = false;
}

void BTaggingTool::getBinFromEfficiency(int &effbin) const{
   if(m_btagcuteff == "60") effbin = 5;
  else if(m_btagcuteff == "70")  effbin = 4;
  else if(m_btagcuteff == "77")  effbin = 3;
  else if(m_btagcuteff == "85")  effbin = 2;
  else if(m_btagcuteff == "100") effbin = 1;
  else{
    std::cout << "Unknown efficiency case, unable to get the quantile" << std::endl;
    effbin = -1;
  }
}


void BTaggingTool::setMCIndex_VHcc (const int &mcChannel, std::string period){

     
      m_MCIndex_TruthTag=m_sample_to_map_index[period][mcChannel];

}

void BTaggingTool::tagjet_selection_vhcc(std::vector<const xAOD::Jet *> &signalJets,
                        std::vector<const xAOD::Jet *> &selectedJets,
                        int &tagcatExcl, int &nSubleadingTags) {
  
  selectedJets.clear();

  auto seltool_veto = m_selectionTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto];

  if(!seltool_veto){ auto status = initializeSelectTool(m_selectionTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto],m_taggerName_Veto,m_jetAuthor,m_OP_Veto); 
                       if (status != StatusCode::SUCCESS) { Error("VHcc","error in loading selection tool "); } 
                       seltool_veto = m_selectionTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto];
                     }
  /////////////////////////////////////////////////////////////
  // **C-Tagging Selection**

  int nSignalJets = signalJets.size();
  
  int nctag = 0;
  nSubleadingTags = 0;

  const xAOD::Jet *Jet1 = nullptr;
  const xAOD::Jet *Jet2 = nullptr;
  const xAOD::Jet *Jet3 = nullptr;

  if (nSignalJets >= 1) Jet1 = signalJets.at(0);
  if (nSignalJets >= 2) Jet2 = signalJets.at(1);
  if (nSignalJets >= 3) Jet3 = signalJets.at(2);

  // fill selected jets
  //------------------
  if (Jet1) {
    selectedJets.push_back(Jet1);
    BTagProps::isTagged.set( Jet1, isTagged(*Jet1) );
    if (BTagProps::isTagged.get(Jet1)) nctag++;
  }
  if (Jet2) {
    selectedJets.push_back(Jet2);
    BTagProps::isTagged.set( Jet2, isTagged(*Jet2) );
    if (BTagProps::isTagged.get(Jet2)) nctag++;
  }
  if (Jet3) {
    selectedJets.push_back(Jet3);
  }


  tagcatExcl = nctag;

  // In case the additional jets are b-tagged we reject the event
  for (int i = 2; i < nSignalJets; i++) {
    const xAOD::Jet *jet = signalJets.at(i);
    float jMV2c10 = Props::MV2c10.get(jet);


    if (seltool_veto->accept(jet->pt(), jet->eta(), jMV2c10)){
      nSubleadingTags++;
    } 
  }
}

void BTaggingTool::tagjet_selection_vhcc(std::vector<const xAOD::Jet *> &signalJets, std::vector<const xAOD::Jet *> &fwdJets, 
                        std::vector<const xAOD::Jet *> &selectedJets,
                        int &tagcatExcl, int &nSubleadingTags) {
  
  selectedJets.clear();

  auto seltool_veto = m_selectionTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto];

  if(!seltool_veto){ auto status = initializeSelectTool(m_selectionTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto],m_taggerName_Veto,m_jetAuthor,m_OP_Veto); 
                       if (status != StatusCode::SUCCESS) { Error("VHcc","error in loading selection tool "); } 
                       seltool_veto = m_selectionTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto];
                     }
  /////////////////////////////////////////////////////////////
  // **C-Tagging Selection**

  int nSignalJets = signalJets.size();
  int nFwdJets = fwdJets.size();
  
  int nctag = 0;
  nSubleadingTags = 0;

  const xAOD::Jet *Jet1 = nullptr;
  const xAOD::Jet *Jet2 = nullptr;
  const xAOD::Jet *Jet3 = nullptr;

  if (nSignalJets >= 1) Jet1 = signalJets.at(0);
  if (nSignalJets >= 2) Jet2 = signalJets.at(1);
  if (nSignalJets >= 3){ Jet3 = signalJets.at(2);}
  else if(nSignalJets==2 && nFwdJets>0){ Jet3 =fwdJets.at(0);}

  // fill selected jets
  //------------------
  if (Jet1) {
    selectedJets.push_back(Jet1);
    BTagProps::isTagged.set( Jet1, isTagged(*Jet1) );
    if (BTagProps::isTagged.get(Jet1)) nctag++;
  }
  if (Jet2) {
    selectedJets.push_back(Jet2);
    BTagProps::isTagged.set( Jet2, isTagged(*Jet2) );
    if (BTagProps::isTagged.get(Jet2)) nctag++;
  }
  if (Jet3) {
    selectedJets.push_back(Jet3);
  }


  tagcatExcl = nctag;

  // In case the additional jets are b-tagged we reject the event
  for (int i = 2; i < nSignalJets; i++) {
    const xAOD::Jet *jet = signalJets.at(i);
    float jMV2c10 = Props::MV2c10.get(jet);


    if (seltool_veto->accept(jet->pt(), jet->eta(), jMV2c10)){
      nSubleadingTags++;
    } 
  }
}


void BTaggingTool::GetVhccEffs(std::vector<float> &jet_effs, std::vector<float> &jet_ineffs,std::shared_ptr<BTaggingEfficiencyTool> efftool_nominal, 
                                    std::shared_ptr<BTaggingEfficiencyTool> efftool_veto,std::vector<const xAOD::Jet*> &xaodjets,
                                    float &veto_ineff_sf, float &direct_tag_weight){
      //

      int njets = xaodjets.size();

      float eff = 1.0;
      float ineff = 1.0;
      float sf = 1.0;
      float ineff_sf = 1.0;


      // compute eff, ineff, scale factor and ineff scale factors on 2 leading jets
      for(int jet_i=0;jet_i<2;jet_i++){
        const xAOD::Jet* jet= xaodjets.at(jet_i);
        auto status = efftool_nominal->getEfficiency(*jet,eff);
        if (status == CorrectionCode::Error){ Error("VHcc truth tagging","error in getEfficiency");  }
        status = efftool_nominal->getScaleFactor(*jet,sf);
        if (status == CorrectionCode::Error){ Error("VHcc truth tagging","error in getScaleFactor");  }
        status =  efftool_nominal->getInefficiency(*jet,ineff);
        if (status == CorrectionCode::Error){ Error("VHcc truth tagging","error in getInefficiency");  }
        status =  efftool_nominal->getInefficiencyScaleFactor(*jet,ineff_sf);
        if (status == CorrectionCode::Error){ Error("VHcc truth tagging","error in getInefficiencyScaleFactor");  }

        // store efficiencies and inefficiencies for 2 leading jets
        jet_effs.push_back(eff);
        jet_ineffs.push_back(ineff);

        // compute direct tag if jet is tagged or not
        if(isJetTagged(*jet)){
          direct_tag_weight*=sf;
        }else{
          direct_tag_weight*=ineff_sf;
        }
      }

      
      // compute b-tag veto inefficiency scale factors on 3+jets
      if(njets > 2){
        for(int jet_i=2;jet_i<njets;jet_i++){
          const xAOD::Jet* jet= xaodjets.at(jet_i);
          auto status =  efftool_veto->getInefficiencyScaleFactor(*jet,ineff_sf);
          if (status == CorrectionCode::Error){ Error("VHcc truth tagging","error in getInefficiencyScaleFactor");  }

          // calculate the b-tag veto ineff scale factors
          veto_ineff_sf*=ineff_sf;
          // also apply on direct tagging event weight
          direct_tag_weight*=ineff_sf;
        }
      }

}


std::map< std::string, std::map<int, float> >  BTaggingTool::compute_TruthTag_EventWeight_VHcc(std::vector<const xAOD::Jet*> &xaodjets,bool doSyst, const float& correction_dR_jet1, const float& correction_dR_jet2){
      //this function computes the event weights (direct tag and truth tag) for the VHcc style of doing truth tagging 
      //(filling each event to multiple histograms, 0, 1 and 2 tag, and whatever direct tag histograms )
      //it fill the map eventweights with event weights for 0,1,2 truth tags, and the -1 entry is for the direct tag histgoram (whatever that is)

      std::map< std::string, std::map<int, float> > eventweights;

      //set up the tools we need:
      
      std::string nominal_wp = m_scheme+"_"+m_workingPoint;

      auto efftool_nominal = m_efficiencyTools[m_tagger][m_jetAuthor][nominal_wp];
      auto efftool_veto = m_efficiencyTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto];

      if(!efftool_nominal){auto status = initializeEffTool(m_efficiencyTools[m_tagger][m_jetAuthor][nominal_wp]
                                                            ,m_tagger,m_jetAuthor,nominal_wp); 
                              if (status != StatusCode::SUCCESS) { Error("VHcc truth tagging","error in loading efficiency tool "); } 
                            efftool_nominal = m_efficiencyTools[m_tagger][m_jetAuthor][m_workingPoint];
                          }
      if(!efftool_veto){ auto status = initializeEffTool(m_efficiencyTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto],m_taggerName_Veto,m_jetAuthor,m_OP_Veto); 
                           if (status != StatusCode::SUCCESS) { Error("VHcc truth tagging","error in loading efficiency tool "); } 
                           efftool_veto = m_efficiencyTools[m_taggerName_Veto][m_jetAuthor][m_OP_Veto];
                         }

      
      int njets = xaodjets.size();

      if(njets<2){
        return eventweights;
      }
      // CDI setup for ttbar based MC/MC scale factors and inefficiency scale factors
      // In order to use custom maps, please replace m_MCIndex by m_MCIndex_TruthTag 
      efftool_nominal->setMapIndex("B", m_MCIndex);
      efftool_nominal->setMapIndex("C", m_MCIndex);
      efftool_nominal->setMapIndex("Light", m_MCIndex);
      efftool_nominal->setMapIndex("T", m_MCIndex);

      efftool_veto->setMapIndex("B", m_MCIndex);
      efftool_veto->setMapIndex("C", m_MCIndex);
      efftool_veto->setMapIndex("Light", m_MCIndex);
      efftool_veto->setMapIndex("T", m_MCIndex);


      std::vector<float> jet_effs;
      std::vector<float> jet_ineffs;


      float direct_tag_weight = 1.0;
      float veto_ineff_sf = 1.0;
      GetVhccEffs(jet_effs, jet_ineffs, efftool_nominal, efftool_veto, xaodjets,veto_ineff_sf,direct_tag_weight);

      // apply min dR jet specific corrections
      // correction_dR_jet1/2 = 1 by default 
      jet_ineffs[0] = (1 - correction_dR_jet1 * (1. - jet_ineffs[0])); // Ineff weight becomes new_ineff = (1 - c * eff )
      jet_ineffs[1] = (1 - correction_dR_jet2 * (1. - jet_ineffs[1])); 

      jet_effs[0] = jet_effs[0] * correction_dR_jet1;
      jet_effs[1] = jet_effs[1] * correction_dR_jet2;

      // compute event weights for direct and truth tagging
      // treat truth tagging and direct tagging in the same way: b-tag veto ineff scale factor on 3+jets applied directly 
      eventweights["Nominal"][-1] = direct_tag_weight;  // Direct tagging weight, note: veto ineff already applied in direct tag sf
      eventweights["Nominal"][0] = jet_ineffs[0] * jet_ineffs[1] * veto_ineff_sf;  // 0 c-tag Truth Tagging weight
      eventweights["Nominal"][1] = (jet_effs[0] * jet_ineffs[1] + jet_effs[1] * jet_ineffs[0]) * veto_ineff_sf; // 1 c-tag Truth Tagging weight
      eventweights["Nominal"][2] = jet_effs[0] * jet_effs[1] * veto_ineff_sf; // 2 c-tag Truth Tagging weight



        // optionally do systematics
        if (doSyst) {
         
          for (auto& var : m_affectingSystematics) {
            auto name = Analysis::replace_all(var.name(), " ", "_");
            SystematicSet set;

            set.insert(var);
            auto sresult = efftool_nominal->applySystematicVariation(set);
            if (sresult != SystematicCode::Ok) continue;
            sresult = efftool_veto->applySystematicVariation(set);
            if (sresult != SystematicCode::Ok) continue;

              jet_effs.clear();
              jet_ineffs.clear();
              
              direct_tag_weight = 1.0;
              veto_ineff_sf = 1.0;

              GetVhccEffs(jet_effs, jet_ineffs, efftool_nominal, efftool_veto, xaodjets,veto_ineff_sf,direct_tag_weight);

              // apply min dR jet specific corrections
              // correction_dR_jet1/2 = 1 by default 
              jet_ineffs[0] = (1 - correction_dR_jet1 * (1. - jet_ineffs[0])); // Ineff weight becomes new_ineff = (1 - c * eff )
              jet_ineffs[1] = (1 - correction_dR_jet2 * (1. - jet_ineffs[1])); 

              jet_effs[0] = jet_effs[0] * correction_dR_jet1;
              jet_effs[1] = jet_effs[1] * correction_dR_jet2;


              // compute event weights for direct and truth tagging
              eventweights[name][-1] = direct_tag_weight; // Direct tagging weight, note: veto ineff already applied in direct tag weight
              eventweights[name][0] = jet_ineffs[0] * jet_ineffs[1] * veto_ineff_sf; //  0 c-tag Truth Tagging weight
              eventweights[name][1] = (jet_effs[0] * jet_ineffs[1] + jet_effs[1] * jet_ineffs[0]) * veto_ineff_sf; // 1 c-tag Truth Tagging weight
              eventweights[name][2] = jet_effs[0] * jet_effs[1] * veto_ineff_sf; // 2 c-tag Truth Tagging weight
          
        }

        // revert to nominal
        SystematicSet default_set;
        auto sresult = efftool_nominal->applySystematicVariation(default_set);
        if (sresult != SystematicCode::Ok) Error("failed to restore nominal setting in efficiency tool: ","compute_TruthTag_EventWeight_VHcc");
        sresult = efftool_veto->applySystematicVariation(default_set);
        if (sresult != SystematicCode::Ok) Error("failed to restore nominal setting in efficiency tool: ","compute_TruthTag_EventWeight_VHcc");
      }
      


      return eventweights;
}


void BTaggingTool::FillVHccIndexMap(){
m_sample_to_map_index = {
{ "a" , {
{ 410471 , 4} , { 345114 , 4} , { 346031 , 4} , { 410464 , 4} , { 345058 , 4} , { 410647 , 5} , { 410646 , 5} , { 308097 , 3} , { 410558 , 4} , { 410558 , 4} ,
{ 345045 , 6} , { 345111 , 4} , { 363355 , 6} , { 308093 , 3} , { 345112 , 4} , { 345951 , 4} , { 345113 , 4} , { 410465 , 4} , { 363357 , 6} , { 308098 , 3} ,
{ 410472 , 4} , { 410470 , 4} , { 345055 , 4} , { 343366 , 4} , { 343367 , 4} , { 343365 , 4} , { 345053 , 4} , { 345054 , 4} , { 410658 , 5} , { 410659 , 5} ,
{ 363359 , 6} , { 363360 , 6} , { 363358 , 6} , { 345057 , 4} , { 364107 , 3} , { 364101 , 3} , { 364110 , 3} , { 364104 , 3} , { 364128 , 3} , { 364131 , 3} ,
{ 364134 , 3} , { 364137 , 3} , { 345056 , 4} , { 410557 , 4} , { 345110 , 4} , { 345109 , 4} , { 345043 , 6} , { 308096 , 3} , { 364303 , 6} , { 407347 , 4} ,
{ 345935 , 4} , { 407346 , 4} , { 407345 , 4} , { 410480 , 4} , { 364130 , 3} , { 364133 , 3} , { 364136 , 3} , { 364139 , 3} , { 364112 , 3} , { 364113 , 3} ,
{ 364169 , 3} , { 364168 , 3} , { 364109 , 3} , { 364106 , 3} , { 364103 , 3} , { 364100 , 3} , { 308094 , 3} , { 364123 , 3} , { 364114 , 3} , { 364117 , 3} ,
{ 364120 , 3} , { 363489 , 6} , { 363624 , 3} , { 363633 , 3} , { 363645 , 3} , { 363636 , 3} , { 363639 , 3} , { 363627 , 3} , { 363642 , 3} , { 363630 , 3} ,
{ 364161 , 3} , { 364164 , 3} , { 364158 , 3} , { 364167 , 3} , { 364196 , 3} , { 364197 , 3} , { 364302 , 6} , { 410250 , 4} , { 410251 , 4} , { 410252 , 4} ,
{ 364119 , 3} , { 364122 , 3} , { 364116 , 3} , { 364125 , 3} , { 364173 , 3} , { 364179 , 3} , { 364176 , 3} , { 364170 , 3} , { 363625 , 3} , { 363646 , 3} ,
{ 363637 , 3} , { 363631 , 3} , { 363640 , 3} , { 363643 , 3} , { 363634 , 3} , { 363628 , 3} , { 364126 , 3} , { 364127 , 3} , { 364182 , 3} , { 364183 , 3} ,
{ 410645 , 5} , { 410644 , 5} , { 364190 , 3} , { 364184 , 3} , { 364187 , 3} , { 364193 , 3} , { 410482 , 4} , { 345044 , 6} , { 364304 , 6} , { 364305 , 6} ,
{ 363127 , 3} , { 363136 , 3} , { 363142 , 3} , { 363130 , 3} , { 363142 , 3} , { 363145 , 3} , { 363139 , 3} , { 363133 , 3} , { 363124 , 3} , { 363145 , 3} ,
{ 364180 , 3} , { 364171 , 3} , { 364177 , 3} , { 364177 , 3} , { 364174 , 3} , { 363356 , 6} , { 363662 , 3} , { 363668 , 3} , { 363656 , 3} , { 363659 , 3} ,
{ 363650 , 3} , { 363671 , 3} , { 363665 , 3} , { 363653 , 3} , { 363623 , 3} , { 363620 , 3} , { 363614 , 3} , { 363608 , 3} , { 363617 , 3} , { 363605 , 3} ,
{ 363611 , 3} , { 363602 , 3} , { 361514 , 3} , { 361513 , 3} , { 361511 , 3} , { 361512 , 3} , { 361510 , 3} , { 308092 , 3} , { 364195 , 3} , { 364192 , 3} ,
{ 364189 , 3} , { 364186 , 3} , { 363603 , 3} , { 363621 , 3} , { 363609 , 3} , { 363606 , 3} , { 363615 , 3} , { 363612 , 3} , { 363618 , 3} , { 363600 , 3} ,
{ 364194 , 3} , { 364191 , 3} , { 364188 , 3} , { 364185 , 3} , { 175962 , 6} , { 364254 , 6} , { 364159 , 3} , { 364156 , 3} , { 364162 , 3} , { 364165 , 3} ,
{ 364102 , 3} , { 364111 , 3} , { 364108 , 3} , { 364105 , 3} , { 363162 , 3} , { 363153 , 3} , { 363159 , 3} , { 363165 , 3} , { 363156 , 3} , { 363147 , 3} ,
{ 363150 , 3} , { 363168 , 3} , { 364132 , 3} , { 364138 , 3} , { 364135 , 3} , { 364129 , 3} , { 410649 , 5} , { 410648 , 5} , { 364141 , 3} , { 364140 , 3} ,
{ 363166 , 3} , { 363163 , 3} , { 363160 , 3} , { 363151 , 3} , { 363154 , 3} , { 363148 , 3} , { 363169 , 3} , { 363157 , 3} , { 364157 , 3} , { 364163 , 3} ,
{ 364160 , 3} , { 364166 , 3} , { 364163 , 3} , { 363654 , 3} , { 363666 , 3} , { 363657 , 3} , { 363663 , 3} , { 363651 , 3} , { 363669 , 3} , { 363660 , 3} ,
{ 363648 , 3} , { 363143 , 3} , { 363140 , 3} , { 363146 , 3} , { 363137 , 3} , { 363131 , 3} , { 363140 , 3} , { 363125 , 3} , { 363146 , 3} , { 363143 , 3} ,
{ 363134 , 3} , { 363128 , 3} , { 364115 , 3} , { 364118 , 3} , { 364121 , 3} , { 364124 , 3} , { 363616 , 3} , { 363619 , 3} , { 363607 , 3} , { 363604 , 3} ,
{ 363601 , 3} , { 363613 , 3} , { 363610 , 3} , { 363622 , 3} , { 364181 , 3} , { 364175 , 3} , { 364178 , 3} , { 364175 , 3} , { 364172 , 3} , { 342287 , 4} ,
{ 342286 , 4} , { 363144 , 3} , { 363135 , 3} , { 363141 , 3} , { 363138 , 3} , { 363123 , 3} , { 363132 , 3} , { 363144 , 3} , { 363126 , 3} , { 363141 , 3} ,
{ 363129 , 3} , { 363158 , 3} , { 363155 , 3} , { 363170 , 3} , { 363161 , 3} , { 363164 , 3} , { 363152 , 3} , { 363167 , 3} , { 363149 , 3} , { 363638 , 3} ,
{ 363641 , 3} , { 363644 , 3} , { 363626 , 3} , { 363629 , 3} , { 363632 , 3} , { 363635 , 3} , { 363647 , 3} , { 363655 , 3} , { 363661 , 3} , { 363652 , 3} ,
{ 363667 , 3} , { 363664 , 3} , { 363649 , 3} , { 363670 , 3} ,
{ 363658 , 3} 
} } ,
{ "d" , {
{ 410464 , 8} , { 410465 , 8} , { 345113 , 8} , { 345951 , 8} , { 308092 , 7} , { 410482 , 8} , { 308094 , 7} , { 410480 , 8} , { 345110 , 8} , { 345109 , 8} ,
{ 410558 , 8} , { 363357 , 10} , { 345058 , 8} , { 345043 , 10} , { 363355 , 10} , { 364303 , 10} , { 308096 , 7} , { 308093 , 7} , { 364105 , 7} , { 364108 , 7} ,
{ 364102 , 7} , { 364108 , 7} , { 364111 , 7} , { 364105 , 7} , { 345057 , 8} , { 410649 , 9} , { 410648 , 9} , { 345044 , 10} , { 345056 , 8} , { 363626 , 7} ,
{ 363632 , 7} , { 363638 , 7} , { 363635 , 7} , { 363629 , 7} , { 363641 , 7} , { 364195 , 7} , { 364186 , 7} , { 364192 , 7} , { 364195 , 7} , { 364189 , 7} ,
{ 364189 , 7} , { 363358 , 10} , { 363358 , 10} , { 364117 , 7} , { 364114 , 7} , { 364120 , 7} , { 364123 , 7} , { 363356 , 10} , { 342286 , 8} , { 342287 , 8} ,
{ 410470 , 8} , { 345053 , 8} , { 345054 , 8} , { 308098 , 7} , { 364125 , 7} , { 364119 , 7} , { 364122 , 7} , { 364116 , 7} , { 407345 , 8} , { 407347 , 8} ,
{ 407346 , 8} , { 410472 , 8} , { 364254 , 10} , { 410645 , 9} , { 410644 , 9} , { 345114 , 8} , { 364127 , 7} , { 364126 , 7} , { 363648 , 7} , { 363651 , 7} ,
{ 363657 , 7} , { 363654 , 7} , { 364112 , 7} , { 364112 , 7} , { 364113 , 7} , { 346031 , 8} , { 364161 , 7} , { 364164 , 7} , { 364167 , 7} , { 364158 , 7} ,
{ 364161 , 7} , { 364158 , 7} , { 364164 , 7} , { 364167 , 7} , { 410471 , 8} , { 364302 , 10} , { 363649 , 7} , { 363658 , 7} , { 363655 , 7} , { 363652 , 7} ,
{ 410646 , 9} , { 410647 , 9} , { 345112 , 8} , { 345055 , 8} , { 364305 , 10} , { 364304 , 10} , { 410252 , 8} , { 363489 , 10} , { 363489 , 10} , { 364121 , 7} ,
{ 364124 , 7} , { 364118 , 7} , { 364115 , 7} , { 364136 , 7} , { 364133 , 7} , { 364139 , 7} , { 364130 , 7} , { 308097 , 7} , { 364128 , 7} , { 364134 , 7} ,
{ 364131 , 7} , { 364137 , 7} , { 364184 , 7} , { 364193 , 7} , { 364190 , 7} , { 364190 , 7} , { 364187 , 7} , { 364184 , 7} , { 363659 , 7} , { 363650 , 7} ,
{ 363653 , 7} , { 363656 , 7} , { 363359 , 10} , { 363360 , 10} , { 363359 , 10} , { 364185 , 7} , { 364194 , 7} , { 364188 , 7} , { 364191 , 7} , { 364191 , 7} ,
{ 364194 , 7} , { 363615 , 7} , { 363609 , 7} , { 363600 , 7} , { 363612 , 7} , { 363606 , 7} , { 363603 , 7} , { 363148 , 7} , { 363157 , 7} , { 363151 , 7} ,
{ 363163 , 7} , { 363169 , 7} , { 363148 , 7} , { 363151 , 7} , { 363154 , 7} , { 363154 , 7} , { 363166 , 7} , { 363163 , 7} , { 363157 , 7} , { 363166 , 7} ,
{ 363160 , 7} , { 363160 , 7} , { 363613 , 7} , { 363601 , 7} , { 363604 , 7} , { 363607 , 7} , { 363616 , 7} , { 363610 , 7} , { 364141 , 7} , { 364140 , 7} ,
{ 364138 , 7} , { 364135 , 7} , { 364129 , 7} , { 364132 , 7} , { 363136 , 7} , { 363142 , 7} , { 363130 , 7} , { 363139 , 7} , { 363145 , 7} , { 363133 , 7} ,
{ 363142 , 7} , { 363127 , 7} , { 363136 , 7} , { 363124 , 7} , { 363145 , 7} , { 363124 , 7} , { 363139 , 7} , { 363130 , 7} , { 363133 , 7} , { 363127 , 7} ,
{ 343366 , 8} , { 343367 , 8} , { 410557 , 8} , { 364106 , 7} , { 364109 , 7} , { 364103 , 7} , { 364109 , 7} , { 364100 , 7} , { 364101 , 7} , { 364107 , 7} ,
{ 364101 , 7} , { 364110 , 7} , { 364104 , 7} , { 361510 , 7} , { 361512 , 7} , { 361513 , 7} , { 361511 , 7} , { 361514 , 7} , { 363168 , 7} , { 363162 , 7} ,
{ 363150 , 7} , { 363159 , 7} , { 363153 , 7} , { 363156 , 7} , { 363147 , 7} , { 363162 , 7} , { 363159 , 7} , { 363153 , 7} , { 363147 , 7} , { 363156 , 7} ,
{ 363165 , 7} , { 363150 , 7} , { 363168 , 7} , { 363165 , 7} , { 363146 , 7} , { 363134 , 7} , { 363146 , 7} , { 363128 , 7} , { 363131 , 7} , { 363143 , 7} ,
{ 363128 , 7} , { 363134 , 7} , { 363137 , 7} , { 363125 , 7} , { 363140 , 7} , { 363125 , 7} , { 363131 , 7} , { 363140 , 7} , { 363137 , 7} , { 363143 , 7} ,
{ 363640 , 7} , { 363631 , 7} , { 363625 , 7} , { 363634 , 7} , { 363637 , 7} , { 363628 , 7} , { 364159 , 7} , { 364159 , 7} , { 364165 , 7} , { 364156 , 7} ,
{ 364159 , 7} , { 364162 , 7} , { 364156 , 7} , { 364165 , 7} , { 364162 , 7} , { 364176 , 7} , { 364173 , 7} , { 364179 , 7} , { 364170 , 7} , { 364179 , 7} ,
{ 364170 , 7} , { 364173 , 7} , { 364176 , 7} , { 410659 , 9} , { 410658 , 9} , { 364169 , 7} , { 364169 , 7} , { 364168 , 7} , { 364168 , 7} , { 363602 , 7} ,
{ 363608 , 7} , { 363614 , 7} , { 363605 , 7} , { 363617 , 7} , { 363611 , 7} , { 364177 , 7} , { 364171 , 7} , { 364174 , 7} , { 364171 , 7} , { 364177 , 7} ,
{ 364174 , 7} , { 364180 , 7} , { 364180 , 7} , { 364177 , 7} , { 363144 , 7} , { 363138 , 7} , { 363126 , 7} , { 363135 , 7} , { 363123 , 7} , { 363123 , 7} ,
{ 363129 , 7} , { 363141 , 7} , { 363126 , 7} , { 363132 , 7} , { 363138 , 7} , { 363141 , 7} , { 363135 , 7} , { 363129 , 7} , { 363132 , 7} , { 363144 , 7} ,
{ 364183 , 7} , { 364182 , 7} , { 364182 , 7} , { 364197 , 7} , { 364196 , 7} , { 364196 , 7} , { 364163 , 7} , { 364157 , 7} , { 364163 , 7} , { 364163 , 7} ,
{ 364166 , 7} , { 364160 , 7} , { 364157 , 7} , { 364166 , 7} , { 364160 , 7} , { 364172 , 7} , { 364175 , 7} , { 364178 , 7} , { 364178 , 7} , { 364181 , 7} ,
{ 364175 , 7} , { 364172 , 7} , { 364181 , 7} , { 364172 , 7} , { 363152 , 7} , { 363149 , 7} , { 363161 , 7} , { 363158 , 7} , { 363158 , 7} , { 363167 , 7} ,
{ 363155 , 7} , { 363170 , 7} , { 363155 , 7} , { 363149 , 7} , { 363164 , 7} , { 363152 , 7} , { 363167 , 7} , { 363170 , 7} , { 363161 , 7} , { 345111 , 8} ,
{ 345045 , 10} , { 410250 , 8} , { 410251 , 8} , { 363624 , 7} , { 363627 , 7} , { 363630 , 7} , { 363636 , 7} , { 363639 , 7} ,
{ 363633 , 7} 
} } ,
{ "e" , {
{ 363356 , 14} , { 308095 , 11} , { 308093 , 11} , { 304014 , 12} , { 345044 , 14} , { 308092 , 11} , { 410464 , 12} , { 345038 , 12} , { 345057 , 12} , { 364140 , 11} ,
{ 364141 , 11} , { 364254 , 14} , { 410472 , 12} , { 345043 , 14} , { 363489 , 14} , { 364222 , 11} , { 364223 , 11} , { 410558 , 12} , { 308097 , 11} , { 410560 , 13} ,
{ 364302 , 14} , { 410081 , 12} , { 364303 , 14} , { 345058 , 12} , { 364197 , 11} , { 364196 , 11} , { 410080 , 12} , { 345110 , 12} , { 345109 , 12} , { 345113 , 12} ,
{ 308094 , 11} , { 410482 , 12} , { 410470 , 12} , { 363633 , 11} , { 363639 , 11} , { 363630 , 11} , { 363624 , 11} , { 363636 , 11} , { 363627 , 11} , { 410471 , 12} ,
{ 410467 , 12} , { 410467 , 12} , { 410648 , 13} , { 410649 , 13} , { 364179 , 11} , { 364170 , 11} , { 364176 , 11} , { 364173 , 11} , { 364138 , 11} , { 364129 , 11} ,
{ 364132 , 11} , { 364135 , 11} , { 364126 , 11} , { 364127 , 11} , { 345045 , 14} , { 363652 , 11} , { 363649 , 11} , { 363655 , 11} , { 363658 , 11} , { 346031 , 12} ,
{ 363614 , 11} , { 363611 , 11} , { 363608 , 11} , { 363605 , 11} , { 363602 , 11} , { 363617 , 11} , { 363360 , 14} , { 363359 , 14} , { 410654 , 13} , { 410655 , 13} ,
{ 363357 , 14} , { 407345 , 12} , { 407347 , 12} , { 407346 , 12} , { 363659 , 11} , { 363656 , 11} , { 363650 , 11} , { 363653 , 11} , { 410647 , 13} , { 410646 , 13} ,
{ 410659 , 13} , { 410658 , 13} , { 363124 , 11} , { 363136 , 11} , { 363127 , 11} , { 363133 , 11} , { 363139 , 11} , { 363130 , 11} , { 363142 , 11} , { 363145 , 11} ,
{ 345056 , 12} , { 363358 , 14} , { 364304 , 14} , { 364305 , 14} , { 363606 , 11} , { 363600 , 11} , { 363603 , 11} , { 363612 , 11} , { 363615 , 11} , { 363609 , 11} ,
{ 308096 , 11} , { 410468 , 12} , { 410468 , 12} , { 345040 , 12} , { 345039 , 12} , { 363355 , 14} , { 363610 , 11} , { 363616 , 11} , { 363607 , 11} , { 363613 , 11} ,
{ 363601 , 11} , { 363604 , 11} , { 364183 , 11} , { 364182 , 11} , { 345112 , 12} , { 345055 , 12} , { 364119 , 11} , { 364122 , 11} , { 364116 , 11} , { 364125 , 11} ,
{ 364178 , 11} , { 364175 , 11} , { 364181 , 11} , { 364172 , 11} , { 364137 , 11} , { 364134 , 11} , { 364128 , 11} , { 364131 , 11} , { 410656 , 13} , { 410657 , 13} ,
{ 410480 , 12} , { 345114 , 12} , { 363143 , 11} , { 363137 , 11} , { 363131 , 11} , { 363146 , 11} , { 363125 , 11} , { 363140 , 11} , { 363134 , 11} , { 363128 , 11} ,
{ 366029 , 11} , { 366031 , 11} , { 366033 , 11} , { 366032 , 11} , { 366030 , 11} , { 366028 , 11} , { 366035 , 11} , { 366034 , 11} , { 364156 , 11} , { 364159 , 11} ,
{ 364165 , 11} , { 364162 , 11} , { 364136 , 11} , { 364139 , 11} , { 364133 , 11} , { 364130 , 11} , { 364164 , 11} , { 364161 , 11} , { 364158 , 11} , { 364167 , 11} ,
{ 364168 , 11} , { 364169 , 11} , { 364192 , 11} , { 364189 , 11} , { 364195 , 11} , { 364186 , 11} , { 345951 , 12} , { 345111 , 12} , { 410645 , 13} , { 410644 , 13} ,
{ 366012 , 11} , { 366010 , 11} , { 366013 , 11} , { 366015 , 11} , { 366014 , 11} , { 366011 , 11} , { 366016 , 11} , { 366017 , 11} , { 363155 , 11} , { 363164 , 11} ,
{ 363152 , 11} , { 363161 , 11} , { 363149 , 11} , { 363167 , 11} , { 363170 , 11} , { 363158 , 11} , { 364194 , 11} , { 364185 , 11} , { 364191 , 11} , { 364188 , 11} ,
{ 364101 , 11} , { 364104 , 11} , { 364107 , 11} , { 364110 , 11} , { 363657 , 11} , { 363651 , 11} , { 363654 , 11} , { 363648 , 11} , { 363129 , 11} , { 363141 , 11} ,
{ 363123 , 11} , { 363144 , 11} , { 363135 , 11} , { 363138 , 11} , { 363126 , 11} , { 363132 , 11} , { 364123 , 11} , { 364117 , 11} , { 364120 , 11} , { 364114 , 11} ,
{ 364113 , 11} , { 364112 , 11} , { 410557 , 12} , { 363632 , 11} , { 363629 , 11} , { 363641 , 11} , { 363626 , 11} , { 363638 , 11} , { 363635 , 11} , { 366024 , 11} ,
{ 366019 , 11} , { 366025 , 11} , { 366022 , 11} , { 366026 , 11} , { 366021 , 11} , { 366023 , 11} , { 366020 , 11} , { 363634 , 11} , { 363637 , 11} , { 363631 , 11} ,
{ 363628 , 11} , { 363625 , 11} , { 363640 , 11} , { 363150 , 11} , { 363156 , 11} , { 363153 , 11} , { 363165 , 11} , { 363147 , 11} , { 363168 , 11} , { 363162 , 11} ,
{ 363159 , 11} , { 364115 , 11} , { 364124 , 11} , { 364121 , 11} , { 364118 , 11} , { 361518 , 11} , { 361517 , 11} , { 361516 , 11} , { 361515 , 11} , { 345053 , 12} ,
{ 345054 , 12} , { 364171 , 11} , { 364177 , 11} , { 364174 , 11} , { 364180 , 11} , { 363154 , 11} , { 363163 , 11} , { 363160 , 11} , { 363169 , 11} , { 363166 , 11} ,
{ 363148 , 11} , { 363151 , 11} , { 363157 , 11} , { 308098 , 11} , { 364193 , 11} , { 364187 , 11} , { 364184 , 11} , { 364190 , 11} , { 364160 , 11} , { 364166 , 11} ,
{ 364163 , 11} , { 364157 , 11} , { 364109 , 11} , { 364103 , 11} , { 364106 , 11} , { 364100 , 11} , { 410469 , 12} , { 410465 , 12} , { 410469 , 12} , { 410218 , 12} ,
{ 410220 , 12} , { 410219 , 12} , { 410156 , 12} , { 410157 , 12} , { 410155 , 12} , { 364105 , 11} , { 364111 , 11} , { 364102 , 11} , { 364108 , 11} , { 361511 , 11} ,
{ 361510 , 11} , { 361513 , 11} , { 361514 , 11} ,
{ 361512 , 11} 
} }
};
}
