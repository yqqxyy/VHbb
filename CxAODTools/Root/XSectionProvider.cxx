
#include "CxAODTools/XSectionProvider.h"

#include <TSystem.h>
#include <fstream>
#include <sstream>

#include "TError.h"

XSectionProvider::XSectionProvider(std::string fileName) {

  std::ifstream file;
  file.open(gSystem->ExpandPathName(fileName.c_str()));
  if (!file.good()) {
    Error("XSectionProvider()", "Can't open file '%s'.", fileName.c_str());
    exit(EXIT_FAILURE);
  }

  Info("XSectionProvider()", "Reading file '%s'.", fileName.c_str());

  // process file
  while (!file.eof()) {
    // read line
    std::string lineString;
    getline(file, lineString);
    //std::cout << lineString << std::endl;

    // skip empty lines
    // TODO - is there a better way to implement this check?
    if (lineString.find(".") > 1000) {
      continue;
    }

    // skip lines starting with #
    if (lineString.find("#") == 0) {
      continue;
    }

    // store in map
    std::stringstream line(lineString);
    int sid;
    std::string name;
    float xSection;
    float kFactor;
    float filterEff;
    std::string samplename;

    line >> sid >> xSection >> kFactor >> filterEff >> samplename >> name;

    if (m_xSections.count(sid) != 0) {
      Warning("XSectionProvider()", "Skipping duplicated mc_channel_number for line '%s'.", lineString.c_str());
      continue;
    }

    m_xSections[sid] = {name, xSection, kFactor, filterEff, samplename};

  }
  file.close();
}

bool XSectionProvider::hasMCchannel(int mc_channel_number) {
  return (m_xSections.count(mc_channel_number) != 0);
}

float XSectionProvider::getXSection(int mc_channel_number) {

  if (m_xSections.count(mc_channel_number) == 0) {
    Error("XSectionProvider::getXSection()", "Unknown mc_channel_number %i", mc_channel_number);
    exit(EXIT_FAILURE);
  }

  XSection& xSec = m_xSections[mc_channel_number];
  return xSec.xSection * xSec.kFactor * xSec.filterEff;
}

std::string XSectionProvider::getSampleName(int mc_channel_number) {
    //This function should only be called when running on MC
    if (m_xSections.count(mc_channel_number) == 0) {
      Error("XSectionProvider::getSampleName()", "Unknown mc_channel_number %i", mc_channel_number);
      exit(EXIT_FAILURE);
    }

    XSection& xSec = m_xSections[mc_channel_number];
    return xSec.samplename;
}

std::string XSectionProvider::getSampleDetailName(int mc_channel_number) {
  if (m_xSections.count(mc_channel_number) == 0) {
    Error("XSectionProvider::getSampleDetailName()", "Unknown mc_channel_number %i", mc_channel_number);
    exit(EXIT_FAILURE);
  }

  XSection& xSec = m_xSections[mc_channel_number];
  return xSec.name;
}
