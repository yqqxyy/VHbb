#include "CxAODTools/OvCompat.h"
#include "TFile.h"

Ov::ToolKitManager *Ov::ToolKitManager::s_instance = nullptr;

std::pair<TFile *, bool> Ov::getFile( Ov::IAnalysisMain &analysis_context, const char *name) {
  std::unique_ptr<TFile> a_file;
  if (analysis_context.worker()) {
    a_file.reset( analysis_context.worker()->getOutputFile(name) );
  }
  if (!a_file.get() || !a_file->IsWritable()) {
    throw RuntimeIssue(NTA_MAKE_MESSAGE_STR(NTA::Message::kFATAL, "getFile",100, "Failed to get a writeable output file " << name));
  }
  return std::make_pair(a_file.release(),false);
}

