#ifndef HistFastSvc_H
#define HistFastSvc_H

#include <TFile.h>
#include <string>
#include <unordered_map>
#include <utility>

#include "Histo.h"

#include "EventLoop/IWorker.h"

// TODO super important: add a check at the end that we never have several regions with
// same name but different integer codes

template <typename RegNaming>
class HistFastSvc {
 private:
  // the storage is a bit complicated... we store by
  // <syst, <sample, <regionCode, <templateHistosAddress, HistoVec>>>>
  using HistosForRegion = std::unordered_map<const HistoVec*, HistoVec>;
  using HistosForSample = std::unordered_map<int, HistosForRegion>;
  using HistosForSyst = std::unordered_map<std::string, HistosForSample>;

  std::unordered_map<std::string, HistosForSyst> m_storage;

  using RegionNames = std::unordered_map<int, std::string>;
  std::unordered_map<std::string, std::unordered_map<std::string, RegionNames>>
      m_regnames;

  // Handling of syst/sample
  HistosForSample* m_current;
  bool m_validCache;

  std::string m_sample;
  std::string m_syst;

  inline void UpdateCurrent() {
    if (!m_validCache) {
      m_current = &(m_storage[m_syst][m_sample]);
      m_validCache = true;
    }
  }

  bool m_fillWeightSysts;

  // Handling of weight systematics
  bool m_validWeightSystsCache;
  std::vector<std::pair<HistosForSample*, double>> m_currentWeightSysts;
  std::vector<std::pair<std::string, double>> m_weightSysts;
  void UpdateCurrentWeightSysts();

  void FillSystAccordingTo(const HistoVec& tplte, HistosForSample* hists,
                           int regCode, double additional_w = 1.0);

 public:
  HistFastSvc() = default;
  ~HistFastSvc() =
      default;  // to be revisited some day by taking care of deallocating histos properly

  /// Setting stuff

  /// Nominal sample and syst
  inline void SetSyst(const std::string& sysName,
                      bool fillWeightSysts = false) {
    if (sysName != m_syst) {
      m_syst = sysName;
      m_validCache = false;
    }
    m_fillWeightSysts = fillWeightSysts;
  }

  inline void SetSample(const std::string& sampleName) {
    if (sampleName != m_sample) {
      m_sample = sampleName;
      m_validCache = false;
    }
  }

  /// Weight systs
  inline void AddWeightSyst(const std::string& sysName,
                            double additional_w = 1.0) {
    m_weightSysts.push_back(std::make_pair(sysName, additional_w));
    m_validWeightSystsCache = false;
  }

  inline void ClearWeightSysts() {
    m_weightSysts.clear();
    m_validWeightSystsCache = false;
  }

  /// It's a kind of magic ©Queen
  template <class... ArgTypes>
  void FillAccordingTo(const HistoVec& tplte, ArgTypes... args);

  /// Write all histos to file. How histos are renamed, or placed into subdirectories, is
  /// delegated to RegNaming
  void Write(TFile* file);

  /// Register all histos to EventLoop. How histos are renamed, or placed into subdirectories, is
  /// delegated to RegNaming
  void Write(EL::IWorker* wk);
};

#include "CxAODReader/HistFastSvc.icc"
#endif
