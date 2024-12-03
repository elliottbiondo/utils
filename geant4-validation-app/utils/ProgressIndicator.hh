//----------------------------------*-C++-*----------------------------------//
//! \brief Loop progress indicator class
//---------------------------------------------------------------------------//
#pragma once

#include <iostream>

//---------------------------------------------------------------------------//
/*!
 * Progress indicator class. It prints the percentage of the current loop given
 * the number of entries to be looped over.
 *
 * \code
 * auto const num_entries = tree->GetEntries();
 * ProgressIndicator progress(num_entries);
 *
 * for (auto i = 0; i < num_entries; i++)
 * {
 *     progress();
 *     tree->GetEntry(i);
 *     // Do stuff
 * }
 * \endcode
 */
class ProgressIndicator
{
  public:
    //! Construct with number of entries that will be processed
    ProgressIndicator(unsigned long number_of_entries);

    //! Default destructor
    ~ProgressIndicator() = default;

    //! Increment counter and update printed percent if needed
    inline void operator()();

  private:
    unsigned long num_entries_;             // Number of loop entries
    unsigned long num_events_per_print_{0}; // Per-print counter
    unsigned long num_printed_msgs_{0};     // Printed messages counter
    unsigned long num_operator_calls_{0};   // Number of operator() calls
    unsigned long events_per_print_;        // Events per increment
    double        percent_increment_; // Print msg at defined increment [%]
};

//---------------------------------------------------------------------------//
/*!
 * Construct with number of entries.
 */
inline ProgressIndicator::ProgressIndicator(unsigned long number_of_entries)
    : num_entries_(number_of_entries)
{
    percent_increment_ = std::ceil(double{100} / num_entries_);
    events_per_print_  = (percent_increment_ == 1)
                             ? percent_increment_ * num_entries_ / 100
                             : 1;
}

//---------------------------------------------------------------------------//
/*!
 * Call at every new entry.
 * Counters and printed message are updated accordingly.
 */
inline void ProgressIndicator::operator()()
{
    if (num_operator_calls_ == num_entries_)
    {
        // Operator() called beyond 100%
        return;
    }

    num_operator_calls_++;
    num_events_per_print_++;

    if (num_events_per_print_ != events_per_print_)
    {
        // Did not reach threshold to print message yet
        return;
    }

    // Reached the number of events needed for each new print
    num_printed_msgs_++;
    double const percent = num_printed_msgs_ * percent_increment_;

    // Print message
    std::cout << "\rProcessing: " << percent << "%";
    if (percent == 100 || num_operator_calls_ == num_entries_)
    {
        // Reached 100%; Add a line break
        std::cout << std::endl;
    }
    std::cout.flush();

    // Reset counter
    num_events_per_print_ = 0;
}
