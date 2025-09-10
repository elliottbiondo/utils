//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/SensitiveDetector.cc
//---------------------------------------------------------------------------//
#include "SensitiveDetector.hh"

#include <G4SystemOfUnits.hh>
#include <corecel/Assert.hh>
#include <corecel/io/Logger.hh>
#include <corecel/math/ArrayUtils.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with sensitive detector name.
 */
SensitiveDetector::SensitiveDetector(std::string sd_name)
    : G4VSensitiveDetector(sd_name)
{
    CELER_VALIDATE(!sd_name.empty(),
                   << "must provide a valid sensitive detector name");

    JsonReader::Validate(JsonReader::Instance(), "celeritas");
    auto const& json = JsonReader::Instance().at("celeritas");
    if (json.contains("offload_particles"))
    {
        valid_pdgs_ = json.at("offload_particles").get<std::vector<PDG>>();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Callback interface between Geant4 and Celeritas.
 */
G4bool SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    using celeritas::Array;

    CELER_EXPECT(step);
    auto* track = step->GetTrack();
    CELER_ASSERT(track);
    auto* pd = track->GetParticleDefinition();
    CELER_ASSERT(pd);

    if (!this->is_pdg_valid(pd->GetPDGEncoding()))
    {
        // Do not score particles that aren't in the offload list
        return false;
    }

    auto* pre = step->GetPreStepPoint();
    CELER_ASSERT(pre);
    auto& pre_th = pre->GetTouchableHandle();
    CELER_ASSERT(pre_th);
    auto* phys_vol = pre_th->GetVolume();
    CELER_ASSERT(phys_vol);

    auto rio = RootIO::Instance();
    auto& data = rio->Histograms().Find(phys_vol->GetInstanceID(),
                                        phys_vol->GetCopyNo());

#define SD_1D_FILL(MEMBER, VALUE) data.MEMBER.Fill(VALUE);
#define SD_2D_FILL(MEMBER, X, Y) data.MEMBER.Fill(X, Y);
#define SD_1D_FILL_WEIGHT(MEMBER, VALUE, WEIGHT)         \
    {                                                    \
        auto& h = data.MEMBER;                           \
        auto const i = h.FindBin(VALUE);                 \
        h.SetBinContent(i, h.GetBinContent(i) + WEIGHT); \
    }

    auto const& pre_pos = pre->GetPosition() / cm;
    auto const len = step->GetStepLength() / cm;
    auto const edep = step->GetTotalEnergyDeposit();

    // Add total energy deposit for this event for this SD
    data.total_edep += edep;

    SD_1D_FILL_WEIGHT(energy_dep_x, pre_pos.x(), edep)
    SD_1D_FILL_WEIGHT(energy_dep_y, pre_pos.y(), edep)
    SD_1D_FILL_WEIGHT(energy_dep_z, pre_pos.z(), edep)
    SD_1D_FILL(step_len, len)
    SD_2D_FILL(pos_xy, pre_pos.x(), pre_pos.y())
    SD_1D_FILL(time, pre->GetGlobalTime())

    auto is_equal = [](G4ThreeVector const& a, G4ThreeVector const& b) -> bool {
        return a.x() == b.x() && a.y() == b.y() && a.z() == b.z();
    };
    auto to_array
        = [](CLHEP::Hep3Vector const& inp) -> celeritas::Array<double, 3> {
        return celeritas::Array<double, 3>{inp.x(), inp.y(), inp.z()};
    };

    if (!is_equal(track->GetVertexPosition(), pre->GetPosition()))
    {
        // This is a hack to have a valid post-step point.
        // Ideally we would do track->GetCurrentStepNumber() > 0, but this
        // information is not available in Celeritas
        auto* post = step->GetPostStepPoint();
        CELER_ASSERT(post);
        auto const pre_dir = to_array(pre->GetMomentumDirection());
        auto const post_dir = to_array(post->GetMomentumDirection());
        SD_1D_FILL(costheta, celeritas::dot_product(pre_dir, post_dir))
    }

    return true;

#undef SD_1D_FILL
#undef SD_2D_FILL
#undef SD_1D_FILL_WEIGHT
}

//---------------------------------------------------------------------------//
/*!
 * Only process PDGs that are listed the \c SetupOptions::offload_particles .
 *
 * If the list is empty, it defaults to the Celeritas basic EM list.
 */
bool SensitiveDetector::is_pdg_valid(PDG id) const
{
    return std::any_of(this->valid_pdgs_.begin(),
                       this->valid_pdgs_.end(),
                       [&id](PDG this_pdg) { return id == this_pdg; });
};
