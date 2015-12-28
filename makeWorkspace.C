/*
This is based on the tutorial: "Higgs analysis at ATLAS using RooStats"; https://twiki.ppe.gla.ac.uk/bin/view/ATLAS/HiggsAnalysisAtATLASUsingRooStats ;  make_test_histograms.cxx
*/
using namespace std;
using namespace RooFit;
using namespace RooStats;

void makeWorkspace() {

  /////////////////////////////////////////////////////
  ///
  /// Code to create the histograms and save them to a root file based on the
  /// tutorial page: "Higgs analysis at ATLAS using RooStats";
  /// https://twiki.ppe.gla.ac.uk/bin/view/ATLAS/HiggsAnalysisAtATLASUsingRooStats
  ///;  make_test_histograms.cxx
  ///
  ////////////////////////////////////////////////////
  
    // Signal histogram
    // Create the function used to describe the signal shape
  // Fun fact, the ROOT Math BW constructor goes X, gamma, xnaught
  // while the RooBW constructor goes X, xnaught, gamma. So that took
  // awhile to figure out.
    TF1 mySignalFunction("mySignalFunction","ROOT::Math::breitwigner_pdf(x,0.5,1.5)"
			 , 0, 10);
    // Create the histogram with 100 bins between 0 and 10
    TH1F mySignal("mySignal", "mySignal", 100, 0, 10);
    // Fill the histogram using the signal function.
    mySignal.FillRandom("mySignalFunction", 200);

    // Background histogram
    // Create the function used to describe the background shape 
    TF1 myBackgroundFunction("myBackgroundFunction","ROOT::Math::gamma_pdf(x,2.0,2.0)"
			     , 0, 10);
    // Create the histogram with 100 bins between 0 and 10 
    TH1F myBackground("myBackground", "myBackground", 100, 0, 10);
    // Fill the histogram using the background function.
    myBackground.FillRandom("myBackgroundFunction", 2000);

    // Create the (simulated) data histogram by combining the signal 
    // (unscaled) and background histograms
    TH1F myData = 1 * mySignal + myBackground;
    // Set the name and title
    myData.SetName("myData");
    myData.SetTitle("myData");

    // Save the histograms created to a ROOT file
    TFile myFile("SampleInputHistos.root", "RECREATE");
    mySignal.Write();
    myBackground.Write();
    myData.Write();
    myFile.Close();

    ///////////////////////////////////////////////////////////////////
    /// Now see if you can save the data as a RooDataHist
    ///
    /// Based on "Higgs analysis at ATLAS using RooStats"
    /// example code:
    /// import a ROOT histogram into a RooDataHist (a RooFit binned dataset)
    ///
    /* FOR LATER: Make it such that you can open a rootfile
    // Access the file.
    TFile* myFile = new TFile("SampleInputHistos.root");
    // Load the histogram.
    
     myFile.Open("SampleInputHistos.root");
     TH1F* myData = (TH1F*) myFile.Get("myData");
    */
    //////////////////////////////////////////////////////////////////
   
    myData.Draw();
        
    // RooDataHist below didn't like the TH1F direct use of myData and
    // class seemed to specify TH1, so just to be safe, I did a casting
    // clone to make the TH1 pointer myHistogram  
    TH1* myHistogram = (TH1*)myData.Clone("myHistogram");

    // Declare an observable x.
    RooRealVar x("x", "x", 0, 10);

    // Create a binned dataset that imports the contents of TH1 and
    // associates its contents to observable 'x'.
    RooDataHist myDataImport("myDataImport", "myDataImport", RooArgList(x),
    			     myHistogram);

    // Plot the imported dataset.
    TCanvas *c1 = new TCanvas("c1","Data Histo Imported as RooDataHist",600,600);
    
    RooPlot* myFrame = x.frame(Title("Created from Combing S+B TF1/TH1, Poisson Stat Error"));
    myDataImport.plotOn(myFrame,MarkerColor(6),LineColor(6),LineStyle(kDashed));
    myFrame->Draw();

    ////////////////////////////////////////////////////////
    /// Recap, above we used TF1/TH1 functions and histograms to create
    /// sample signal, background and data histograms, followed by a
    /// RooDataHist (binned RooFit dataset) of the data.
    ///
    /// Let's see if we do the same thing within a workspace
    ///
    // Note that in RooFit UNBINNED data is stored using the RooDataSet
    // class and BINNED data is stored with RooDataHist. This means it's
    // not going to be easy boping back and forth between the two.
    // With that in mind, it may be easier to create a RooDataSet from the
    // BGRD and SIGNAL PDFs rather than making them into binned histos first.
    //
    // This portion follows (roughly) the fitHgg.C example macro from the
    // 2015 TetraScale Summer School Roofit/Roostat tutorial
    //
    ////////////////////////////////////////////////////////
    
   RooWorkspace w("w");
   // define a Breit-Wigner pdf, signal 
   w.factory("BreitWigner:signalmodel(x[0,10],gam[1.5],xnaught[0.5])");
   // create pointer to model
   RooAbsPdf *signalmodel = w.pdf("signalmodel");
   
   // define a Gamma funct pdf, background
   // note, the variable x was defined above, so now I don't need to
   // include the range when I use it subsequently
   w.factory("Gamma:backgroundmodel(x,alpha[2.0],theta[2.0],0.0)");
   // created pointer to model
   RooAbsPdf *backgroundmodel = w.pdf("backgroundmodel");

   // create a model from the signal and background
   w.factory("nsig[200.0, 0.0, 1000.0]");
   w.factory("nback[2000.0, 0.0, 5000.0]");
   w.factory("SUM::model(nsig*signalmodel, nback*backgroundmodel)");
   RooAbsPdf *model = w.pdf("model");
   
   // but what can we do to generate a RooDataSet from the same input?
   // Looks like we can create two parts of the dataset, then use the
   // merge() function to put them together.
   RooDataSet *datapt1 = signalmodel->generate(x,200);
   RooDataSet* data = (RooDataSet*)datapt1->Clone("data");
   RooDataSet *datapt2 = backgroundmodel->generate(x,2000);
  
   //data->merge(datapt2);
   data->append(*datapt2);
   
   // plot data and function
   TCanvas *c2 = new TCanvas("c2","Check Two Data Shapes",600,600);
   RooPlot * plot = w.var("x")->frame(Title("Sanity Check Data Overlay"));
   
   // Check to make sure that the RooDataHist plot made from the
   // histogram additions is fundementally the same shape and stats
   // as the one made from the PDFs
   data->plotOn(plot,Name("data"));
   myDataImport.plotOn(plot,Name("myDataImport"),MarkerColor(6),LineStyle(kDashed),LineColor(6));
   plot->Draw();
   TLegend *leg1 = new TLegend(0.65,0.73,0.86,0.87);
   leg1->AddEntry("myDataImport","From TF1/TH1","LP");
   leg1->AddEntry("data","From PDFs","LP");
   leg1->Draw();

   ///////////////////////////////////////////////////////////
   ///
   /// At this point we're in good shape, we have both a binned
   /// (myDataImport) and unbinned (data) data set, and can work
   /// with PDFs to get shapes.
   
   RooFitResult *r = model->fitTo(*data, Minimizer("Minuit2"),Save(true),
				  Offset(true));
   // plot data and function
   TCanvas *c3 = new TCanvas("c3","Fit Data w/ PDFs",600,600);
   RooPlot * plot2 = w.var("x")->frame(Title("Fitting Data with PDF S+B"));
   data->plotOn(plot2,Name("data"));
   model->plotOn(plot2,Name("model"),LineColor(8));
   model->plotOn(plot2, Name("bkdmodel"),Components("backgroundmodel"),LineStyle(kDashed));
   model->plotOn(plot2, Name("sigmodel"),Components("signalmodel"),LineColor(kRed));
   plot2->Draw();
TLegend *leg2 = new TLegend(0.65,0.73,0.86,0.87);
   leg2->AddEntry("model","S+B model","L");
   leg2->AddEntry("data","Data (From PDFs)","LP");
   leg2->AddEntry("bkdmodel","Background","L");
   leg2->AddEntry("sigmodel","Signal","L");
   leg2->Draw();  
///////// below is beginning of ModelConfig 

RooStats::ModelConfig mc("ModelConfig",&w);
mc.SetPdf(*w.pdf("model"));
// setting the param of interest to be the amount of signal
//
// futher below I set the two shape parameters of the BW PDF
// signal form to be constant. This is following the tutorial, but
// I think that I could rearrange these lines more logically.
mc.SetParametersOfInterest(*w.var("nsig"));

mc.SetObservables(*w.var("x"));

// define set of nuisance parameters
// 
// these are things that can fluctuate, so for now we're just letting that be the number
// of background events, but I could also have the parameters of the gamma function
// describing the background PDF (alpha, theta) fluctuate. However, in order to do that
// I believe I would need to set them as varables with ranges (like I did nback), rather than
// as constants. L 
w.defineSet("nuisParams","nback");

mc.SetNuisanceParameters(*w.set("nuisParams"));

// similarly, the below are the shape parameters for the signal (BW) PDF
// here I declare them to be be (explicitly) constant, but as I defined them as
// constants to begin with, it seems to be irrelevant. 
w.var("gam")->setConstant(true);
w.var("xnaught")->setConstant(true);


// import the ModelConfig in the workspace
w.import(mc); 

// import the data 
// according to the tutorial for performance reasons we transform the data in a binned data set
//
// SMZ note, to me this is kind of weird because I spent all of that time
// creating an unbinned RooDataSet (data) when I already had a binned
// one (myDataImport). Maybe I can stick them both in, and see what I get.
TH1 * h1 = data->createHistogram("x");
RooDataHist binData("obsData","obsData",*w.var("x"), h1); 
w.import(binData);
w.import(myDataImport); 

// write workspace in the file 
w.writeToFile("SampleWorkspaceFile.root");  
 
  
}
