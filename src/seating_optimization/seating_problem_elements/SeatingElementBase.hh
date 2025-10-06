/*
    Seating Optimization Masala Plugins 
    Copyright (C) 2025 Vikram K. Mulligan

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/// @file src/seating_optimization/seating_problem_elements/SeatingElementBase.hh
/// @brief Headers for a SeatingElementBase.
/// @details The SeatingElementBase base class is a common base class for all seating elements.  It is
/// not meant to be instantiated outside of the API definition system.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).

#ifndef Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_SeatingElementBase_hh
#define Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_SeatingElementBase_hh

// Forward declarations:
#include <seating_optimization/seating_problem_elements/SeatingElementBase.fwd.hh>

// Parent header:
#include <base/managers/plugin_module/MasalaPlugin.hh>

// Base headers:
#include <base/types.hh>

// STL headers:
#include <mutex>
#include <string>

namespace seating_optimization_masala_plugins {
namespace seating_optimization {
namespace seating_problem_elements {

/// @brief A SeatingElementBase.
/// @details The SeatingElementBase base class is a common base class for all seating elements.  It is
/// not meant to be instantiated outside of the API definition system.
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
class SeatingElementBase : public masala::base::managers::plugin_module::MasalaPlugin {

	typedef masala::base::managers::plugin_module::MasalaPlugin Parent;
	typedef masala::base::managers::plugin_module::MasalaPluginSP ParentSP;
	typedef masala::base::managers::plugin_module::MasalaPluginCSP ParentCSP;

public:

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION
////////////////////////////////////////////////////////////////////////////////

	/// @brief Default constructor.
	SeatingElementBase() = default;

	/// @brief Copy constructor.  Explicit due to mutex.
	SeatingElementBase( SeatingElementBase const & src );

	/// @brief Assignment operator.  Explicit due to mutex.
	SeatingElementBase & operator=( SeatingElementBase const & src );

	/// @brief Destructor.
	~SeatingElementBase() override = default;

	/// @brief Make a copy of this object.
	virtual
	seating_optimization_masala_plugins::seating_optimization::seating_problem_elements::SeatingElementBaseSP
	clone() const;

	/// @brief Make a fully independent copy of this object.
	SeatingElementBaseSP
	deep_clone() const;

	/// @brief Make this object fully independent by deep-cloning all of its sub-objects.
	void make_independent();

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Get the category or categories for this plugin class.  Default for all
	/// optimization problems; may be overridden by derived classes.
	/// @returns { { "OptimizationSolution", "CostFunctionNetworkOptimizationSolution", "SeatingElementBase" } }
	/// @note Categories are hierarchical (e.g. Selector->AtomSelector->AnnotatedRegionSelector,
	/// stored as { {"Selector", "AtomSelector", "AnnotatedRegionSelector"} }). A plugin can be
	/// in more than one hierarchical category (in which case there would be more than one
	/// entry in the outer vector), but must be in at least one.  The first one is used as
	/// the primary key.
	std::vector< std::vector< std::string > >
	get_categories() const override;

	/// @brief Get the keywords for this plugin class.  Default for all
	/// optimization solutions; may be overridden by derived classes.
	/// @returns { "optimization_solution", "cost_function_network_optimization_solution", "qubo_solution", "numeric" }
	std::vector< std::string >
	get_keywords() const override;

	/// @brief Get the name of this class.
	/// @returns "SeatingElementBase".
	std::string
	class_name() const override;

	/// @brief Get the namespace for this class.
	/// @returns "seating_optimization_masala_plugins::seating_optimization::seating_problem_elements".
	std::string
	class_namespace() const override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE DEFINITION
////////////////////////////////////////////////////////////////////////////////

    /// @brief Get a description of the API for the CostFunctionNetworkOptimizationSolution class.
    masala::base::api::MasalaObjectAPIDefinitionCWP
    get_api_definition() override;

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC GETTERS
////////////////////////////////////////////////////////////////////////////////


public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC SETTERS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Configure this object from a line in a problem definition file.
	/// @details Must be implemented by derived classes.  Base class implementation throws.
	virtual void configure_from_input_line( std::string const & file_line );

public:

////////////////////////////////////////////////////////////////////////////////
// PUBLIC WORK FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

protected:

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

	/// @brief Deried classes can lock the mutex.
	std::mutex & mutex() const { return mutex_; }

	/// @brief Derived classes can access the API definition.
	masala::base::api::MasalaObjectAPIDefinitionCSP &
	api_definition() {
		return api_definition_;
	}

	/// @brief Make this object fully indepdendent.  Derived classes must override this, and the override must call
	/// the parent class implementation.
	virtual void protected_make_independent();

	/// @brief Assign src to this object.  Derived classes must override this, and the override must call
	/// the parent class implementation.
	virtual void protected_assign( SeatingElementBase const & src );

private:

////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
////////////////////////////////////////////////////////////////////////////////

	/// @brief A mutex for this object and its children.
	mutable std::mutex mutex_;

	/// @brief The API definition for this object.s
	masala::base::api::MasalaObjectAPIDefinitionCSP api_definition_;

}; // class SeatingElementBase

} // namespace seating_problem_elements
} // namespace seating_optimization
} // namespace seating_optimization_masala_plugins

#endif // Seating_Optimization_Masala_Plugins_src_seating_optimization_seating_problem_elements_SeatingElementBase_hh