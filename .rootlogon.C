{
  TStyle* tauStyle = new TStyle("tauStyle","dune-tau-sim style");
  tauStyle->SetPalette(1,0); // avoid horrible default color scheme
  tauStyle->SetOptStat(0);
  tauStyle->SetOptTitle(0);
  tauStyle->SetOptDate(0);
  tauStyle->SetLabelSize(0.03,"xyz"); // size of axis value font
  tauStyle->SetTitleSize(0.035,"xyz"); // size of axis title font
  tauStyle->SetTitleFont(22,"xyz"); // font option
  tauStyle->SetLabelFont(22,"xyz");
  tauStyle->SetTitleOffset(1.2,"y");
  gROOT->SetStyle("tauStyle");
  return;
}
