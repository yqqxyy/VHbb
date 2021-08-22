#ifndef Histo_h
#define Histo_h

#include <functional>
#include <memory>
#include <vector>

class TH1;
class TH2;

class Histo {
 private:
  std::unique_ptr<TH1> m_h;
  std::unique_ptr<TH2> m_h2;
  double& m_w;
  double* const m_x;
  double* const m_y;
  std::function<double()> m_getx;
  std::function<double()> m_gety;

 public:
  // empty
  Histo() noexcept = default;
  // mpve
  Histo(Histo&&) = default;
  // copy
  Histo(const Histo& other) noexcept;

  // 1-D histos
  Histo(TH1* h, double* x, double& w) noexcept;
  Histo(TH1* h, std::function<double()> x, double& w) noexcept;

  // 2-D histos
  Histo(TH2* h, double* x, double* y, double& w) noexcept;
  Histo(TH2* h, double* x, std::function<double()> y, double& w) noexcept;
  Histo(TH2* h, std::function<double()> x, double* y, double& w) noexcept;
  Histo(TH2* h, std::function<double()> x, std::function<double()> y,
        double& w) noexcept;

  ~Histo();

  void Fill(double additional_w = 1.0);
  TH1* GetHistCopy();
};

using HistoVec = std::vector<Histo>;
void FillHistos(HistoVec& hv, double additional_w = 1.0);

#endif
