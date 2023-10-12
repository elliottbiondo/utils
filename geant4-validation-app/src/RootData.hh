//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RootData.hh
//! \brief Data structures for the ROOT output file.
//---------------------------------------------------------------------------//
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

namespace rootdata
{
//---------------------------------------------------------------------------//
/*!
 * Array 3 type.
 */
struct Array3
{
    double x;
    double y;
    double z;

    double operator[](int i)
    {
        switch (i)
        {
            case 0:
                return this->x;
                break;
            case 1:
                return this->y;
                break;
            case 2:
                return this->z;
                break;
            default:
                __builtin_unreachable();
        }
    }
};

//---------------------------------------------------------------------------//
/*!
 * Geant4 processes and Celeritas actions.
 */
enum class ProcessId
{
    transportation,
    ion_ioni,
    msc,
    h_ioni,
    h_brems,
    h_pair_prod,
    coulomb_scat,
    e_ioni,
    e_brems,
    photoelectric,
    compton,
    conversion,
    rayleigh,
    annihilation,
    mu_ioni,
    mu_brems,
    mu_pair_prod,
    // Celeritas actions
    pre_step,
    msc_range,
    eloss_range,
    physics_discrete_select,
    physics_integral_rejected,
    physics_failure,
    along_step_general_linear,
    extend_from_primaries,
    initialize_tracks,
    along_step_neutral,
    geo_propagation_limit,
    kill_looping,
    geo_boundary,
    extend_from_secondaries,
    action_diagnostic,
    step_diagnostic,
    step_gather_pre,
    step_gather_post,
    not_mapped
};

//---------------------------------------------------------------------------//
/*!
 * Sensitive detector scoring. Sensitive detector names and their respective
 * IDs are stored in a separate TTree to reduce file size.
 *
 * The \c sens_det_id.copy_number is the physical volume copy number, while
 * \c sens_det_id.event_sd_index is the index of said sensitive detector in
 * \c event.sensitive_detectors[idx] .
 */
struct SensDetScoreData
{
    //!@{
    //! Type aliases
    using SDProcessMapUL = std::map<ProcessId, std::size_t>;
    using SDProcessMapD  = std::map<ProcessId, double>;
    //!@}

    SDProcessMapUL process_counter; //!< Count process interactions
    SDProcessMapD  process_edep;    //!< Tally process energy deposition [MeV]
    double         energy_deposition; //!< [MeV]
    std::size_t    number_of_steps;

    // Helper function to add data to process maps
    template<typename SDMap, typename type>
    static void map_adder(SDMap& map, ProcessId pid, type data)
    {
        auto& iter = *map.find(pid);
        if (iter != *map.end())
        {
            // Process already inserted; increment counter
            iter.second += data;
        }

        else
        {
            // Process not yet in map; insert it with its first count
            map.insert({pid, data});
        }
    }
};

struct SensDetGdml
{
    std::string  name;        //!< Sensitive detector name
    unsigned int copy_number; //!< Physical volume copy number
};

inline bool operator<(const SensDetGdml& lhs, const SensDetGdml& rhs)
{
    return std::make_tuple(lhs.name, lhs.copy_number)
           < std::make_tuple(rhs.name, rhs.copy_number);
}

//---------------------------------------------------------------------------//
/*!
 * Event data definition. An event includes step, track, and sensitive detector
 * scoring information.
 */
struct Step
{
    ProcessId process_id{ProcessId::not_mapped};
    double    kinetic_energy{0};  //!< [MeV]
    double    energy_loss{0};     //!< [MeV]
    double    length{0};          //!< [cm]
    Array3    direction{0, 0, 0}; //!< Unit vector
    Array3    position{0, 0, 0};  //!< [cm]
    double    global_time{0};     //!< [s]
};

struct Track
{
    int               pdg{0};
    int               id{-1};
    unsigned long     parent_id{0};
    double            length{0};                 //!< [cm]
    double            energy_dep{0};             //!< [MeV]
    double            vertex_energy{0};          //!< [MeV]
    double            vertex_global_time{0};     //!< [s]
    Array3            vertex_direction{0, 0, 0}; //!< Unit vector
    Array3            vertex_position{0, 0, 0};  //!< [cm]
    std::size_t       number_of_steps{0};
    std::vector<Step> steps;
};

struct Event
{
    std::size_t                   id;
    std::vector<Track>            primaries;
    std::vector<Track>            secondaries;
    std::vector<SensDetScoreData> sensitive_detectors;
};

//---------------------------------------------------------------------------//
/*!
 * Performance metrics. Time units must be provided in [s] when using print().
 */
struct ExecutionTime
{
    double wall_total{0};
    double cpu_total{0};
    double wall_sim_run{0};
    double cpu_sim_run{0};

    void print()
    {
        using std::cout;
        using std::endl;

        double init_time = this->cpu_total - this->cpu_sim_run;

        cout << endl;
        cout << std::fixed << std::scientific;
        cout << "| Performance metric | Time [s]     |" << endl;
        cout << "| ------------------ | ------------ |" << endl;
        cout << "| Wall total         | " << this->wall_total << " |" << endl;
        cout << "| CPU total          | " << this->cpu_total << " |" << endl;
        cout << "| Initialization     | " << init_time << " |" << endl;
        cout << endl;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Store max values. Especially useful to simplify histogram definitions during
 * the analysis.
 */
struct DataLimits
{
    std::size_t max_num_primaries{0};
    std::size_t max_primary_num_steps{0};
    std::size_t max_secondary_num_steps{0};
    std::size_t max_num_secondaries{0};
    std::size_t max_steps_per_event{0};

    double max_primary_energy{0};
    double max_secondary_energy{0};

    double max_time{0};
    double max_length{0};
    double max_trk_length{0};

    double      max_sd_energy{0};
    std::size_t max_sd_num_steps{0};

    Array3 min_vertex{0, 0, 0};
    Array3 max_vertex{0, 0, 0};
};

//---------------------------------------------------------------------------//
// Free functions
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Map Geant4 string names and our enums.
 */
const std::map<std::string, ProcessId> process_map = {
    // clang-format off
    {"Transportation",            ProcessId::transportation},
    {"ionIoni",                   ProcessId::ion_ioni},
    {"msc",                       ProcessId::msc},
    {"hIoni",                     ProcessId::h_ioni},
    {"hBrems",                    ProcessId::h_brems},
    {"hPairProd",                 ProcessId::h_pair_prod},
    {"CoulombScat",               ProcessId::coulomb_scat},
    {"eIoni",                     ProcessId::e_ioni},
    {"eBrem",                     ProcessId::e_brems},
    {"phot",                      ProcessId::photoelectric},
    {"compt",                     ProcessId::compton},
    {"conv",                      ProcessId::conversion},
    {"Rayl",                      ProcessId::rayleigh},
    {"annihil",                   ProcessId::annihilation},
    {"muIoni",                    ProcessId::mu_ioni},
    {"muBrems",                   ProcessId::mu_brems},
    {"muPairProd",                ProcessId::mu_pair_prod},
    // Celeritas actions
    {"pre-step",                  ProcessId::pre_step},
    {"msc-range",                 ProcessId::msc_range},
    {"eloss-range",               ProcessId::eloss_range},
    {"physics-discrete-select",   ProcessId::physics_discrete_select},
    {"physics-integral-rejected", ProcessId::physics_integral_rejected},
    {"physics-failure",           ProcessId::physics_failure},
    {"along-step-general-linear", ProcessId::along_step_general_linear},
    {"extend-from-primaries",     ProcessId::extend_from_primaries},
    {"initialize-tracks",         ProcessId::initialize_tracks},
    {"along-step-neutral",        ProcessId::along_step_neutral},
    {"geo-propagation-limit",     ProcessId::geo_propagation_limit},
    {"kill-looping",              ProcessId::kill_looping},
    {"geo-boundary",              ProcessId::geo_boundary},
    {"extend-from-secondaries",   ProcessId::extend_from_secondaries},
    {"action-diagnostic",         ProcessId::action_diagnostic},
    {"step-diagnostic",           ProcessId::step_diagnostic},
    {"step-gather-pre",           ProcessId::step_gather_pre},
    {"step-gather-post",          ProcessId::step_gather_post},
    {"not_mapped",                ProcessId::not_mapped}
    // clang-format on
};

//---------------------------------------------------------------------------//
/*!
 * Safely retrieve the correct process enum from a given string.
 */
static ProcessId to_process_name_id(const std::string& process_name)
{
    auto iter = process_map.find(process_name);
    if (iter == process_map.end())
    {
        return ProcessId::not_mapped;
    }
    return iter->second;
}

//---------------------------------------------------------------------------//
/*!
 * Safely retrieve the correct process string from a given enum.
 */
static std::string to_process_name(ProcessId process_name_id)
{
    auto result = std::find_if(process_map.begin(),
                               process_map.end(),
                               [process_name_id](const auto& process_map) {
                                   return process_map.second == process_name_id;
                               });

    if (result == process_map.end())
    {
        return "not_mapped";
    }

    return result->first;
}

//---------------------------------------------------------------------------//
} // namespace rootdata
