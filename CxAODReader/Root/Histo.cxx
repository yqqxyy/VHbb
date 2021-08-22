#include "CxAODReader/Histo.h"
#include <TH1.h>
#include <TH2.h>

void Histo::Fill(double additional_w) {
  if (!m_h && !m_h2) return;

  if (!m_h) {  // 4 cases
    if (m_x == nullptr && m_y == nullptr) {
      m_h2->Fill(m_getx(), m_gety(), m_w * additional_w);
    } else if (m_x == nullptr) {
      m_h2->Fill(m_getx(), *m_y, m_w * additional_w);
    } else if (m_y == nullptr) {
      m_h2->Fill(*m_x, m_gety(), m_w * additional_w);
    } else {
      m_h2->Fill(*m_x, *m_y, m_w * additional_w);
    }
  } else {  // 2 cases
    if (m_x == nullptr) {
      m_h->Fill(m_getx(), m_w * additional_w);
    } else {
      m_h->Fill(*m_x, m_w * additional_w);
    }
  }
}

Histo::Histo(const Histo& other) noexcept
    : m_h(other.m_h ? (TH1*)(other.m_h->Clone()) : nullptr),
      m_h2(other.m_h2 ? (TH2*)(other.m_h2->Clone()) : nullptr),
      m_w(other.m_w),
      m_x(other.m_x),
      m_y(other.m_y),
      m_getx(other.m_getx),
      m_gety(other.m_gety) {
  /*
  std::cout << "In copy ctor" << std::endl;
  if(!other.m_h) { std::cout << "other nullptr ! something is weird" << std::endl; }
  else if(!m_h) { std::cout << "this nullptr ! something is weird" << std::endl; }
  else { std::cout << m_h->GetName() << std::endl;}
  */
}

Histo::Histo(TH1* h, double* x, double& w) noexcept
    : m_h(h), m_h2(nullptr), m_w(w), m_x(x), m_y(nullptr), m_getx(), m_gety() {
  m_h->Sumw2();
}

Histo::Histo(TH1* h, std::function<double()> x, double& w) noexcept
    : m_h(h),
      m_h2(nullptr),
      m_w(w),
      m_x(nullptr),
      m_y(nullptr),
      m_getx(x),
      m_gety() {
  m_h->Sumw2();
}

Histo::Histo(TH2* h, double* x, double* y, double& w) noexcept
    : m_h(nullptr), m_h2(h), m_w(w), m_x(x), m_y(y), m_getx(), m_gety() {
  m_h2->Sumw2();
}

Histo::Histo(TH2* h, double* x, std::function<double()> y, double& w) noexcept
    : m_h(nullptr), m_h2(h), m_w(w), m_x(x), m_y(nullptr), m_getx(), m_gety(y) {
  m_h2->Sumw2();
}

Histo::Histo(TH2* h, std::function<double()> x, double* y, double& w) noexcept
    : m_h(nullptr), m_h2(h), m_w(w), m_x(nullptr), m_y(y), m_getx(x), m_gety() {
  m_h2->Sumw2();
}

Histo::Histo(TH2* h, std::function<double()> x, std::function<double()> y,
             double& w) noexcept
    : m_h(nullptr),
      m_h2(h),
      m_w(w),
      m_x(nullptr),
      m_y(nullptr),
      m_getx(x),
      m_gety(y) {
  m_h2->Sumw2();
}

Histo::~Histo() {
  //std::cout << "In destructor" << std::endl;
}

TH1* Histo::GetHistCopy() {
  if (m_h) {
    return (TH1*)(m_h->Clone());
  }
  if (m_h2) {
    return (TH2*)(m_h2->Clone());
  }
  return nullptr;
}

void FillHistos(HistoVec& hv, double additional_w) {
  for (auto& h : hv) {
    h.Fill(additional_w);
  }
}
