//This macro is used to draw plots that compare the effect on the integral for each systematic
//limitFileCheckClass.C makes plots of the shape differences on input variables for each systematic

void SetStyle(TGraphAsymmErrors* &plot1, TGraphAsymmErrors* &plot2, double Xmin = -1, double Xmax = -1);
void changeErrorPos(int N, double *down, double *up);
void drawyieldplots(TFile *file1,TFile *file2,TString ptvregion,TString jetnumber,TString region,TString sample);

void sysyieldplots(){
//  TString TestFile= "TEST Path"; // put the paths of the files you want to compare here  
//  TString RefFile = "REF Path";
  TString TestFile = "root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/Summer2018/v31-10/TwoLep/LimitHistograms.VH.llbb.13TeV.mc16ad.LiverpoolBmham.v11.root";  // ICHEP 2L inputs
  TString RefFile = "root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/Summer2018/v31-10/TwoLep/LimitHistograms.VH.llbb.13TeV.mc16ad.LiverpoolBmham.v11.root";  // ICHEP 2L inputs
  vector<TString> ptvs = {"75_150ptv", "150ptv"};
  vector<TString> njets = {"2jet", "3pjet"}; 
  vector<TString> regions = {"SR"};//, "topemucr"}; 
  vector<TString> samples = {"qqZllH125"};

  TFile *plotfile1 = TFile::Open(TestFile);
  TFile *plotfile2 = TFile::Open(RefFile);
  for(int ip=0;ip<ptvs.size();ip++){
     for(int ij=0;ij<njets.size();ij++){
	for(int ir=0;ir<regions.size();ir++){
	  for(int is=0;is<samples.size();is++){ 
	    cout<<"Reading Regions:  "<<ptvs.at(ip)<<"  "<<njets.at(ij)<<"  "<<regions.at(ir)<<endl;
	    drawyieldplots(plotfile1, plotfile2, ptvs.at(ip), njets.at(ij), regions.at(ir), samples.at(is));
	  }  //loop all samples 
	}  //loop all regions
     }  //loop n jets
  }  //loop ptv bins
  plotfile1->Close();
  plotfile2->Close();
}

void drawyieldplots(TFile *file1, TFile *file2, TString ptvregion = "75_150ptv", TString jetnumber="2jet", TString region = "SR" , TString sample="qqZllH125"){
   bool is2LICHEP=false;

   TDirectory *Dir1 = (TDirectory *)file1->Get("Systematics");
   TDirectory *Dir2 = (TDirectory *)file2->Get("Systematics");

   vector<TString> Systematics1;
   vector<TString> Systematics;
   vector<TString> plotlabel;

   vector<string> sysread;
   TKey* key;
   TIter nextkey(Dir1 -> GetListOfKeys());
	 //Chanege Dir1 to Dir2 if you want to use sys list from ref files
   while ((key = (TKey*) nextkey())) {
     TClass* cl = gROOT -> GetClass(key -> GetClassName());
     if (!cl) continue;
       TString sysName = key -> GetName();
       if (sysName.Contains("_Sys") && sysName.Contains(sample)) {
         sysName.Remove(0,sysName.Index("_Sys")+4);
         if(sysName.Contains("__1up")) 
           sysName.Remove(sysName.Index("__1up"), sysName.Index("__1up")+4);
         if(sysName.Contains("__1down"))
           sysName.Remove(sysName.Index("__1down"), sysName.Index("__1down")+6);
         if (sysName.Contains("_AntiKt4EMTopoJets") && is2LICHEP) //for 2L MIA ICHEP 
           sysName.Remove(sysName.Index("_AntiKt4EMTopoJets"), sysName.Index("_AntiKt4EMTopoJets")+18 );
         sysread.push_back(sysName.Data());
       }
   }
   std::sort( sysread.begin(), sysread.end() );
   sysread.erase( std::unique( sysread.begin(), sysread.end() ), sysread.end() );

   for(int is=0;is<sysread.size();is++){
     TString sysname = sysread.at(is);
     //Some 0 uncertainties should be skipped, add them here
     //JET SYS are different for current and ICHEP, should show them separately 
//     if(sysname.Contains("JET_CR")) continue; 
//     if(! (sysname.Contains("JET")&&sysname.Contains("NP")) ) continue;
     Systematics.push_back(sysname+"__1up");
     Systematics.push_back(sysname+"__1down");
     if(sysname.Contains("FT_EFF") && is2LICHEP){  //for 2L MIA ICHEP
       Systematics1.push_back(sysname+"_AntiKt4EMTopoJets__1up");
       Systematics1.push_back(sysname+"_AntiKt4EMTopoJets__1down");
     }
     else{
       Systematics1.push_back(sysname+"__1up");
       Systematics1.push_back(sysname+"__1down");
     }
     plotlabel.push_back(sysname);
   }

   TH1F *plot1, *plot2;
   TString plotname1, plotname2;

   //Systematics
   int nS = plotlabel.size();
   double x1[nS], y1[nS], xl1[nS],xh1[nS], yield_up1[nS], yield_down1[nS];
   double x2[nS], y2[nS], xl2[nS],xh2[nS], yield_up2[nS], yield_down2[nS];
   double nominal1, nominal2, staterr1, staterr2;

   //get Nominal plots
   plotname1=sample+"_2tag"+jetnumber+"_"+ ptvregion+"_"+region+"_mBBMVA";
   plotname2=sample+"_2tag"+jetnumber+"_"+ ptvregion+"_"+region+"_mBBMVA";
   plot1 = (TH1F*)file1->Get(plotname1);
   plot2 = (TH1F*)file2->Get(plotname2);
   if(plot1!=NULL) nominal1 = plot1->IntegralAndError(-1,99999, staterr1);
   if(plot2!=NULL) nominal2 = plot2->IntegralAndError(-1,99999, staterr2);


   for(int isys=0; isys<Systematics.size(); isys++){
     int igr=isys/2;
     if(Systematics.at(isys).Contains("up")){
       x1[igr]=igr-0.15;
       x2[igr]=igr+0.15;
       y1[igr]=nominal1;
       y2[igr]=nominal2;
       xl1[igr]=xh1[igr]=xl2[igr]=xh2[igr]=0;
       yield_up1[nS]=yield_down1[nS]=yield_up2[nS]=yield_down2[nS]=0;
     }
     double tesweight, refweight;
     plotname1=sample+"_2tag"+jetnumber+"_"+ ptvregion+"_"+region+"_mBBMVA_Sys"+Systematics1.at(isys);
     plotname2=sample+"_2tag"+jetnumber+"_"+ ptvregion+"_"+region+"_mBBMVA_Sys"+Systematics.at(isys);
     if(sample == "qqWlvH125"  && is2LICHEP)  // for MIA 2L ICHEP results, different histogram names
        plotname2="WlvH125_2tag"+jetnumber +"_"+ptvregion+"_"+region+"_mBBMVA_Sys"+Systematics1.at(isys);
     plot1 = (TH1F*)Dir1->Get(plotname1);
     plot2 = (TH1F*)Dir2->Get(plotname2);
     if(plot1==NULL) tesweight = nominal1;
     else tesweight = plot1->Integral(-1,99999);
     if(plot2==NULL) refweight = nominal2;
     else refweight = plot2->Integral(-1,99999);
     if(Systematics.at(isys).Contains("__1up")){
       yield_up1[igr]=tesweight-nominal1;
       yield_up2[igr]=refweight-nominal2;
     }
     if(Systematics.at(isys).Contains("__1down")){
        yield_down1[igr]=nominal1-tesweight;
        yield_down2[igr]=nominal2-refweight;
     }
  } //systematics bins
  
  //Draw all systematics
  TH1F* Nominal=new TH1F("","",nS,-0.5,nS-0.5);
  
  changeErrorPos(nS, yield_down1, yield_up1);
  changeErrorPos(nS, yield_down2, yield_up2);

  TGraphAsymmErrors *Gerr1, *Gerr2;
  TGraphAsymmErrors *Gfac1, *Gfac2;
  Gerr1 = new TGraphAsymmErrors(nS,x1,y1,xl1,xh1,yield_down1,yield_up1); 
  Gerr2 = new TGraphAsymmErrors(nS,x2,y2,xl2,xh2,yield_down2,yield_up2); 
  double fac_down1[nS], fac_up1[nS]; 
  double fac_down2[nS], fac_up2[nS]; 
  double One[nS], Stat[nS];
  for(int ib=0;ib<nS;ib++){
     One[ib]=1;
     Stat[ib]=staterr1/y1[ib]; // only show stat unc from test sample;
     fac_down1[ib] = yield_down1[ib]/y1[ib];
     fac_up1[ib]   = yield_up1[ib]/y1[ib];
     fac_down2[ib] = yield_down2[ib]/y2[ib];
     fac_up2[ib]   = yield_up2[ib]/y1[ib];
  }
  Gfac1 = new TGraphAsymmErrors(nS,x1,One,xl1,xh1,fac_down1,fac_up1); 
  Gfac2 = new TGraphAsymmErrors(nS,x2,One,xl2,xh2,fac_down2,fac_up2); 

  if(nS!=plotlabel.size()) cout<<"Error: Different number of bins !!!!!"<<endl;
  for(int ib=0;ib<nS;ib++){
    Nominal->GetXaxis()->SetBinLabel(ib+1,plotlabel.at(ib));
    Nominal->SetBinContent(ib+1,One[ib]);
    Nominal->SetBinError(ib+1,Stat[ib]);
  }
  Nominal->GetXaxis()->LabelsOption("v");
  Nominal->SetStats(0);
  Nominal->SetFillColor(kAzure-9);
  Nominal->SetMarkerColor(kAzure-9);
  Nominal->GetYaxis()->SetRangeUser(0.95,1.05);
  Nominal->GetYaxis()->SetTitle("Systematics/Nominal");
  Nominal->GetYaxis()->SetTitleSize(0.045);

  SetStyle(Gerr1,Gerr2);
  SetStyle(Gfac1,Gfac2);

  TF1 *line1 = new TF1("line1","1.0",0,nS+1);
  line1->SetLineColor(1);
  line1->SetLineWidth(1);
  line1->SetLineStyle(1);

  TCanvas *MyN = new TCanvas("Sys_Fac","",0,2,1500,600);
  TPad *padc = new TPad("padc","padc", 0,0,1.0,1.0);
  padc->SetBottomMargin(0.55);
  padc->SetTopMargin(0.05);
  MyN->Draw();
  padc->Draw();
  padc->cd();
  padc->SetGridy(1);

  Nominal->Draw("E2");
  line1->Draw("same");
  Gfac1->Draw("F");
  Gfac2->Draw("F"); 

  TLegend *legend = new TLegend(0.65, 0.8, 1, 0.95);
  legend->AddEntry(Nominal, "MC stat unc");
  legend->AddEntry(Gfac1, "32-10");
  legend->AddEntry(Gfac2, "ICHEP");
  legend->SetLineWidth(0);
  legend->SetFillStyle(0);
  legend->Draw("same");

//  MyN->SaveAs(sample+"_"+jetnumber+"_"+ptvregion+"_SR_JET_ICHEP.png");
//  MyN->SaveAs(sample+"_"+jetnumber+"_"+ptvregion+"_SR_JET_NEW.png");
  MyN->SaveAs(sample+"_"+jetnumber+"_"+ptvregion+"_"+region+"_all.png");
}

void SetStyle(TGraphAsymmErrors* &plot1, TGraphAsymmErrors* &plot2,   double Xmin = -1, double Xmax = -1){
  double inte1,inte2;
  inte1 = plot1->Integral();
  inte2 = plot2->Integral();

  plot1->GetXaxis()->SetRangeUser(Xmin,Xmax);
  plot1->SetTitle("");
  plot1->SetLineColor(2);
  plot1->SetLineWidth(2);
  plot1->SetMarkerColor(2);
  plot1->SetMarkerStyle(8);
  plot1->SetMarkerSize(0.7);
  plot2->SetLineColor(4);
  plot2->SetLineWidth(2);
  plot2->SetMarkerColor(4);
  plot2->SetMarkerStyle(8);
  plot2->SetMarkerSize(0.7);

  plot2->SetFillColor(0);
  plot1->SetFillColor(0);

 //Set title
  double title_size = 0.04;
  double label_size = 0.03;
  double title_offset = 1.3;
  double scale_plot  = 0.7;
  double scale_ratio = 0.3;
  plot1->GetYaxis()->SetTitleSize(title_size/scale_plot);
  plot1->GetYaxis()->SetTitleOffset(title_offset*scale_plot);
  plot1->GetYaxis()->SetLabelSize(label_size/scale_plot);
}

void changeErrorPos(int N, double *down, double *up){
  for(int i=0;i<N;i++){
    double d=down[i];
    double u=up[i];
    bool useD = (fabs(d) > fabs(u));
    if(d>=0 && u>=0) continue;
    if(d>=0 && u <0){
       up[i]=0;
       if(useD) continue;
       else down[i]=-u;
    }
    if(d< 0 && u>=0){
       down[i]=0;
       if(useD) up[i]=-d;
       else continue;
    }
    if( d<0 && u<0){
       down[i]=-u;
       up[i]=-d;
    }
  }
}
