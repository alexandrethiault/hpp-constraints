// Copyright (c) 2015, LAAS-CNRS
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

#ifndef HPP__CONSTRAINTS_SYMBOLIC_FUNCTION_HH
# define HPP__CONSTRAINTS_SYMBOLIC_FUNCTION_HH

# include <boost/assign/list_of.hpp>

# include <hpp/_constraints/fwd.hh>
# include <hpp/constraints/config.hh>
# include <hpp/_constraints/symbolic-calculus.hh>
# include <hpp/_constraints/differentiable-function.hh>

# include <hpp/model/device.hh>

namespace hpp {
  namespace _constraints {
    template <typename Expression>
    class HPP_CONSTRAINTS_DLLAPI SymbolicFunction : public DifferentiableFunction
    {
      public:
        typedef boost::shared_ptr<SymbolicFunction> Ptr_t;
        typedef boost::weak_ptr<SymbolicFunction> WkPtr_t;

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        /// Return a shared pointer to a new instance
        static Ptr_t create (
            const std::string& name, const DevicePtr_t& robot,
            const typename Traits<Expression>::Ptr_t expr)
        {
          std::vector <bool> mask (expr->value ().size (), true);
          return create (name, robot, expr, mask);
        }

        /// Return a shared pointer to a new instance
        static Ptr_t create (
            const std::string& name, const DevicePtr_t& robot,
            const typename Traits<Expression>::Ptr_t expr,
            std::vector <bool> mask)
        {
          assert (mask.size() == expr->value().size());
          Ptr_t ptr (new SymbolicFunction (name,robot,expr,mask));
          ptr->init (ptr);
          return ptr;
        }

        virtual ~SymbolicFunction () throw () {}

        SymbolicFunction (const std::string& name, const DevicePtr_t& robot,
            const typename Traits<Expression>::Ptr_t expr,
            std::vector <bool> mask) :
          DifferentiableFunction (robot->configSize(), robot->numberDof(), expr->value().size(), name),
          robot_ (robot), expr_ (expr), mask_ (mask) {}

      protected:
        /// Compute value of error
        ///
        /// \param argument configuration of the robot,
        /// \retval result error vector
        virtual void impl_compute (vectorOut_t result,
            ConfigurationIn_t argument) const throw ()
        {
          robot_->currentConfiguration (argument);
          robot_->computeForwardKinematics ();
          expr_->invalidate ();
          expr_->computeValue ();
          size_t index = 0;
          for (std::size_t i = 0; i < mask_.size (); i++) {
            if (mask_[i])
              result[index++] = expr_->value ()[i];
          }
        }

        virtual void impl_jacobian (matrixOut_t jacobian,
            ConfigurationIn_t arg) const throw ()
        {
          robot_->currentConfiguration (arg);
          robot_->computeForwardKinematics ();
          expr_->invalidate ();
          expr_->computeJacobian ();
          size_t index = 0;
          for (std::size_t i = 0; i < mask_.size (); i++) {
            if (mask_[i])
              jacobian.row (index++) = expr_->jacobian ().row (i);
          }
        }

        void init (const Ptr_t& self) {
          wkPtr_ = self;
        }
      private:
        WkPtr_t wkPtr_;
        DevicePtr_t robot_;
        typename Traits<Expression>::Ptr_t expr_;
        std::vector <bool> mask_;
    }; // class ComBetweenFeet
  } // namespace _constraints
} // namespace hpp
#endif // HPP__CONSTRAINTS_SYMBOLIC_FUNCTION_HH
