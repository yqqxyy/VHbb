#include <iostream>
#include "CxAODTools/CommonProperties.h"

#include "CxAODMaker/TruthProcessor.h"
#include "CxAODMaker/EventInfoHandler.h"

#include "xAODBase/IParticle.h"
#include "xAODRootAccess/TActiveStore.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODRootAccess/TStore.h"

#include "xAODTruth/TruthVertexContainer.h"

TruthProcessor::TruthProcessor(const std::string& name, ConfigStore & config,
                               xAOD::TEvent * event, EventInfoHandler & eventInfoHandler)
  : m_name(name),
    m_config(config),
    m_event(event), 
    m_eventInfoHandler(eventInfoHandler),
    m_listOfContainers()
{
  // fill this with list of input containers
  m_listOfContainers.clear();
  m_listOfContainers.insert("TruthParticles");
  m_listOfContainers.insert("TruthTopQuarkWithDecayParticles");
  m_listOfContainers.insert("TruthBosonWithDecayParticles");
  m_listOfContainers.insert("TruthElectrons");
  m_listOfContainers.insert("TruthMuons");
  m_listOfContainers.insert("TruthTaus");
  m_listOfContainers.insert("TruthHFWithDecayParticles");
  m_listOfContainers.insert("HardScatterParticles");
  m_first = true;
  m_newStyle = false;

  m_config.getif<std::string>("selectionName", m_selectionName);
}


TruthProcessor::~TruthProcessor()
{
}
const xAOD::TruthParticleContainer *TruthProcessor::getInParticle(const std::string &containerName) {
  return m_in[containerName];
}

const xAOD::TruthParticleContainer *TruthProcessor::getElectrons() {
  if (!m_newStyle) return getInParticle("TruthParticles");
  return getInParticle("TruthElectrons");
}

const xAOD::TruthParticleContainer *TruthProcessor::getMuons() {
  if (!m_newStyle) return getInParticle("TruthParticles");
  return getInParticle("TruthMuons");
}

const xAOD::TruthParticleContainer *TruthProcessor::getTaus() {
  if (!m_newStyle) return getInParticle("TruthParticles");
  return getInParticle("TruthTaus");
}

const xAOD::TruthParticleContainer *TruthProcessor::getNeutrinos() {
  if (!m_newStyle) return getInParticle("TruthParticles");
  return getInParticle("TruthNeutrinos");
}

void TruthProcessor::setP4(const xAOD::TruthParticle * in, xAOD::TruthParticle * out) { 
  out->setPx(in->px());
  out->setPy(in->py());
  out->setPz(in->pz());
  out->setE(in->e());
  out->setM(in->m());
}

EL::StatusCode TruthProcessor::clearEvent() {
  
  // Nothing to do here - memory handled by EventLoop's TStore
  return EL::StatusCode::SUCCESS;

}

bool TruthProcessor::isNewStyle() {
  return m_newStyle;
}

EL::StatusCode TruthProcessor::setObjects() {
  // try to guess if we have a new file format
  if (m_first) {
    const xAOD::TruthParticleContainer *constParticles = nullptr;
    std::set<std::string> newListOfContainers;
    m_newStyle = false;
    for (std::set<std::string>::iterator it = m_listOfContainers.begin(); it != m_listOfContainers.end(); ++it) {
      std::string k = *it;
      constParticles = nullptr;
      if (m_event->retrieve(constParticles, k.c_str()).isSuccess()) {
        if (k == "HardScatterParticles") { // use this as a criteria to check if this is new or old style
          m_newStyle = true;
        }
        newListOfContainers.insert(k);
      }
    }
    m_listOfContainers = newListOfContainers;
    m_first = false;
  }

  for (std::set<std::string>::iterator it = m_listOfContainers.begin(); it != m_listOfContainers.end(); ++it) {
    std::string k = *it;

    m_in[k] = nullptr;

    // record containers in TEvent
    TString shallowCopyName = "ShallowInputNominal" + k;

    const xAOD::TruthParticleContainer *constParticles = nullptr;
    if (!m_event->retrieve(constParticles, k).isSuccess()) {
      Error("TruthProcessor::run()", ("Failed to retrieve particle container '" + k + "'").c_str());
      return EL::StatusCode::FAILURE;
    }


    std::pair<xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer *> shallowCopyCont = xAOD::shallowCopyContainer(*constParticles);
    m_in[k] = shallowCopyCont.first;



    // Record containers in active TStore.
    // We set the active TStore to point to EventLoop's TStore just before calling this method.
    // This means that the memory is managed for us and we shouldn't try and clean up ourselves.
    // clearEvent() below is disabled for this reason. 
    xAOD::TStore * store = xAOD::TActiveStore::store();
    EL_CHECK("TruthProcessor::run()", store->record( shallowCopyCont.first , (shallowCopyName).Data()));
    EL_CHECK("TruthProcessor::run()", store->record( shallowCopyCont.second, (shallowCopyName + "Aux.").Data()));
  } // for each container
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode TruthProcessor::select() {
  for (std::set<std::string>::iterator it = m_listOfContainers.begin(); it != m_listOfContainers.end(); ++it) {
    std::string k = *it;
    if (!m_in[k]) continue;
    unsigned int nParticles = m_in[k]->size();
    for (unsigned int partIndex = 0; partIndex < nParticles; partIndex++) {
      passTruthParticle(m_in[k]->at(partIndex), k);
    }
  }
  return EL::StatusCode::SUCCESS;
}

//Method:  Check for valid truth particle based on the presence of either
//         1) Props::passPreSel = true
//         2) Props::RecoMatch  = true
bool TruthProcessor::ValidParticle( xAOD::TruthParticle *inPart){
  //PassPresel
  bool passPreSel = Props::passPreSel.get(inPart);
  //Is particle Reco. Matched (VHbb analysis systematics)
  bool RecoMatch = false;
  if( Props::RecoMatch.exists(inPart)){
    RecoMatch = Props::RecoMatch.get(inPart);
  }
  return passPreSel || RecoMatch;
}


void TruthProcessor::fillOutputContainer(xAOD::TruthParticleContainer * outPartCont, const std::string &inContainerName) {
  if (!m_in[inContainerName]) return;
  unsigned int nParticles = m_in[inContainerName]->size();
  for (unsigned int partIndex = 0; partIndex < nParticles; partIndex++) {

    xAOD::TruthParticle * inParticle = m_in[inContainerName]->at(partIndex);
    //if (!Props::passPreSel.get(inParticle)) continue;
    if(!ValidParticle(inParticle)) continue;

    xAOD::TruthParticle * outParticle = new xAOD::TruthParticle();
    outPartCont->push_back(outParticle);
      
    //here it is decided which variables will be written to the output
    if (writeOutputVariables(inParticle, outParticle) != EL::StatusCode::SUCCESS) {
      Error("TruthProcessor::fillOutputContainer()","Failed to set output variables!");
    }

    outParticle->auxdata<int>("partIndex") = partIndex;
  } // for every in particle
}

EL::StatusCode TruthProcessor::fillOutputContainer() {
  for (std::set<std::string>::iterator it = m_listOfContainers.begin(); it != m_listOfContainers.end(); ++it) {
    std::string k = *it;
    // fill nominal container
    xAOD::TruthParticleContainer * outContainerNominal = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase * outContainerNominalAux = new xAOD::AuxContainerBase();
    outContainerNominal->setStore(outContainerNominalAux);

    fillOutputContainer(outContainerNominal, k);

    // add container to output objects
    m_out[k] = outContainerNominal;

    //necessary even if we do not want to write an xAOD!
    //because otherwise we cannot create shallow copies in the following
    //using a TStore for this purpose does not work currently (?)
    EL_CHECK("TruthProcessor::fillOutputContainer()", m_event->record(outContainerNominal, k + "___Nominal"));
    EL_CHECK("TruthProcessor::fillOutputContainer()", m_event->record(outContainerNominalAux, k + "___NominalAux."));

  }
  return EL::StatusCode::SUCCESS;
}

void TruthProcessor::print(const xAOD::TruthParticle * part) {

  std::cout << std::fixed;
  std::cout << "\tbar=" << std::setw(7) << part->barcode();
  std::cout << "\tpdgId=" << std::setw(4) << part->pdgId();
  std::cout << "\tstatus=" << std::setw(2) << part->status();
  std::cout << "\tpt=" << std::setprecision(1) << part->pt();
  std::cout << "\teta=" << std::setprecision(3) << part->eta();
  std::cout << "\tphi=" << std::setprecision(3) << part->phi();
  std::cout << "\tE=" << std::setprecision(1) << part->e();
  std::cout << "\tnPar=" << part->nParents();
  std::cout << " ( ";
  for (unsigned int i = 0; i < part->nParents(); ++i) {
      if (!part->parent(i)) std::cout << "NULL ";
      else std::cout << part->parent(i)->barcode() << " ";
  }
  std::cout << ")";
  std::cout << "\tnChi=" << part->nChildren();
  std::cout << " ( ";
  for (unsigned int i = 0; i < part->nChildren(); ++i) {
      if (!part->child(i)) std::cout << "NULL ";
      else std::cout << part->child(i)->barcode() << " ";
  }
  std::cout << ")";

  std::cout << std::endl;

}

void TruthProcessor::printDecayTree(const xAOD::TruthParticle * part, int depth, bool statusOnly) {
  if (!part) return;
  // skip non elementary particles
  if (part->absPdgId() > 99) {
    return;
  }
  if (depth==0) {
    std::cout << "Decay tree line format: pdgId (state) (next state) ... [children available / total]" << std::endl;
  }
  // count available (not pruned) children
  int nChildrenUnpruned = 0;
  for (unsigned int i = 0; i < part->nChildren(); ++i) {
    if (part->child(i)) nChildrenUnpruned++;
  }
  // for status changes (1 child) print just the new status
  bool nextStatusOnly = (nChildrenUnpruned == 1);
  for (int i = 0; i < depth && !statusOnly; i++) {
    std::cout << "  |";
  }
  // print pdgId and status
  if (!statusOnly) {
    std::cout << std::setw(3) << part->pdgId();
  }
  std::cout << " (" <<  part->status() << ")";
  // print number of children
  if (part->nChildren() > 1) {
    std::cout << " [" << nChildrenUnpruned << "/" << part->nChildren() << "]";
  }
  if (!nextStatusOnly) {
    std::cout << std::endl;
  }
  // recursively print children
  for (unsigned int i = 0; i < part->nChildren(); ++i) {
    if (!part->child(i)) continue;
    int nextDepth = depth + !statusOnly;
    printDecayTree(part->child(i), nextDepth, nextStatusOnly);
  }
}

void TruthProcessor::printFullTree(const xAOD::TruthParticle * part) {
  if (!part) return;
  while (true) {
    if (part->nParents() < 1) break;
    const xAOD::TruthParticle * parent = part->parent(0);
    if (!parent) break;
    // go only to parents with <= 2 children (avoid 'event' objects)
    if (parent->nChildren() > 2) break;
    // need to check if current particle is child of parent (some MC records seem incomplete)
    bool foundSelf = false;
    for (unsigned int i = 0; i < parent->nChildren(); i++) {
      if (!parent->child(i)) continue;
      if (parent->child(i)->barcode() == part->barcode()) {
        foundSelf = true;
        break;
      }
    }
    if (!foundSelf) break;
    part = parent;
  }
  printDecayTree(part);
}

bool TruthProcessor::isTopFromTtbar(const xAOD::TruthParticle *part) {
  if (!part)
    return false;
  int bc = part->barcode();
  for (auto &k : m_tBarcode) {
    if (bc == k) return true;
  }
  return false;
}

bool TruthProcessor::isWFromTtbar(const xAOD::TruthParticle *part) {
  if (!part)
    return false;
  int bc = part->barcode();
  for (auto &k : m_WBarcode) {
    if (bc == k) return true;
  }
  return false;
}

bool TruthProcessor::isWdecayFromTtbar(const xAOD::TruthParticle *part) {
  if (!part)
    return false;
  int bc = part->barcode();
  for (auto &k : m_WDecaysBarcode) {
    if (bc == k) return true;
  }
  return false;
}

bool TruthProcessor::isMuonFromHadronDecay(const xAOD::TruthParticle *part) {
  if (!part)
    return false;
  int bc = part->barcode();
  for (auto &k : m_muonsFromHadronDecays) {
    if (bc == k) return true;
  }
  return false;
}

EL::StatusCode TruthProcessor::getTTBarDecayFromTruthTop(int & codeTTBarDecay, float &final_pt, float &final_ptl, float &final_etal) {
  //std::cout << "Starting getTTBarDecayFromTruthTop" << std::endl;

  codeTTBarDecay = -1;

  int n_el = 0;
  int n_mu = 0;
  int n_tau = 0;
  int n_q = 0;

  const xAOD::TruthParticleContainer *TruthTop = m_in["TruthTopQuarkWithDecayParticles"];
  const xAOD::TruthParticleContainer *TruthBoson = m_in["TruthBosonWithDecayParticles"];
  const xAOD::TruthParticleContainer *TruthElectron = m_in["TruthElectrons"];
  const xAOD::TruthParticleContainer *TruthMuon = m_in["TruthMuons"];
  const xAOD::TruthParticleContainer *TruthTau = m_in["TruthTaus"];

  m_tBarcode.clear();
  m_WBarcode.clear();
  m_WDecaysBarcode.clear();

  // for each top and top decay particle
  for (const xAOD::TruthParticle *topPart : *TruthTop) {
    if (topPart->isTop()) {    // check if we found a top
      
      const xAOD::TruthVertex *t_decvtx = topPart->decayVtx();
      const xAOD::TruthVertex *t_prodvtx = topPart->prodVtx();

      // only mark the top before its decay to a W and the first top in the event
      bool firstT = true;
      if (t_prodvtx) {
        const unsigned int t_n_parent = t_prodvtx->nIncomingParticles();
        for (unsigned int kparent = 0; kparent < t_n_parent; ++kparent) {
          const xAOD::TruthParticle *t_parent = t_prodvtx->incomingParticle(kparent);
          if (!t_parent) continue;
          if (t_parent->isTop()) {
            firstT = false;
            break;
          }
        }
      }
      bool lastT = true;
      if (t_decvtx) {
        const unsigned int t_n_children = t_decvtx->nOutgoingParticles();
        for (unsigned int kchild = 0; kchild < t_n_children; ++kchild) {
          const xAOD::TruthParticle *t_daughter = t_decvtx->outgoingParticle(kchild);
          if (!t_daughter) continue;
          if (t_daughter->isTop()) {
            lastT = false;
            break;
          }
        }
      }

      if (lastT) {
        m_tBarcode.push_back(topPart->barcode());
        //std::cout << "Found top that decays into W with barcode " << topPart->barcode() << ", pdg = " << topPart->pdgId() << ", status = " << topPart->status() << ", pT = " << topPart->pt() << std::endl;
      }
      if (firstT) {
        m_tBarcode.push_back(topPart->barcode());
        //std::cout << "Found first top in chain with barcode " << topPart->barcode() << ", pdg = " << topPart->pdgId() << ", status = " << topPart->status() << ", pT = " << topPart->pt() << std::endl;
      }
    }

    if (!topPart->isW())    // check if we found a W
      continue;

    int Wbarcode = topPart->barcode();

    // this is a W -- only mark the first and last
    // look for the first
    bool firstW = true;
    const xAOD::TruthVertex *W_prodvtx = topPart->prodVtx();
    if (W_prodvtx) {
      const unsigned int W_n_parent = W_prodvtx->nIncomingParticles();
      for (unsigned int kparent = 0; kparent < W_n_parent; ++kparent) {
        const xAOD::TruthParticle *W_parent = W_prodvtx->incomingParticle(kparent);
        if (W_parent->isW()) {
          firstW = false;
          break;
        }
      }
    }
    if (firstW) {
      m_WBarcode.push_back(Wbarcode);
      //std::cout << "Found first W with barcode " << topPart->barcode() << ", pdg = " << topPart->pdgId() << ", status = " << topPart->status() << ", pT = " << topPart->pt() << std::endl;
    }
  }

  // now loop over W list,so we can also see the W->W decays and the last W
  for (const xAOD::TruthParticle *WPart : *TruthBoson) {
    if (!WPart->isW()) continue;

    int Wbarcode = WPart->barcode();

    float W_pt = -1;
    float W_child_pt = -1;
    float W_child_eta = -1;

    // look at the W decays
    // if the decay vertex is not available, return an error code
    const xAOD::TruthVertex *W_decvtx = WPart->decayVtx();
    if (!W_decvtx) {
      //std::cout << "Could not find a W boson decay vertex." << std::endl;
      codeTTBarDecay = -3;
      return StatusCode::SUCCESS;
    }

    bool lastW = true;
    if (W_decvtx) {
      const unsigned int W_n_children = W_decvtx->nOutgoingParticles();
      for (unsigned int kchild = 0; kchild < W_n_children; ++kchild) {
        const xAOD::TruthParticle *W_daughter = W_decvtx->outgoingParticle(kchild);
        if (W_daughter->isW()) {
          lastW = false;
          break;
        }
      }
    }
    if (lastW) {
      m_WBarcode.push_back(Wbarcode);
      //std::cout << "Found last W with barcode " << WPart->barcode() << ", pdg = " << WPart->pdgId() << ", status = " << WPart->status() << ", pT = " << WPart->pt() << std::endl;
    }

    // loop over the W children
    if (lastW) {
      const unsigned int W_n_children = W_decvtx->nOutgoingParticles();
      for (unsigned int kchild = 0; kchild < W_n_children; ++kchild) {
        const xAOD::TruthParticle *W_daughter = W_decvtx->outgoingParticle(kchild);

        W_pt = WPart->pt();
        W_child_pt = W_daughter->pt();
        W_child_eta = W_daughter->eta();

        m_WDecaysBarcode.push_back(W_daughter->barcode());
        //std::cout << "Found W decay product with barcode " << W_daughter->barcode() << ", pdg = " << W_daughter->pdgId() << ", pT = " << W_daughter->pt() << std::endl;

        bool isEl = W_daughter->isElectron();
        bool isMu = W_daughter->isMuon();
        bool isTau = W_daughter->isTau();

        // Check for charged leptons, forget about neutrinos (to avoid double counting)
        if (isEl) {
          n_el++;
          final_pt = W_pt;
          final_ptl = W_child_pt;
          final_etal = W_child_eta;

          int ElectronBarcode = W_daughter->barcode();
          for (const xAOD::TruthParticle *electron : *TruthElectron) {
            if (electron->barcode() == ElectronBarcode) {
              m_WDecaysBarcode.push_back(electron->barcode());
              break;
            }
          }

          for (const xAOD::TruthParticle *electron : *TruthElectron) {
            if (electron->p4().DeltaR(W_daughter->p4()) < 0.4) {
              m_WDecaysBarcode.push_back(electron->barcode());
            }
          }
        } // if it is an electron
        if (isMu) {
          n_mu++;
          final_pt = W_pt;
          final_ptl = W_child_pt;
          final_etal = W_child_eta;

          int MuonBarcode = W_daughter->barcode();
          for (const xAOD::TruthParticle *muon : *TruthMuon) {
            if (muon->barcode() == MuonBarcode) { // found the same tau
              m_WDecaysBarcode.push_back(muon->barcode());
              break;
            }
          }

          for (const xAOD::TruthParticle *muon : *TruthMuon) {
            if (muon->p4().DeltaR(W_daughter->p4()) < 0.4) {
              m_WDecaysBarcode.push_back(muon->barcode());
              break;
            }
          }
          
        } // if it is a muon
        if (isTau) {
          n_tau++;
          final_pt = W_pt;
          int TauBarcode = W_daughter->barcode();

          // match this tau with the same particle in the TruthTaus
          // using the bar code for this
          const xAOD::TruthParticle *tauInTruthTau = nullptr;
          for (const xAOD::TruthParticle *tau : *TruthTau) {
            //std::cout << "Trying to match W decay product with a tau in TruthTaus: barcode " << tau->barcode() << ", pdg = " << tau->pdgId() << ", pT = " << tau->pt() << std::endl;
            if (tau->barcode() == TauBarcode) { // found the same tau
              tauInTruthTau = tau;
              m_WDecaysBarcode.push_back(tauInTruthTau->barcode());
              break;
            }
          }

          for (const xAOD::TruthParticle *tau : *TruthTau) {
            if (tau->p4().DeltaR(W_daughter->p4()) < 0.4) {
              m_WDecaysBarcode.push_back(tau->barcode());
              if (!tauInTruthTau) tauInTruthTau = tau;
            }
          }
          if (!tauInTruthTau) { // ok, now really complain, but don't stop
            std::cout << "Could not find a tau in the TruthTaus list." << std::endl;
            //codeTTBarDecay = -5;
            //return StatusCode::SUCCESS;
          } else {
            // get visible four-momenta information from the TruthTau
            // this is double for some reason
            // using auxdataConst here to avoid making a Props, as it would
            // overlap with a derivation made by HH->bbtautau
            final_ptl = tauInTruthTau->auxdataConst<double>("pt_vis");
            final_etal = tauInTruthTau->auxdataConst<double>("eta_vis");
          }

        } // if this is a tau
        // Check for up and charm, forget about down and strange (to avoid double counting)
        if ((W_daughter->absPdgId() == 2) || (W_daughter->absPdgId() == 4)) n_q++;
      } // for each W child
    } // if this is the last W
  } // for each top or decay product

  if (n_el == 0 && n_mu == 0 && n_tau==0 && n_q == 2) codeTTBarDecay = 0;  //jet jet
  if (n_el == 1 && n_mu == 0 && n_tau==0 && n_q == 1) codeTTBarDecay = 1;  //e jet
  if (n_el == 0 && n_mu == 1 && n_tau==0 && n_q == 1) codeTTBarDecay = 2;  //mu jet
  if (n_el == 0 && n_mu == 0 && n_tau==1 && n_q == 1) codeTTBarDecay = 3;  //tau jet
  if (n_el == 2 && n_mu == 0 && n_tau==0 && n_q == 0) codeTTBarDecay = 4;  //e e
  if (n_el == 0 && n_mu == 2 && n_tau==0 && n_q == 0) codeTTBarDecay = 5;  //mu mu
  if (n_el == 0 && n_mu == 0 && n_tau==2 && n_q == 0) codeTTBarDecay = 6;  //tau tau
  if (n_el == 1 && n_mu == 1 && n_tau==0 && n_q == 0) codeTTBarDecay = 7;  //e mu
  if (n_el == 1 && n_mu == 0 && n_tau==1 && n_q == 0) codeTTBarDecay = 8;  //e tau
  if (n_el == 0 && n_mu == 1 && n_tau==1 && n_q == 0) codeTTBarDecay = 9;  //mu tau

  //std::cout << "codeTTBarDecay " << codeTTBarDecay << std::endl;
  //
  //std::cout << "Ending getTTBarDecayFromTruthTop" << std::endl;

  return StatusCode::SUCCESS;
}

// The function getCodeTTBarDecay is independent, since it makes no assumption on the status codes.
EL::StatusCode TruthProcessor::getCodeTTBarDecay(int & codeTTBarDecay, float &final_pt, float &final_ptl, float &final_etal) {
  // use specific class if the TruthParticles is not available
  if (m_newStyle)
    return getTTBarDecayFromTruthTop(codeTTBarDecay, final_pt, final_ptl, final_etal);

  // Decay codes:
  // 0 -> jet jet
  // 1 -> jet e
  // 2 -> jet mu
  // 3 -> jet tau
  // 4 -> e   e
  // 5 -> mu  mu
  // 6 -> tau tau
  // 7 -> e   mu
  // 8 -> e   tau
  // 9 -> mu  tau

  // Error codes:
  // -1 -> The code was not able to reconstruct all the decay products of the ttbar event.
  // -2 -> The W doesn't have any parents.
  // -3 -> The W doesn't have a decay vertex.

  codeTTBarDecay = -1;

  m_tBarcode.clear();
  m_WBarcode.clear();
  m_WDecaysBarcode.clear();

  int n_el = 0;
  int n_mu = 0;
  int n_tau = 0;
  int n_q = 0;
  for (const xAOD::TruthParticle *part : *m_in["TruthParticles"]) {

    if (part->isTop()) {
      bool isFirstTop = true;
      for (size_t idx = 0; idx < part->nParents(); ++idx) {
        const xAOD::TruthParticle* parent_part = part->parent(idx);
        if (parent_part && parent_part->isTop()) {
          isFirstTop = false;
          break;
        }
      }
      if (isFirstTop)
        m_tBarcode.push_back(part->barcode());
    }

    bool isW = part->isW();

    // Look for W
    if (!isW)
      continue;

    // Look for W decaying from top
    //int nParents = part->nParents();
    //if (nParents != 1) {
    //  codeTTBarDecay = -2;
    //  return StatusCode::SUCCESS;
    //}
    //
    bool isFirstW = false;
    for (size_t idx = 0; idx < part->nParents(); ++idx) {
      const xAOD::TruthParticle* parent_part = part->parent(idx);
      if (parent_part && parent_part->isTop()) {
        m_tBarcode.push_back(part->parent(idx)->barcode());
        isFirstW = true;
        break;
      }
    }

    if (!isFirstW)
      continue;

    m_WBarcode.push_back(part->barcode());

    //std::cout << "Found top with barcode " << part->parent(0)->barcode() << ", pdg = " << part->parent(0)->pdgId() << ", status = " << part->parent(0)->status() << ", pT = " << part->parent(0)->pt() << std::endl;

    //Find decaying W
    const xAOD::TruthParticle *W = findLastParticleDecayingInItself(part);

    m_WBarcode.push_back(W->barcode());
    //std::cout << "Found W with barcode " << W->barcode() << ", pdg = " << W->pdgId() << ", status = " << W->status() << ", pT = " << W->pt() << std::endl;

    const xAOD::TruthVertex *W_decvtx = W->decayVtx();

    if (W_decvtx) {
      const unsigned int W_n_children = W_decvtx->nOutgoingParticles();

      //Look for the decay products of the W
      for (unsigned int W_child_index = 0; W_child_index < W_n_children; W_child_index++) {

        const xAOD::TruthParticle *W_daughter = W_decvtx->outgoingParticle(W_child_index);

        if (!W_daughter) continue; // not all DxAODs have the W decay products, so escape it here

        float W_pt = W->pt();
        float Wchild_pt = W_daughter->pt();
        float Wchild_eta = W_daughter->eta();

        m_WDecaysBarcode.push_back(W_daughter->barcode());
        //std::cout << "Found W child with barcode " << W_daughter->barcode() << ", pdg = " << W_daughter->pdgId() << ", pT = " << W_daughter->pt() << std::endl;

        bool isEl = W_daughter->isElectron();
        bool isMu = W_daughter->isMuon();
        bool isTau = W_daughter->isTau();

        //Check for charged leptons, forget about neutrinos (to avoid double counting)
        if (isEl) {
	  n_el++;
	  final_pt = W_pt;
	  final_ptl = Wchild_pt;
	  final_etal = Wchild_eta;
	}
        if (isMu) {
	  n_mu++;
	  final_pt = W_pt;
	  final_ptl = Wchild_pt;
	  final_etal = Wchild_eta;
	}
        if (isTau) {
	  n_tau++;
	  final_pt = W_pt;
	  if (n_tau == 1) {
	    const unsigned int Tau_n_children = W_daughter->nChildren();
	    for (unsigned int Tau_child_index=0; Tau_child_index < Tau_n_children; Tau_child_index++) {
	      const xAOD::TruthParticle* Tau_daughter = W_daughter->child(Tau_child_index);
	      if (Tau_daughter->isElectron() || Tau_daughter->isMuon()){
	        final_ptl = Tau_daughter->pt();
		final_etal = Tau_daughter->eta();
	      }
	    } 
	  }
        }
        //Check for up and charm, forget about down and strange (to avoid double counting)
        if ((W_daughter->absPdgId() == 2) || (W_daughter->absPdgId() == 4)) n_q++;

      }
    } else {
      codeTTBarDecay = -3;
    }

  }

  if(n_el == 0 && n_mu == 0 && n_tau==0 && n_q == 2) codeTTBarDecay = 0;  //jet jet
  if(n_el == 1 && n_mu == 0 && n_tau==0 && n_q == 1) codeTTBarDecay = 1;  //e jet
  if(n_el == 0 && n_mu == 1 && n_tau==0 && n_q == 1) codeTTBarDecay = 2;  //mu jet
  if(n_el == 0 && n_mu == 0 && n_tau==1 && n_q == 1) codeTTBarDecay = 3;  //tau jet
  if(n_el == 2 && n_mu == 0 && n_tau==0 && n_q == 0) codeTTBarDecay = 4;  //e e
  if(n_el == 0 && n_mu == 2 && n_tau==0 && n_q == 0) codeTTBarDecay = 5;  //mu mu
  if(n_el == 0 && n_mu == 0 && n_tau==2 && n_q == 0) codeTTBarDecay = 6;  //tau tau
  if(n_el == 1 && n_mu == 1 && n_tau==0 && n_q == 0) codeTTBarDecay = 7;  //e mu
  if(n_el == 1 && n_mu == 0 && n_tau==1 && n_q == 0) codeTTBarDecay = 8;  //e tau
  if(n_el == 0 && n_mu == 1 && n_tau==1 && n_q == 0) codeTTBarDecay = 9;  //mu tau

  //std::cout << "codeTTBarDecay " << codeTTBarDecay << std::endl;

  return StatusCode::SUCCESS;
}

const xAOD::TruthParticle* TruthProcessor::findLastParticleDecayingInItself(const xAOD::TruthParticle* part) {
  int original_id = part->pdgId();

  // rewritten to avoid recursion
  
  bool childIdIsTheSame = true;
  const xAOD::TruthParticle *current_part = part;
  while (childIdIsTheSame) {
    const xAOD::TruthVertex* part_decvtx = current_part->decayVtx();

    if (!part_decvtx) {
      return current_part;
    }

    const xAOD::TruthParticle *next_part = nullptr;

    const unsigned int n_children = part_decvtx->nOutgoingParticles();
    for (unsigned int child_index = 0; child_index < n_children; child_index++) {
      const xAOD::TruthParticle* child = part_decvtx->outgoingParticle(child_index);
      if (!child) continue; // not all children written in DxAODs

      if (child->pdgId() == original_id) {
        next_part = child;
        break;
      }
    }
    if (!next_part) { // no child found with the same pdg id as the parent, so stop here
      childIdIsTheSame = false;
      // current_part is the last particle before there was a different particle in the decay
    } else {
      // if we found another particle in the decay with the same pdg id,
      // use it in the next while loop to find its children
      current_part = next_part;
    }
  }

  return current_part;
}

EL::StatusCode TruthProcessor::findMuonsFromHadronDecay() {
  // if we don't have TruthParticles, we just don't do it ...
  // TODO -- implement this for TruthParticles (do we need it?)
  if (!m_newStyle)
    return StatusCode::SUCCESS;
  m_muonsFromHadronDecays.clear();
  std::vector<int> muons;

  const xAOD::TruthParticleContainer *TruthHadron = getInParticle("TruthHFWithDecayParticles");
  const xAOD::TruthParticleContainer *TruthMuon = getMuons();

  for (const xAOD::TruthParticle *part : *TruthHadron) {
    if (part->isMuon()) {
      m_muonsFromHadronDecays.push_back(part->barcode()); // the TruthMuons have only final state muons, so save the non-final muons too
      muons.push_back(part->barcode());

      // do dR match (removed to save CPU time if the barcode match above is sufficient)
      //for (const xAOD::TruthParticle *mu : *TruthMuon) {
      //  if (mu->p4().DeltaR(part->p4()) < 0.4) {
      //    m_muonsFromHadronDecays.push_back(mu->barcode());
      //  }
      //}

    }
  }

  // find muons in TruthMuons with the same barcode and navigate through children
  for (const xAOD::TruthParticle *part : *TruthMuon) {
    bool isFromHadron = false;
    int bc = part->barcode();
    for (int &k : muons) {
      if (k == bc) {
        isFromHadron = true;
        break;
      }
    }
    if (!isFromHadron)
      continue;

    int pdg_id = part->pdgId();

    // loop over the chain to find the last muon
    const xAOD::TruthParticle *current_part = part;
    while (current_part) { // while checks if this particle is non-nullptr
      // get decay vertex
      const xAOD::TruthVertex* part_decvtx = current_part->decayVtx();

      // check if the decay vertex is non-nullptr
      if (!part_decvtx)
        break;

      // try to find a next particle with the same pdg id among this one's children
      const xAOD::TruthParticle *next_part = nullptr;

      const unsigned int n_children = part_decvtx->nOutgoingParticles();
      for (unsigned int child_index = 0; child_index < n_children; child_index++) {
        const xAOD::TruthParticle* child = part_decvtx->outgoingParticle(child_index);
        if (!child) continue; // not all children written in DxAODs
        if (child->pdgId() == pdg_id) { // found a child with the same pdg id, mark it as the next particle
          next_part = child;
          break;
        }
      }
      // stop when we did not find a child with the same pdg id
      // in this case, take the lest one we could find (current_part) as the last muon
      if (!next_part) break;

      // go to the next child with the same pdg id
      current_part = next_part;
    } // while there is a child
    if (current_part) m_muonsFromHadronDecays.push_back(current_part->barcode());


  }


  return StatusCode::SUCCESS;
}

float TruthProcessor::ComputeSherpapTV() {
  TLorentzVector final_V;
  std::string contName = "TruthParticles";
  if (m_newStyle)
    contName = "TruthBosonWithDecayParticles";

  for (const xAOD::TruthParticle *particle : *getInParticle(contName)) {
    if (particle->status() == 3) {    // Getting truth boson information (for Sherpa)
      if (!particle->isLepton()) continue; // Only interested in the leptonic decays!

      final_V += particle->p4();
    }
  }
  return final_V.Pt();
}

bool TruthProcessor::checkSherpaVZqqZbb() {
  std::string contName = "TruthParticles";
  if (m_newStyle)
    contName = "TruthBosonWithDecayParticles";

  unsigned int nZll = 0;
  unsigned int nZantill = 0;
  unsigned int nZvv = 0;
  unsigned int nZantivv = 0;
  unsigned int nZqq = 0;
  unsigned int nZbb = 0;
  unsigned int nWlv_l = 0;
  unsigned int nWlv_v = 0;
  for (const xAOD::TruthParticle *part : *getInParticle(contName)) {
    if (part->status() == 11) {
      int id = part->pdgId();
      bool isZll = (id == 11 || id == 13 || id == 15);
      bool isZantill = (id == -11 || id == -13 || id == -15);
      bool isZvv = (id == 12 || id == 14 || id == 16);
      bool isZantivv = (id == -12 || id == -14 || id == -16);
      bool isWlv_l = (abs(id) == 11 || abs(id) == 13 || abs(id) == 15);
      bool isWlv_v = (abs(id) == 12 || abs(id) == 14 || abs(id) == 16);
      // to do: check the impact when adding also negative values for the id of quarks
      bool isZqq = (id == 1 || id == 2 || id == 3 || id == 4);
      bool isZbb = (id == 5);
      if (isZll) nZll++;
      if (isZantill) nZantill++;
      if (isZvv) nZvv++;
      if (isZantivv) nZantivv++;
      if (isZqq) nZqq++;
      if (isZbb) nZbb++;
      if (isWlv_l) nWlv_l++;
      if (isWlv_v) nWlv_v++;
    }
  }

  bool isZll = ((nZll == 1) && (nZantill == 1));
  bool isZvv = ((nZvv == 1) && (nZantivv == 1));
  bool isWlv = ((nWlv_l == 1) && (nWlv_v == 1));
  bool isVZbb = ((isZll || isZvv || isWlv) && nZbb == 1);
  return isVZbb;

}

bool TruthProcessor::isW(const xAOD::TruthParticle * part) {
  return part->isW();
}

bool TruthProcessor::isZ(const xAOD::TruthParticle * part) {
  return part->isZ();
}

bool TruthProcessor::isWorZ(const xAOD::TruthParticle * part) {
  return (part->isW() || part->isZ());
}

bool TruthProcessor::isHiggs(const xAOD::TruthParticle * part) {
  return part->isHiggs();
}

bool TruthProcessor::isTop(const xAOD::TruthParticle * part) {
  return part->isTop();
}

bool TruthProcessor::isTau(const xAOD::TruthParticle * part) {
  return part->absPdgId() == 15;
}

bool TruthProcessor::checkWorZPythia(const xAOD::TruthParticle * part) {
  return (isWorZ(part) && (part->status() == 22 || part->status() == 62));
}

bool TruthProcessor::checkHiggsPythia(const xAOD::TruthParticle * part) {
  return (isHiggs(part) && (part->status() == 22 || part->status() == 62));
}

bool TruthProcessor::isH35(const xAOD::TruthParticle * part) {
  return part->absPdgId()==35;
}

bool TruthProcessor::isFromDecay(const xAOD::TruthParticle * part,
        bool(*checkParent)(const xAOD::TruthParticle*),
        bool allowStateChanges) {
  
  // check parent (assuming there can be just one)
  
  // rewritten to do this in a non-recursive way
  bool isItFromDecay = false;


  const xAOD::TruthParticle *current_part = part;
  while (true) {

    // give up if the particle is nullptr
    if (!current_part) {
      isItFromDecay = false;
      break;
    }

    // give up if the particle has more than one parent
    // this is an assumption here
    if (current_part->nParents() != 1) {
      isItFromDecay = false;
      break;
    }

    // get the parent
    const xAOD::TruthParticle * parent = current_part->parent(0);

    // if it is nullptr, give up
    if (!parent) {
      isItFromDecay = false;
      break;
    }

    // check if this parent satisfies the "checkParent" function criteria
    // if it does, stop this: the original particle has a parent that satisfy whatever
    // the desired criteria in checkParent is
    if (checkParent(parent)) {
      isItFromDecay = true;
      break;
    }

    // if allowStateChanges is false,
    // then stop here: we only checked one parent
    // if it is true, then we would keep going to the parent of the parent and so on
    // to find a parent that satisfies the checkParent criteria
    // as long as we are in the same pdg id
    if (!allowStateChanges) {
      isItFromDecay = false;
      break;
    }

    int original_id = current_part->pdgId();
    int original_status = current_part->status();

    // go upwards in the decay chain for status changes
    if (parent->pdgId()  == original_id &&
        parent->status() != original_status) {
      current_part = parent;
    } else { // not the same id, or same status, so break this
      isItFromDecay = false;
      break;
    }
  }
  return isItFromDecay;
}

bool TruthProcessor::setPassPresel(xAOD::TruthParticle * part) {
  Props::passPreSel.set(part, true);
  return true;
}

bool TruthProcessor::isEWLepton(const xAOD::TruthParticle * part) {

  if (!part)
    return false;

  // Leptons only
  int absPdgId = (int) std::fabs(part->pdgId());
  if ( ! (absPdgId == 15 || absPdgId == 11 || absPdgId == 13) )
    return false;

  //std::cout << "PART: "  << part->pdgId() << " " << part->status() << std::endl;

  // 1) powheg
  if (part->isLepton() && part->status() == 3) {
    // skip hadronic V decays, as they are very messy
    for (unsigned int i = 0; i < part->nParents(); ++i) {
      auto parent = part->parent(i);
      if (!parent) continue;
      // check if parent is W/Z
      if ( (parent->isW() || parent->isZ() || parent->isHiggs()) && parent->status() == 3) {
	//std::cout << "    PASS POWHEG" << std::endl;
	return true;
      }
    }
  }

  // 2) pythia8
  if (part->isLepton() && (part->status() == 1 || part->status() == 2)) {
    if (isFromDecay(part, &checkWorZPythia) || isFromDecay(part, &checkHiggsPythia)) {
      //std::cout << "    PASS PYTHIA" << std::endl;
      return true;
    }
  }

  // 3) sherpa
  if (part->isLepton() && part->status() == 3) {
    for (unsigned int i = 0; i < part->nParents(); ++i) {
      auto parent = part->parent(i);
      if (!parent) continue;
      // check if parent is also from hard process (to distinguish from pythia)
      if (parent->status() == 3) {
	//std::cout << "    PASS SHERPA" << std::endl;
	return true;
      }
    }
  }
  return false;
}

void TruthProcessor::decorate() {
  //std::cout << "=================" << std::endl;
  for (std::set<std::string>::iterator it = m_listOfContainers.begin(); it != m_listOfContainers.end(); ++it) {
    std::string k = *it;
    if (!m_in[k]) continue;
    unsigned int nParticles = m_in[k]->size();
    //std::cout << "In container " << k << ", there are " << nParticles << " particles." << std::endl;
    for (unsigned int partIndex = 0; partIndex < nParticles; partIndex++) {

      xAOD::TruthParticle * inParticle = m_in[k]->at(partIndex);
      decorate(inParticle, k);
    }
  }
}

void TruthProcessor::decorate(xAOD::TruthParticle * part, const std::string &) {
  part->auxdata<char>("isWdecayFromTtbar") = 0;
  part->auxdata<char>("isWFromTtbar") = 0;
  part->auxdata<char>("isTFromTtbar") = 0;
  part->auxdata<char>("isMuonFromHadron") = 0;

  // keep muons from hadron decays
  if (isMuonFromHadronDecay(part)) {
    part->auxdata<char>("isMuonFromHadron") = 1;
  }
  // keep top, W and W decay products in ttbar
  if (isTopFromTtbar(part)) {
    part->auxdata<char>("isTFromTtbar") = 1;
  }
  if (isWFromTtbar(part)) {
    part->auxdata<char>("isWFromTtbar") = 1;
  }
  if (isWdecayFromTtbar(part)) {
    part->auxdata<char>("isWdecayFromTtbar") = 1;
  }

}

/**
 * This function decides if a particle should be saved or not
 */
bool TruthProcessor::passTruthParticle(xAOD::TruthParticle * part, const std::string &containerName) {
  // ensure writing of flag for all events
  Props::passPreSel.set(part, false);

  return passCustomTruthParticle(part, containerName);
}

bool TruthProcessor::passCustomTruthParticle(xAOD::TruthParticle * part, const std::string &containerName) {

  // accept all input TruthParticles
  // left commented out previous setup below
  // it is recommended to write a new class for the TruthProcessor that inherits from it
  // and override this function to have a more detailed decision to choose particles to save in
  // a specific analysis

  if (m_newStyle) {
    // do not store everything to save space
    if ( (containerName != "TruthHFWithDecayParticles") &&
         (containerName != "TruthBosonWithDecayParticles") &&
         (containerName != "TruthTopQuarkWithDecayParticles") ) {
      setPassPresel(part);
      return true;
    }
    return false;
  }

  // keep muons from hadron decays
  if (isMuonFromHadronDecay(part)) {
    setPassPresel(part);
  }
  // keep top, W and W decay products in ttbar
  if (isTopFromTtbar(part)) {
    setPassPresel(part);
  }
  if (isWFromTtbar(part)) {
    setPassPresel(part);
  }
  if (isWdecayFromTtbar(part)) {
    setPassPresel(part);
  }

  // check for stable photons and all quarks for Vgamma analysis
  // TODO: check for W/Z/Higgs and quarks from them on signal samples

  int status = part->status();
  int absPdgId = part->absPdgId();
  bool status3 = status == 3;
  bool status22 = status == 22;
  bool isWB = part->isW();
  bool isZB = part->isZ();
  bool isHB = part->isHiggs();
  bool isQ = part->isQuark();
  bool isTopQ = part->isTop();
  bool isB = part->absPdgId() == 5;
  bool isLepton = part->isLepton();

  if (m_selectionName == "vgamma") {
    if ( status3 && (isWB || isZB || isHB || isQ || absPdgId == 22) ) {
      setPassPresel(part);
    }
    if ( isQ ) {
      setPassPresel(part);
    }
  }

  //if (part->nParents() == 0 && part->pdgId() != 21) {
  //  // print decay tree
  //  printDecayTree(part);
  //}
  //if (part->isHiggs() && part->nChildren() == 2) {
  //  // print full decay tree (including ancestors)
  //  printFullTree(part);
  //}


  // TODO: check for MC channel number to make it robust against generator changes

  // check top decay (already done above ... do we really need this now?)
  // 1) powheg, acerMC:
  //    - status==3 for hard process
  //    - require part->status() == 3 for all particles;

  if (isTopQ && status3) {
    setPassPresel(part);
  }
  for (unsigned int i = 0; i < part->nParents(); ++i) {
    auto parent = part->parent(i);
    if (!parent) continue;

    // check if parent is top
    if (parent->isTop() && status3) {
      setPassPresel(part);
    }
    if (parent->isW() && status3) {
      // check is grandparent is top and parent is not bottom (not interested in bottom decay)
      for (unsigned int j = 0; j < parent->nParents(); ++j) {
        auto grandparent = parent->parent(j);
        if (!grandparent) continue;
        if (grandparent->isTop() && status3) {
          setPassPresel(part);
        }
      } // for each grand parent
    }
  }
  // check bottom from W->tb for single top diagrams
  if (isB && status3) {
    for (unsigned int i = 0; i < part->nParents(); ++i) {
      auto parent = part->parent(i);
      if (!parent) continue;
      // check if parent is top
      for (unsigned int j = 0; j < parent->nChildren(); ++j) {
        auto sibling = parent->child(j);
        if (!sibling) continue;
        // check if sibling is top)
        if (sibling->isTop() && sibling->status() == 3) {
          setPassPresel(part);
        }
      } // for each child
    } // for each parent
  }

  // 2) Powheg+Pythia8
  if (isTopQ && status22) {
    setPassPresel(part);
  }
  if (isB && isFromDecay(part, &isTop, false)) {
    setPassPresel(part);
  }
  if (isWB && isFromDecay(part, &isTop, false)) {
    setPassPresel(part);
  }
  if ((isLepton || isQ) && isFromDecay(part, &isW, false)) {
    setPassPresel(part);
  }

  // check Higgs decay
  // 1) pythia
  //    - event record contains several Higgs with different status codes
  //    - need to select part->status() == 22 or 62
  //      from pythia manual:
  //      22 : intermediate (intended to have preserved mass)
  //      62 : outgoing subprocess particle with primordial kT included
  //    - c or b from Higgs has status() == 23
  //      from pythia manual:
  //      23: outgoing

  // Don't check Higgs status to be compatible with other generators.
  // For additional status check use checkHiggsPythia() instead if isHiggs()
  if (isHB && part->nChildren() == 2 ) {
    setPassPresel(part);
  }
  if ((absPdgId == 4 || isB) && status == 23) {
    if (isFromDecay(part, &isHiggs)) {
      setPassPresel(part);
    }
  }

  // Keep H->tautau decays too
  if (absPdgId == 15 && isFromDecay(part, &isHiggs)){
    setPassPresel(part);
  }

  // Keep lepton decaying from tau
  if ((absPdgId == 11 || absPdgId == 13) && isFromDecay(part, &isTau)){
    setPassPresel(part);
  }

  // check vector bosons
  // 1) powheg:
  //    - need to select part->status() == 3 for W/Z from hard process
  //    - status code 2 involves internal W -> W, but also W -> lnu
  // 2) pythia8
  //    - need to select part->status() == 62 for W/Z
  //    - need to select part->status() == 1,2 for l/nu
  // 3) sherpa
  //    - sherpa does not store (virtual) W, but only final state partons from hard process
  //    - status() == 3

  // 1) powheg
  if ((isWB || isZB) && status3) {
    setPassPresel(part);
  }
  if (isLepton && status3) {
    // skip hadronic V decays, as they are very messy
    for (unsigned int i = 0; i < part->nParents(); ++i) {
      auto parent = part->parent(i);
      if (!parent) continue;
      // check if parent is W/Z
      if ( (parent->isW() || parent->isZ()) && parent->status() == 3) {
        setPassPresel(part);
      }
    } // for each parent
  }

  // 2) pythia8
  if (checkWorZPythia(part) && (part->nChildren() == 2)) {
    setPassPresel(part);
  }
  if (isLepton && (status == 1 || status == 2)) {
    if (isFromDecay(part, &checkWorZPythia)) {
      setPassPresel(part);
    }
  }

  // 3) sherpa
  if (isLepton && status3) {
    for (unsigned int i = 0; i < part->nParents(); ++i) {
      auto parent = part->parent(i);
      if (!parent) continue;
      // check if parent is also from hard process (to distinguish from pythia)
      if (parent->status() == 3) {
        setPassPresel(part);
      }
    }
  }

  // Add DM particles (+-1000022) and pseudoscalar A0 in Z'-2HDM models
  if (absPdgId == 1000022) {
    setPassPresel(part);
  }

  if (absPdgId == 28) {
    setPassPresel(part);
  }

  // extra functions

  // for A
  if (absPdgId == 36 && part->nChildren() == 2 ) {
    setPassPresel(part);
  }

  // for H
  if (absPdgId == 35 && part->nChildren() == 2 ) {
    setPassPresel(part);
  }

  // for Z
  if (isZB && part->nChildren() == 2 ) {
    setPassPresel(part);
  }

  // for Hbb
  if (isB) {
    if (isFromDecay(part, &isH35)) {
      setPassPresel(part);
    }
  }

  // for Zll
  if (isLepton) {
    if (isFromDecay(part, & isZ)) {
      setPassPresel(part);
    }
  }

  // for HWW
  if (absPdgId == 24) {
    if (isFromDecay(part, &isH35)) {
      setPassPresel(part);
    }
  }

  //   print(part, 0);

  // Diboson
  // 1) Identify Sherpa VZbb events
  // W and Z will be status 3 and have 2 children with status 11 
  if (status == 11) {
    // printDecayTree(part);   
    if (part->nParents()==1) {
      if (part->parent(0)->status() == 3 && (part->parent(0)->isW() || part->parent(0)->isZ())) {
	setPassPresel(part); 
	// std::cout << "found " << part->pdgId() << " status " << part->status() << " bc " << part->barcode() << std::endl;
      }
    }
  }
  
  return false;  
}

EL::StatusCode TruthProcessor::writeOutputVariables(xAOD::TruthParticle * inPart, xAOD::TruthParticle * outPart) {
  // set kinematics
  setP4( inPart , outPart );

  outPart->setPdgId(inPart->pdgId());

  // the next two are temp for testing
  outPart->setBarcode(inPart->barcode());
  outPart->setStatus(inPart->status());

  outPart->auxdata<char>("isWdecayFromTtbar") = 0;
  outPart->auxdata<char>("isWFromTtbar") = 0;
  outPart->auxdata<char>("isTFromTtbar") = 0;
  outPart->auxdata<char>("isMuonFromHadron") = 0;

  for (size_t k = 0; k < m_WDecaysBarcode.size(); ++k) {
    int bc = m_WDecaysBarcode[k];
    if (inPart->barcode() == bc) outPart->auxdata<char>("isWdecayFromTtbar") = 1;
  }
  for (size_t k = 0; k < m_WBarcode.size(); ++k) {
    int bc = m_WBarcode[k];
    if (inPart->barcode() == bc) outPart->auxdata<char>("isWFromTtbar") = 1;
  }
  for (size_t k = 0; k < m_tBarcode.size(); ++k) {
    int bc = m_tBarcode[k];
    if (inPart->barcode() == bc) outPart->auxdata<char>("isTFromTtbar") = 1;
  }
  for (size_t k = 0; k < m_muonsFromHadronDecays.size(); ++k) {
    int bc = m_muonsFromHadronDecays[k];
    if (inPart->barcode() == bc) outPart->auxdata<char>("isMuonFromHadron") = 1;
  }

  //Check for matching quantities
  Props::RecoMatch.copyIfExists(inPart, outPart);
  
  outPart->toPersistent();

  return writeCustomOutputVariables(inPart, outPart);

}

EL::StatusCode TruthProcessor::writeCustomOutputVariables(xAOD::TruthParticle*, xAOD::TruthParticle*) {
  // This method is meant to be overridden in derived handlers for writing
  // additional decorations. Argument names are omitted here to avoid warnings.
  return EL::StatusCode::SUCCESS;
}

