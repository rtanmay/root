% ROOT Version 6.18 Release Notes
% 2018-11-14
<a name="TopOfPage"></a>

## Introduction

ROOT version 6.18/00 is scheduled for release in May, 2019.

For more information, see:

[http://root.cern](http://root.cern)

The following people have contributed to this new version:

 Bertrand Bellenot, CERN/SFT,\
 Rene Brun, CERN/SFT,\
 Philippe Canal, FNAL,\
 Olivier Couet, CERN/SFT,\
 Gerri Ganis, CERN/SFT,\
 Andrei Gheata, CERN/SFT,\
 Stephan Hageboeck, CERN/SFT,\
 Sergey Linev, GSI,\
 Pere Mato, CERN/SFT,\
 Lorenzo Moneta, CERN/SFT,\
 Axel Naumann, CERN/SFT,\
 Danilo Piparo, CERN/SFT,\
 Fons Rademakers, CERN/SFT,\
 Enric Tejedor Saavedra, CERN/SFT,\
 Matevz Tadel, UCSD/CMS,\
 Vassil Vassilev, Princeton/CMS,\
 Wouter Verkerke, NIKHEF/Atlas,

## Deprecation and Removal

### THttpServer classes

The following methods were deprecated and removed:

   * Bool_t THttpServer::SubmitHttp(THttpCallArg *arg, Bool_t can_run_immediately = kFALSE, Bool_t ownership = kFALSE);
   * Bool_t THttpServer::ExecuteHttp(THttpCallArg *arg)
   * Bool_t TRootSniffer::Produce(const char *path, const char *file, const char *options, void *&ptr, Long_t &length, TString &str);
   * TString THttpCallArg::GetPostDataAsString();
   * void THttpCallArg::FillHttpHeader(TString &buf, const char *header = nullptr);
   * void THttpCallArg::SetBinData(void *data, Long_t length);

The methods could be replaced by equivalent methods with other signature:

   * Bool_t THttpServer::SubmitHttp(std::shared_ptr<THttpCallArg> arg, Bool_t can_run_immediately = kFALSE);
   * Bool_t THttpServer::ExecuteHttp(std::shared_ptr<THttpCallArg> arg);
   * Bool_t TRootSniffer::Produce(const std::string &path, const std::string &file, const std::string &options, std::string &res);
   * const void *THttpCallArg::GetPostData() const;
   * Long_t THttpCallArg::GetPostDataLength() const;
   * std::string THttpCallArg::FillHttpHeader(const char *header = nullptr);
   * void THttpCallArg::SetContent(std::string &&cont);


## Core Libraries


## I/O Libraries


## TTree Libraries

### RDataFrame
  - Use TPRegexp instead of TRegexp to interpret the regex used to select columns
  in the invocation of `Cache` and `Snapshot`.


## Histogram Libraries


## Math Libraries


## RooFit Libraries
  - HistFactory: hist2workspace performance optimisations. For a large, ATLAS-style Higgs-->bb workspace with > 100 systematic uncertainties and more than
    10 channels, the run time decreases by a factor 11 to 12.

## 2D Graphics Libraries

  - In the statistics painting for 2D histograms, the central cell of
    the underflow/overflow grid was not properly rendered for very large contents.
    This problem was reported [here](https://root-forum.cern.ch/t/stat-box-for-th2/).
  - The automatic placement of legend now "sees" TMultiGraph and THStack.
  - Improve and simplify the drawing the 2D histogram's option "ARR".
  - The option ARR can be combined with the option COL or COLZ.
  - `TBox::DistancetoPrimitive` and `TBox::ExecuteEvent` now work in log scales (by Jérémie Dudouet).
  - Take the line attributes into account when drawing a histogram with option bar or hbar.
    They were ignored until now.

## 3D Graphics Libraries

  - Make sure a TF3 is painted the same way in GL and non-GL mode.
    The mismatch was reported in [this post](https://root-forum.cern.ch/t/how-to-specify-the-level-value-in-isosurface-drawing-with-tf3-and-gl/32179)

## Geometry Libraries


## Database Libraries


## Networking Libraries


## GUI Libraries


## Montecarlo Libraries


## PROOF Libraries


## Language Bindings


## JavaScript ROOT


## Tutorials


## Class Reference Guide


## Build, Configuration and Testing Infrastructure


