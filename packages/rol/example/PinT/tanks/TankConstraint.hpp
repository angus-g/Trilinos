// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#pragma once
#ifndef TANKCONSTRAINT_HPP
#define TANKCONSTRAINT_HPP

#include "Teuchos_Array.hpp"
#include "Teuchos_ParameterList.hpp"
#include "ROL_Constraint_TimeSimOpt.hpp"
#include "ROL_StdVector.hpp"

#include "TankState.hpp"

#include <utility>

/** \class TankConstraint
    \brief Compute time-step for the coupled tank network

*/

namespace details {

using namespace std;


template<typename Real>
class TankConstraint : public ROL::Constraint_TimeSimOpt<Real> {

private:

  using Vector    = ROL::Vector<Real>;
  using StdVector = ROL::StdVector<Real>;

  TankState<Real> tankState_;

  // Tank Array Numbers
  Real Qin00_;           // Corner inflow 

  vector<Real>& getVector( Vector& x ) const {
    return *( dynamic_cast<StdVector&>(x).getVector() );
  }

  const vector<Real>& getVector( const Vector& x ) const {
    return *( dynamic_cast<const StdVector&>(x).getVector() );
  }

public:

  TankConstraint( Teuchos::ParameterList& pl ) :
    tankState_( pl),
    Qin00_( pl.get( "Corner Inflow", 100.0 ) ) {

  } // end Constructor


  void value(       Vector &c,     const Vector &u_old,
              const Vector &u_new, const Vector &z,
              Real &tol) override {

    tankState_.value( getVector(c),     getVector(u_old), 
                      getVector(u_new), getVector(z) );
    
  }

  void solve(Vector &c,     const Vector &u_old, 
             Vector &u_new, const Vector &z, Real &tol) override {

    auto c_v  = getVector(c);
    auto uo_v = getVector(u_old);
    auto un_v = getVector(u_new);
    auto z_v  = getVector(z);
 
    // Solve the for the new level
    tankState_.solve_level( uo_v, un_v, z_v );

    // Compute the in/out flows
    tankState_.compute_flow( un_v, z_v );

    value(c,u_old,u_new,z);
  }

  void applyJacobian_1_old(Vector &jv,
                           const Vector &v_old,
                           const Vector &u_old, const Vector &u_new,
                           const Vector &z,
                           Real &tol) {
     tankState_.value( getVector(jv), getVector(v_old), 
                       getVector(u_new), getVector(z) );
  }


}; // class TankConstraint

} // namespace details

using details::TankConstraint;

#endif // TANKCONSTRAINT_HPP
