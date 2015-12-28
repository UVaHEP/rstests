///////////////////////////////////////////////////////
// The rootmacro skeletonMacro.C takes as inputs the 
// Workspace and Model created from skeletonMacro.C then
// calls a number of RooStats methods.
//

// Read the workspace from the file and get a pointer to
// the ModelConfig and data set objects. Create the Profile 
// likelihood calculator class. Run the calculator by 
// getting the interval at the corresponding desired
// confidence level. Plot the interval and the profile
// likelihood function.

///////////////////////////////////////
// Initially borrow code from 2015 Terascale Stat School
// which we will generalize (if possible). Below uses
// (Terascale Roostats tutorial) SimplePL. My guess is the PL in
// that naming convention stands for Profile Likelihood.
// Note that the SimplePL contains much of the code from 
// $ROOTSYS/root/tutorials/rootstats/StandardProfileLikelihood.C

 using namespace RooStats;

//void SimplePL(  
void makeMacro(
   const char* infile =  "SampleWorkspaceFile.root", 
   const char* workspaceName = "w",
   const char* modelConfigName = "ModelConfig",
   const char* dataName = "obsData" )
{
  /////////////////////////////////////////////////////////////
  // First part is just to access the workspace file 
  ////////////////////////////////////////////////////////////

  // open input file 
  TFile *file = TFile::Open(infile);
  if (!file) return;

  // get the workspace out of the file
  RooWorkspace* w = (RooWorkspace*) file->Get(workspaceName);


  // get the modelConfig out of the file
  RooStats::ModelConfig* mc = (RooStats::ModelConfig*) w->obj(modelConfigName);

  // get the modelConfig out of the file
  RooAbsData* data = w->data(dataName);


  ProfileLikelihoodCalculator pl(*data,*mc);
  pl.SetConfidenceLevel(0.683); // 68% interval
  LikelihoodInterval* interval = pl.GetInterval();

   // find the iterval on the first Parameter of Interest
  RooRealVar* firstPOI = (RooRealVar*) mc->GetParametersOfInterest()->first();

  double lowerLimit = interval->LowerLimit(*firstPOI);
  double upperLimit = interval->UpperLimit(*firstPOI);


  cout << "\n68% interval on " <<firstPOI->GetName()<<" is : ["<<
    lowerLimit << ", "<<
    upperLimit <<"] "<<endl;


  LikelihoodIntervalPlot * plot = new LikelihoodIntervalPlot(interval);
//   if (TString(infile) == "GausExpModel.root")
//      plot->SetRange(0,150);  // possible eventually to change ranges
  //plot->SetNPoints(50);  // do not use too many points, it could become very slow for some models
  plot->Draw("tf1");  // use option TF1 if too slow (plot.Draw("tf1")
  
}
