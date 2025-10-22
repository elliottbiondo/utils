//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/RootDataStore.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <TH1D.h>
#include <TH2D.h>
#include <corecel/Assert.hh>

#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Data storage container for sensitive detectors.
 *
 * This struct contains every object used by the ROOT I/O and it is constructed
 * for every sensitive detector found in the \c G4PhysicalVolumeStore .
 */
struct SensDetData
{
    std::string sd_name;  //!< Sensitive detector name

    //!@{
    //! ROOT histograms
    TH1D energy_dep_x;  //!< Step energy deposition along x-axis
    TH1D energy_dep_y;  //!< Step energy deposition along y-axis
    TH1D energy_dep_z;  //!< Step energy deposition along z-axis
    TH1D total_energy_dep;  //!< Total energy deposited in this SD
    TH1D step_len;  //!< Step length
    TH2D pos_xy;  //!< Pre-step position in (x, y) plane
    TH1D time;  //!< Pre-step global time
    TH1D costheta;  //!< Pre/post step direction dot product
    //!@}

    //!@{
    //! User-defined data
    // Accumulated at every step, used at ::EndOfEventAction to fill histogram
    double total_edep{};
    //!@}

    //! Initialize histograms using the SD name and JSON input data
    static SensDetData Initialize(std::string sd_name)
    {
        // Local helper struct for initializing histograms
        struct HistDef
        {
            size_t nbins;
            double min;
            double max;
        };

        // Populate HistDef object from JSON (used by 1D and 2D histograms)
        auto from_json
            = [](nlohmann::json const& j, std::string name) -> HistDef {
            JsonReader::ValidateHistogram(j, name.c_str());
            HistDef hist_def;
            auto const& jh = j.at(name);
            hist_def.nbins = jh.at("num_bins").get<size_t>();
            hist_def.min = jh.at("min").get<double>();
            hist_def.max = jh.at("max").get<double>();
            return hist_def;
        };

        JsonReader::Validate(JsonReader::Instance(), "histograms");
        auto const& json_hist = JsonReader::Instance().at("histograms");

#define SDD_INIT_TH1D(HIST)                                                  \
    {                                                                        \
        std::string htitle = sd_name + "_" + #HIST;                          \
        auto const hd = from_json(json_hist, #HIST);                         \
        result.HIST = TH1D(#HIST, htitle.c_str(), hd.nbins, hd.min, hd.max); \
        result.HIST.AddDirectory(false);                                     \
    }
#define SDD_INIT_TH2D(HIST)                                   \
    {                                                         \
        std::string htitle = sd_name + "_" + #HIST;           \
        auto const hdx = from_json(json_hist.at(#HIST), "x"); \
        auto const hdy = from_json(json_hist.at(#HIST), "y"); \
        result.HIST = TH2D(#HIST,                             \
                           htitle.c_str(),                    \
                           hdx.nbins,                         \
                           hdx.min,                           \
                           hdx.max,                           \
                           hdy.nbins,                         \
                           hdy.min,                           \
                           hdy.max);                          \
        result.HIST.AddDirectory(false);                      \
    }

        //// Initialie histograms ////
        SensDetData result;
        result.sd_name = sd_name;
        SDD_INIT_TH1D(energy_dep_x)
        SDD_INIT_TH1D(energy_dep_y)
        SDD_INIT_TH1D(energy_dep_z)
        SDD_INIT_TH1D(total_energy_dep)
        SDD_INIT_TH1D(step_len)
        SDD_INIT_TH2D(pos_xy)
        SDD_INIT_TH1D(time)
        SDD_INIT_TH1D(costheta)
        return result;

#undef SDD_INIT_TH1D
#undef SDD_INIT_TH2D
    }
};

//---------------------------------------------------------------------------//
/*!
 * Helper struct for indexing physical volumes to an object.
 * (e.g. std::map<SensDetId, SensDetData> map)
 */
struct SensDetId
{
    size_t physvol_id;
    size_t copy_number;
};

//---------------------------------------------------------------------------//
//! Overload \c operator< for \c map.find(sensdetid) compatibility.
inline bool operator<(SensDetId const& lhs, SensDetId const& rhs)
{
    return std::make_tuple(lhs.physvol_id, lhs.copy_number)
           < std::make_tuple(rhs.physvol_id, rhs.copy_number);
}

//---------------------------------------------------------------------------//
/*!
 * ROOT I/O data storage manager class.
 *
 * This class stores a \c SensDetData for every sensitive detector in
 * the geometry and allows an easy way to access them using the physical volume
 * instance ID and copy number.
 */
class RootDataStore
{
  public:
    //!@{
    //! \name Type aliases
    using PhysVolId = size_t;
    using CopyNumber = size_t;
    //!@}

    //! Construct empty
    RootDataStore() = default;

    //! Map and initialize histograms for a sensitive detector
    void InsertSensDet(PhysVolId pv_id, CopyNumber copy_num, std::string name);

    //! Get histogram data for a given physical volume ID and copy number
    SensDetData& Find(PhysVolId pv_id, CopyNumber copy_num);

    //! Access full SD map
    std::map<SensDetId, SensDetData>& Map() { return sensdet_map_; }

  private:
    std::map<SensDetId, SensDetData> sensdet_map_;
};
