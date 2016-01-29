////////////////////////////////////////////////////////////////////
// Implementation of the Blurred Clustering algorithm
//
// Converts a hit map into a 2D image of the hits before convoling
// with a Gaussian function to introduce a weighted blurring.
// Clustering proceeds on this blurred image to create more
// complete clusters.
//
// M Wallbank (m.wallbank@sheffield.ac.uk), May 2015
////////////////////////////////////////////////////////////////////

#ifndef BlurredClustering_h
#define BlurredClustering_h

// Framework includes
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// LArSoft includes
#include "Utilities/LArProperties.h"
#include "Utilities/DetectorProperties.h"
#include "Utilities/AssociationUtil.h"
#include "RecoBase/Hit.h"
#include "RecoBase/Track.h"
#include "RecoBase/SpacePoint.h"
#include "Geometry/PlaneGeo.h"
#include "Geometry/WireGeo.h"
#include "Geometry/Geometry.h"
#include "SimulationBase/MCParticle.h"
#include "MCCheater/BackTracker.h"

// ROOT & C++
#include <TTree.h>
#include <TH2F.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TCutG.h>
#include <TString.h>
#include <TMarker.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TVirtualPad.h>
#include <TLatex.h>
#include <TGraph.h>
#include <TF1.h>
#include <TLine.h>
#include <TPrincipal.h>
#include <TMath.h>
#include <TVector.h>
#include <TVectorD.h>
#include <TVector2.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>


namespace cluster {
  class BlurredClusteringAlg;
}

class cluster::BlurredClusteringAlg {
public:

  BlurredClusteringAlg(fhicl::ParameterSet const& pset);
  virtual ~BlurredClusteringAlg();

  void reconfigure(fhicl::ParameterSet const&p);

  ///
  void ConvertBinsToClusters(std::vector<std::vector<double> > const& image,
			     std::vector<std::vector<int> > const& allClusterBins,
			     std::vector<art::PtrVector<recob::Hit> >& clusters);

  ///
  void CreateDebugPDF(int run, int subrun, int event);

  ///
  std::vector<std::vector<double> > ConvertRecobHitsToTH2(std::vector<art::Ptr<recob::Hit> > const& hits);

  ///
  int FindClusters(std::vector<std::vector<double> > const& image, std::vector<std::vector<int> >& allcluster);

  ///
  int GlobalWire(geo::WireID const& wireID);

  ///
  std::vector<std::vector<double> > GaussianBlur(std::vector<std::vector<double> > const& image);

  ///
  unsigned int GetMinSize() { return fMinSize; }

  ///
  TH2F* MakeHistogram(std::vector<std::vector<double> > const& image, TString name);

  ///
  void RemoveTrackHits(std::vector<art::Ptr<recob::Hit> > const& ihits,
		       std::vector<art::Ptr<recob::Track> > const& tracks,
		       std::vector<art::Ptr<recob::SpacePoint> > const& spacePoints,
		       art::FindManyP<recob::Track> const& fmth,
		       art::FindManyP<recob::Track> const& fmtsp,
		       art::FindManyP<recob::Hit> const& fmh,
		       std::vector<art::Ptr<recob::Hit> >& hits,
		       int Event, int Run);

  ///
  void SaveImage(TH2F* image, std::vector<art::PtrVector<recob::Hit> > const& allClusters, int pad, int tpc, int plane);
  ///
  void SaveImage(TH2F* image, int pad, int tpc, int plane);
  ///
  void SaveImage(TH2F* image, std::vector<std::vector<int> > const& allClusterBins, int pad, int tpc, int plane);

  std::map<int,std::map<int,art::Ptr<recob::Hit> > > fHitMap;

private:

  ///
  art::PtrVector<recob::Hit> ConvertBinsToRecobHits(std::vector<std::vector<double> > const& image, std::vector<int> const& bins);

  ///
  art::Ptr<recob::Hit> ConvertBinToRecobHit(std::vector<std::vector<double> > const& image, int bin);

  ///
  std::vector<std::vector<double> > Convolve(std::vector<std::vector<double> > const& image, std::vector<double> const& kernel, int width, int height);//, const char *new_name = 0);

  ///
  int ConvertWireTickToBin(std::vector<std::vector<double> > const& image, int xbin, int ybin);

  ///
  int ConvertBinToCharge(std::vector<std::vector<double> > const& image, int bin);

  ///
  void FindBlurringParameters(int& blurwire, int& blurtick, int& sigmawire, int& sigmatick);

  ///
  double GetTimeOfBin(std::vector<std::vector<double> > const& image, int bin);

  ///
  unsigned int NumNeighbours(int nx, std::vector<bool> const& used, int bin);

  ///
  bool PassesTimeCut(std::vector<double> const& times, double time);

  // Parameters used in the Blurred Clustering algorithm
  int          fBlurWire;                 // blur radius for Gauss kernel in the wire direction
  int          fBlurTick;                 // blur radius for Gauss kernel in the tick direction
  double       fBlurSigma;                // sigma for Gaussian kernel
  int          fClusterWireDistance;      // how far to cluster from seed in wire direction
  int          fClusterTickDistance;      // how far to cluster from seed in tick direction
  unsigned int fMinMergeClusterSize;      // minimum size of a cluster to consider merging it to another
  double       fMergingThreshold;        // the PCA eigenvalue needed to consider two clusters a merge
  unsigned int fNeighboursThreshold;      // min. number of neighbors to add to cluster
  int          fMinNeighbours;            // minumum number of neighbors to keep in the cluster
  unsigned int fMinSize;                  // minimum size for cluster
  double       fMinSeed;                  // minimum seed after blurring needed before clustering proceeds
  double       fTimeThreshold;            // time threshold for clustering
  double       fChargeThreshold;          // charge threshold for clustering

  // Wire and tick information for histograms
  int fLowerHistTick, fUpperHistTick;
  int fLowerHistWire, fUpperHistWire;

  // Blurring stuff
  int fLastBlurWire;
  int fLastBlurTick;
  double fLastSigma;
  std::vector<double> fLastKernel;

  // For the debug pdf
  TCanvas *fDebugCanvas;
  std::string fDebugPDFName;

  // art service handles
  art::ServiceHandle<geo::Geometry> fGeom;
  art::ServiceHandle<util::DetectorProperties> fDetProp;

};

#endif
