//===========================================================================
//
// File: setupGridAndProps.hpp
//
// Created: Tue Aug 11 14:47:35 2009
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//            B�rd Skaflestad     <bard.skaflestad@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of The Open Reservoir Simulator Project (OpenRS).

  OpenRS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenRS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENRS_SETUPGRIDANDPROPS_HEADER
#define OPENRS_SETUPGRIDANDPROPS_HEADER

#include <opm/core/utility/parameters/ParameterGroup.hpp>
#include <opm/core/utility/Units.hpp>
#include <dune/grid/CpGrid.hpp>
#include <dune/grid/sgrid.hh>
#include <opm/porsol/common/ReservoirPropertyCapillary.hpp>
#include <boost/filesystem.hpp>

namespace Opm
{

    /// Helper for determining whether we should
    template <class RP>
    bool useJ()
    {
        return false;
    }

    template<>
    bool useJ< ReservoirPropertyCapillary<3> >()
    {
        return true;
    }

    /// @brief
    /// @todo Doc me!
    /// @param
    template <template <int> class ResProp>
    inline void setupGridAndProps(const Opm::parameter::ParameterGroup& param,
				  Dune::CpGrid& grid,
				  ResProp<3>& res_prop)
    {
	// Initialize grid and reservoir properties.
	// Parts copied from Dune::CpGrid::init().
	std::string fileformat = param.getDefault<std::string>("fileformat", "cartesian");
	if (fileformat == "sintef_legacy") {
	    std::string grid_prefix = param.get<std::string>("grid_prefix");
	    grid.readSintefLegacyFormat(grid_prefix);
	    MESSAGE("Warning: We do not yet read legacy reservoir properties. Using defaults.");
	    res_prop.init(grid.size(0));
	} else if (fileformat == "eclipse") {
            std::string ecl_file = param.get<std::string>("filename");
	    Opm::EclipseGridParser parser(ecl_file);
	    double z_tolerance = param.getDefault<double>("z_tolerance", 0.0);
	    bool periodic_extension = param.getDefault<bool>("periodic_extension", false);
	    bool turn_normals = param.getDefault<bool>("turn_normals", false);
	    grid.processEclipseFormat(parser, z_tolerance, periodic_extension, turn_normals);
            // Save EGRID file in case we are writing ECL output.
            if (param.getDefault("output_ecl", false)) {
                boost::filesystem::path ecl_path(ecl_file);
                ecl_path.replace_extension(".FEGRID");
                parser.saveEGRID(ecl_path.string());
            }
            double perm_threshold_md = param.getDefault("perm_threshold_md", 0.0);
	    double perm_threshold = Opm::unit::convert::from(perm_threshold_md, Opm::prefix::milli*Opm::unit::darcy);
	    std::string rock_list = param.getDefault<std::string>("rock_list", "no_list");
	    std::string* rl_ptr = (rock_list == "no_list") ? 0 : &rock_list;
            bool use_j = param.getDefault("use_jfunction_scaling", useJ<ResProp<3> >());
            double sigma = 1.0;
            double theta = 0.0;
            if (use_j) {
                sigma = param.getDefault("sigma", sigma);
                theta = param.getDefault("theta", theta);
            }
            if (param.has("viscosity1") || param.has("viscosity2")) {
                double v1 = param.getDefault("viscosity1", 0.001);
                double v2 = param.getDefault("viscosity2", 0.003);
                res_prop.setViscosities(v1, v2);
            }
	    res_prop.init(parser, grid.globalCell(), perm_threshold, rl_ptr,
                          use_j, sigma, theta);
	} else if (fileformat == "cartesian") {
	    Dune::array<int, 3> dims = {{ param.getDefault<int>("nx", 1),
				    param.getDefault<int>("ny", 1),
				    param.getDefault<int>("nz", 1) }};
	    Dune::array<double, 3> cellsz = {{ param.getDefault<double>("dx", 1.0),
					 param.getDefault<double>("dy", 1.0),
					 param.getDefault<double>("dz", 1.0) }};
	    grid.createCartesian(dims, cellsz);
	    double default_poro = param.getDefault("default_poro", 0.2);
	    double default_perm_md = param.getDefault("default_perm_md", 100.0);
	    double default_perm = Opm::unit::convert::from(default_perm_md, Opm::prefix::milli*Opm::unit::darcy);
	    MESSAGE("Warning: For generated cartesian grids, we use uniform reservoir properties.");
	    res_prop.init(grid.size(0), default_poro, default_perm);
	} else {
	    THROW("Unknown file format string: " << fileformat);
	}
	if (param.getDefault("use_unique_boundary_ids", false)) {
	    grid.setUniqueBoundaryIds(true);
	}
    }

    /// @brief
    /// @todo Doc me!
    /// @param
    template <template <int> class ResProp>
    inline void setupGridAndPropsEclipse(const Opm::EclipseGridParser& parser,
                                         double z_tolerance,
                                         bool periodic_extension,
                                         bool turn_normals,
                                         bool clip_z,
                                         bool unique_bids,
                                         double perm_threshold,
                                         const std::string& rock_list,
                                         bool use_jfunction_scaling,
                                         double sigma,
                                         double theta,
                                         Dune::CpGrid& grid,
                                         ResProp<3>& res_prop)
    {
        grid.processEclipseFormat(parser, z_tolerance, periodic_extension, turn_normals, clip_z);
        const std::string* rl_ptr = (rock_list == "no_list") ? 0 : &rock_list;
        res_prop.init(parser, grid.globalCell(), perm_threshold, rl_ptr, use_jfunction_scaling, sigma, theta);
	if (unique_bids) {
	    grid.setUniqueBoundaryIds(true);
	}
    }

    /// @brief
    /// @todo Doc me!
    /// @param
    template <template <int> class ResProp>
    inline void setupGridAndProps(const Opm::parameter::ParameterGroup& param,
				  Dune::SGrid<3, 3>& grid,
				  ResProp<3>& res_prop)
    {
	// Initialize grid and reservoir properties.
	// Parts copied from Dune::CpGrid::init().
	std::string fileformat = param.getDefault<std::string>("fileformat", "cartesian");
	if (fileformat == "cartesian") {
	    Dune::array<int, 3> dims = {{ param.getDefault<int>("nx", 1),
				    param.getDefault<int>("ny", 1),
				    param.getDefault<int>("nz", 1) }};
	    Dune::array<double, 3> cellsz = {{ param.getDefault<double>("dx", 1.0),
					 param.getDefault<double>("dy", 1.0),
					 param.getDefault<double>("dz", 1.0) }};
            grid.~SGrid<3,3>();
            new (&grid) Dune::SGrid<3, 3>(&dims[0], &cellsz[0]);
	    double default_poro = param.getDefault("default_poro", 0.2);
	    double default_perm = param.getDefault("default_perm", 100.0*Opm::prefix::milli*Opm::unit::darcy);
	    MESSAGE("Warning: For generated cartesian grids, we use uniform reservoir properties.");
	    res_prop.init(grid.size(0), default_poro, default_perm);
	} else {
	    THROW("Dune::SGrid can only handle cartesian grids, unsupported file format string: " << fileformat);
	}
    }

} // namespace Opm


#endif // OPENRS_SETUPGRIDANDPROPS_HEADER
