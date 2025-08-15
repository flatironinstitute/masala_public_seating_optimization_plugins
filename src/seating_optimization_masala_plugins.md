\mainpage Seating Optimization Masala Plugins

## Description

The Seating Optimization Masala Plugins library version 0.1 (alpha) contains classes for defining seating optimization problems that can be solved with Masala CFN solvers defined in other libraries.  Masala provides solvers that run classically (_i.e._ on CPU or GPU) or on current-generation quantum computers (_i.e._ on QPU)

## Author

- Vikram K. Mulligan, Ph.D., Research Scientist, Center for Computational Biology, Flatiron Institute (vmulligan@flatironinstitute.org).

## Library organization

This library is divided into hand-written sub-libraries and auto-generated API sub-libraries.  All sub-libraries are in appropriately named sub-directories of the src/ directory; all namespaces start with seating_optimization_masala_plugins, and then follow the directory structure.  The namespaces of the hand-written sub-libraries are:

- \link seating_optimization_masala_plugins::seating_optimization \endlink
- \link seating_optimization_masala_plugins::registration \endlink

The corresponding auto-generated libraries are in the following namespaces:

- \link seating_optimization_masala_plugins::seating_optimization_api \endlink
- \link seating_optimization_masala_plugins::registration_api \endlink

## Using this library as a conventional library linked at compilation time

If you choose the link this library at compilation time, it is _only_ safe to directly call functions or instantiate classes in the auto-generated API namespaces.  These have stable APIs that will be preserved in future versions of the Seating Optimization Masala Plugins library, or deprecated in a manner that provides ample warning.  Handwritten libraries are _not_ part of the API, and can change without warning.

## Using this library as a dynamically-linked library only provided at runtime

As an alternative, you may choose only to link the Masala Core library.  In that case, you can use the \link masala::base::managers::plugin_module::MasalaPluginLibraryManager \endlink to load the Seating Optimization Masala Plugins at runtime, which will trigger registration of all plugin modules in this library that have API definitions with the \link masala::base::managers::plugin_module::MasalaPluginModuleManager \endlink, which your code may interrogate to obtain Masala plugins.  The interface for plugin modules may be accessed through the \link masala::base::api::MasalaObjectAPIDefinition \endlink for each plugin module.
