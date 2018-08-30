// Copyright (c) 2015, Joseph Mirabel
// Authors: Joseph Mirabel (joseph.mirabel@laas.fr)
//
// This file is part of hpp-constraints.
// hpp-constraints is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-constraints is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-constraints. If not, see <http://www.gnu.org/licenses/>.

#include <hpp/constraints/configuration-constraint.hh>

#include <pinocchio/multibody/liegroup/liegroup.hpp>

#include <hpp/util/debug.hh>
#include <hpp/pinocchio/device.hh>
#include <hpp/pinocchio/joint.hh>
#include <hpp/pinocchio/configuration.hh>
#include <hpp/pinocchio/liegroup-element.hh>

namespace hpp {
  namespace constraints {

    ConfigurationConstraintPtr_t ConfigurationConstraint::create (
        const std::string& name, const DevicePtr_t& robot,
        ConfigurationIn_t goal, std::vector <bool> mask)
    {
      vector_t ws (vector_t::Ones(robot->numberDof ()));
      for (std::size_t i = 0; i < mask.size (); ++i) {
        if (!mask[i]) ws[i] = 0;
      }

      ConfigurationConstraint* ptr = new ConfigurationConstraint
        (name, robot, goal, ws);
      return ConfigurationConstraintPtr_t (ptr);
    }

    ConfigurationConstraintPtr_t ConfigurationConstraint::create (
        const std::string& name, const DevicePtr_t& robot,
        ConfigurationIn_t goal, const vector_t& weights)
    {
      ConfigurationConstraint* ptr = new ConfigurationConstraint
        (name, robot, goal, weights);
      return ConfigurationConstraintPtr_t (ptr);
    }

    ConfigurationConstraint::ConfigurationConstraint (
        const std::string& name, const DevicePtr_t& robot,
        ConfigurationIn_t goal, const vector_t& weights) :
      DifferentiableFunction (robot->configSize (), robot->numberDof (),
                              LiegroupSpace::R1 (), name),
      robot_ (robot), weights_ (weights), diff_ (robot->numberDof())
    {
      assert (weights.size() == robot->numberDof());
      LiegroupSpacePtr_t s (LiegroupSpace::createCopy(robot->configSpace()));
      s->mergeVectorSpaces();
      goal_ = LiegroupElement (goal, s);
    }

    void ConfigurationConstraint::impl_compute (LiegroupElement& result,
                                                ConfigurationIn_t argument)
      const throw ()
    {
      using namespace hpp::pinocchio;
      LiegroupConstElementRef a (argument, goal_.space());
      diff_.noalias() = (goal_ - a).cwiseAbs2();
      result.vector () [0] = 0.5 * weights_.dot(diff_);
    }

    void ConfigurationConstraint::impl_jacobian (matrixOut_t jacobian,
        ConfigurationIn_t argument) const throw ()
    {
      using namespace hpp::pinocchio;
      matrix_t unused;

      LiegroupConstElementRef a (argument, goal_.space());
      diff_ = goal_ - a;

      // Apply jacobian of the difference on the right.
      goal_.space()->Jdifference<false> (argument, goal_.vector(), diff_.transpose(), unused);

      jacobian.leftCols (robot_->numberDof ()).noalias() =
        weights_.cwiseProduct(diff_).transpose ();
    }
  } // namespace constraints
} // namespace hpp
