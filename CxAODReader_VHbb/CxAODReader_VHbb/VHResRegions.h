#ifndef VHResRegions_h
#define VHResRegions_h

#include <map>
#include <string>

class TH1;
class TFile;
namespace EL {
class IWorker;
}

/* VHResRegions: an example of an encoding of region naming conventions
 *
 * The structure is extremely free.
 * It needs to provide
 * * int regCode() with any arguments
 * * string regName() with any arguments
 * * void writeHistToFile with specific arguments
 *
 * regCode must be some kind of hashing, i.e every (valid) set of input
 * variables must produce a unique code.
 *
 * All the rest (variables with their types) is left up to the users to tune
 *
 */
struct VHResRegions {
  enum Label { NONE = 0, SR, mBBCR, mBBCRlow, mBBCRhigh, mbbPresel, mbbIncl };
  static std::map<Label, std::string> m_labelNames;

  static int regCode(int ntag, int naddTag, float pTV, float mVH,
                     Label label = NONE);

  static std::string regName(int ntag, int naddTag, float pTV, float mVH,
                             Label label = NONE);

  static void writeHistToFile(TH1 *h, const std::string &syst,
                              const std::string &sample,
                              const std::string &regName, TFile *f);
  static void registerToELWorker(TH1 *h, const std::string &syst,
                                 const std::string &sample,
                                 const std::string &regName, EL::IWorker *wk);
};

#endif
