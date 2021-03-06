// Tests for the RooWorkspace
// Authors: Stephan Hageboeck, CERN  01/2019
#include "RooWorkspace.h"
#include "RooGlobalFunc.h"
#include "RooHelpers.h"
#include "RooGaussian.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooAbsReal.h"
#include "RooStats/ModelConfig.h"

#include "TFile.h"
#include "TSystem.h"

#include "gtest/gtest.h"

using namespace RooStats;

/// ROOT-9777, cloning a RooWorkspace. The ModelConfig did not get updated
/// when a workspace was cloned, and was hence pointing to a non-existing workspace.
///
TEST(RooWorkspace, CloneModelConfig_ROOT_9777)
{
   const char* filename = "ROOT-9777.root";

   RooRealVar x("x", "x", 1, 0, 10);
   RooRealVar mu("mu", "mu", 1, 0, 10);
   RooRealVar sigma("sigma", "sigma", 1, 0, 10);

   RooGaussian pdf("Gauss", "Gauss", x, mu, sigma);

   {
      TFile outfile(filename, "RECREATE");
      
      // now create the model config for this problem
      RooWorkspace* w = new RooWorkspace("ws");
      ModelConfig modelConfig("ModelConfig", w);
      modelConfig.SetPdf(pdf);
      modelConfig.SetParametersOfInterest(RooArgSet(sigma));
      modelConfig.SetGlobalObservables(RooArgSet(mu));
      w->import(modelConfig);

      outfile.WriteObject(w, "ws");
      delete w;
   }
   
   RooWorkspace *w2;
   {
      TFile infile(filename, "READ");
      RooWorkspace *w;
      infile.GetObject("ws", w);
      ASSERT_TRUE(w) << "Workspace not read from file.";

      w2 = new RooWorkspace(*w);
      delete w;
   }
   
   w2->Print();

   ModelConfig *mc = dynamic_cast<ModelConfig*>(w2->genobj("ModelConfig"));
   ASSERT_TRUE(mc) << "ModelConfig not retrieved.";
   mc->Print();

   ASSERT_TRUE(mc->GetGlobalObservables()) << "GlobalObsevables in mc broken.";
   mc->GetGlobalObservables()->Print();

   ASSERT_TRUE(mc->GetParametersOfInterest()) << "ParametersOfInterest in mc broken.";
   mc->GetParametersOfInterest()->Print();

   gSystem->Unlink(filename);
}



/// Set up a simple workspace for later tests.
class TestRooWorkspaceWithGaussian : public ::testing::Test {
protected:
  TestRooWorkspaceWithGaussian() :
  Test()
  {
    RooRealVar x("x", "x", 1, 0, 10);
    RooRealVar mu("mu", "mu", 1, 0, 10);
    RooRealVar sigma("sigma", "sigma", 1, 0, 10);

    RooGaussian pdf("Gauss", "Gauss", x, mu, sigma);

    TFile outfile(_filename, "RECREATE");

    // now create the model config for this problem
    RooWorkspace w("ws");
    RooStats::ModelConfig modelConfig("ModelConfig", &w);
    modelConfig.SetPdf(pdf);
    modelConfig.SetParametersOfInterest(RooArgSet(sigma));
    modelConfig.SetGlobalObservables(RooArgSet(mu));
    w.import(modelConfig);

    outfile.WriteObject(&w, "ws");
  }

  ~TestRooWorkspaceWithGaussian() {
    gSystem->Unlink(_filename);
  }

  const char* _filename = "ROOT-9777.root";
};


/// Test the string tokeniser that does all the string splitting for the RooWorkspace
/// implementation.
TEST(RooHelpers, Tokeniser)
{
  std::vector<std::string> tok = RooHelpers::tokenise("abc, def, ghi", ", ");
  EXPECT_EQ(tok.size(), 3U);
  EXPECT_EQ(tok[0], "abc");
  EXPECT_EQ(tok[1], "def");
  EXPECT_EQ(tok[2], "ghi");

  std::vector<std::string> tok2 = RooHelpers::tokenise("abc, def", ":");
  EXPECT_EQ(tok2.size(), 1U);
  EXPECT_EQ(tok2[0], "abc, def");

  std::vector<std::string> tok3 = RooHelpers::tokenise(",  ,abc, def,", ", ");
  EXPECT_EQ(tok3.size(), 2U);
  EXPECT_EQ(tok3[0], "abc");
  EXPECT_EQ(tok3[1], "def");

  std::vector<std::string> tok4 = RooHelpers::tokenise(",  ,abc, def,", ",");
  EXPECT_EQ(tok4.size(), 3U);
  EXPECT_EQ(tok4[0], "  ");
  EXPECT_EQ(tok4[1], "abc");
  EXPECT_EQ(tok4[2], " def");


}

/// Test proper string handling when importing an object from a workspace
/// in a different file.
TEST_F(TestRooWorkspaceWithGaussian, ImportFromFile)
{
  std::ostringstream spec;
  spec << _filename << ":" << "ws:Gauss";

  RooWorkspace w("ws");

  //Expect successful import:
  EXPECT_FALSE(w.import(spec.str().c_str()));
  //Expect import failures:
  EXPECT_TRUE(w.import("bogus:abc"));
  EXPECT_TRUE(w.import( (spec.str()+"bogus").c_str()));
}



