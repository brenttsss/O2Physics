// O2 Framework includes
#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"

// C++ includes
#include <iostream>
#include <fstream>
#include <vector>

using namespace o2;
using namespace o2::framework;

// Output text file to store selected muon track information
std::ofstream muonTracksOut("muontracks.txt");

// Define analysis task
struct HFMuonFwdTracks {
    // Histogram registry to store histograms
    HistogramRegistry histos{"histos", {},
    OutputObjHandlingPolicy::AnalysisObject};

    // Vector to keep track of selected muon MC track IDs to avoid
    // writing duplicate/ambiguous tracks
    std::vector<int64_t> selectedTracksID;

    // Initialization function
    void init(InitContext const&)
    {
        // Define histogram axes: for event counting, η range, and pT range
        const AxisSpec axisCounter{1, 0, +1, ""};
        const AxisSpec axisEta{10, -4.0, -2.5, "#eta"}; // forward region
        const AxisSpec axisPt{10, 0.0, +20.0, "p_{T} (GeV/c)"};

        // Define and register histograms
        histos.add("eventCounterReco", "eventCounterReco", kTH1F, {axisCounter});
        histos.add("muPtHistReco", "muPtHistReco", kTH1F, {axisPt});
        histos.add("muPtHistRecoD", "muPtHistRecoD", kTH1F, {axisPt});

        // Set textfile header for output file
        muonTracksOut << "ID,eta,pt,p,phi,motherPDG,nClusters,pDca,chi2,chi2MatchMCHMID,chi2MatchMCHMFT,isPrompt\n";
    }

    // Define input table: joins reconstructed forward tracks with their Monte Carl labels
    using muonTracks = soa::Join<aod::FwdTracks, aod::McFwdTrackLabels>;

    // Main processing function
    void process(aod::Collision const& collision, muonTracks const& tracks, aod::McParticles const&)
    {
        // Count each event processed
        histos.fill(HIST("eventCounterReco"), 0.5);

        // Loop over all matched tracks
        for (auto& track : tracks) {
            if(track.has_mcParticle()){
                auto mcParticle = track.mcParticle();

                //auto statusCode = mcParticle.getGenStatusCode();

                // Select only true muons (PDG13)
                if (abs(mcParticle.pdgCode())==13) {
                    auto muID = mcParticle.globalIndex();
                    auto muEta = mcParticle.eta();

                    // Fill histogram for true muons in forward η range
                    if (muEta >= -4.0 && muEta <= -2.5) histos.fill(HIST("muPtHistReco"), mcParticle.pt()); // forward eta region

                    // Check for duplicate muon entries (ambiguous track)
                    int occuranceCount = count(selectedTracksID.begin(), selectedTracksID.end(), muID);

                    // Write to textfile
                    if (occuranceCount < 1) {
                        // Exctract reconstructed kinematics
                        auto muRecoEta = track.eta();
                        auto muRecoPt = track.pt();
                        auto muRecoP = track.p();
                        auto muRecoPhi = track.phi();
                        auto muCluster = track.nClusters();
                        auto muDCA = track.pDca();
                        auto muChi2 = track.chi2();
                        auto muChi2MCHMID = track.chi2MatchMCHMID();
                        auto muChi2MCHMFT = track.chi2MatchMCHMFT();

                        // Access MC HF mother of muon
                        auto muMother = mcParticle.mothers_first_as<aod::McParticles>(); // Retrieve the first mother particle of the muon
                        auto muMotherPDG = abs(muMother.pdgCode()); // Access its PDG code to idenify the parent
                        auto motherStatusCode = muMother.getGenStatusCode();

                        // Fill separate histogram if muon is from HF D-mesons
                        if (abs(muMotherPDG) >= 411 && abs(muMotherPDG) <= 435) { // D meson parent - can be prompt/no-prompt
                            histos.fill(HIST("muPtHistRecoD"), mcParticle.pt());
                        }

                        // Determine if the muon originates from a prompt HF hadron
                        auto mcPart(muMother);
                        auto prevMcPart(muMother);
                        auto mcPartPDG = abs(mcPart.pdgCode());
                        int isPrompt = 1;

                        std::cout << "==== Forward muon decay chain: mu";

                        // Traverse the decay chain backwards
                        while (mcPart.has_mothers() && (abs(mcPart.getGenStatusCode()) > 80 || mcPart.getGenStatusCode() == 0)) { // print out mother chain
                            std::cout << " <- " << mcPartPDG;
                            prevMcPart = *(mcPart);
                            mcPart = *(mcPart.mothers_first_as<aod::McParticles>());
                            mcPartPDG = abs(mcPart.pdgCode());
                        }

                        // Prompt if last strong-decaying ancestor shares same meson family as first HF mother
                        if (div(abs(prevMcPart.pdgCode()), 100).quot != div(muMotherPDG, 100).quot) isPrompt = 0;

                        std::cout << "; isPrompt = " << isPrompt << std::endl;

                        // Write muon info to textfile
                        muonTracksOut << muID << "," << muRecoEta << "," << muRecoPt << "," << muRecoP << "," << muRecoPhi << "," << muMotherPDG << "," << std::to_string(muCluster) << "," << muDCA << "," << muChi2 << "," << muChi2MCHMID << "," << muChi2MCHMFT << "," << isPrompt << std::endl;

                        // Track ID registered to prevent reprocessing
                        selectedTracksID.emplace(selectedTracksID.end(), muID);
                    }
                }
            }
        }
    }
};

// Register task in workflow
WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<HFMuonFwdTracks>(cfgc)};
}