#pragma once
#ifndef TANKSTATE_IMPL_HPP
#define TANKSTATE_IMPL_HPP

namespace details {

using namespace std;

template<typename Real>
TankState<Real>::TankState( Teuchos::ParameterList& pl ) :
  // ----------- Begin Initializer List ----------------//
  rows_   ( pl.get( "Number of Rows",          3      ) ),
  cols_   ( pl.get( "Number of Columns",       3      ) ),
  Cv_     ( pl.get( "Valve Constant",          1.0e-2 ) ),
  rho_    ( pl.get( "Density of Fluid",        1.0e3  ) ),
  h0_     ( pl.get( "Initial Fluid Level",     2.0    ) ),
  H_      ( pl.get( "Height of Tank",          10.0   ) ),
  A_      ( pl.get( "Cross-sectional Area",    10.0   ) ),
  g_      ( pl.get( "Gravity Constant",        9.8    ) ),
  T_      ( pl.get( "Total Time",              20.0   ) ),
  theta_  ( pl.get( "Theta",                   0.5    ) ),
  Nt_     ( pl.get( "Number of Time Steps",    100    ) ),
  //----------------------------------------------------//
  Ntanks_(rows_*cols_),
  p_(Ntanks_,1.0), 
  coeff1_( Cv_*rho_*g_ ), 
  kappa_( 0.5*coeff1_*dt_/A_ ),
  betaL_( (theta_-1)*dt_/A_ ),
  betaR_( theta_*dt_/A_ ) {
  // ------------- End Initializer List ----------------//

  auto ptrows = Teuchos::getArrayFromStringParameter<int>( pl, "Pass-Through Rows"    );
  auto ptcols = Teuchos::getArrayFromStringParameter<int>( pl, "Pass-Through Columns" );

  vector<Real> w(Ntanks_,1.0);

  vector<size_type> band_index{0, 1, cols_};
  
  for( size_type j=0; j<cols_; ++j ) w[j]       = 0.0;
  for( size_type i=0; i<rows_; ++i ) w[i*cols_] = 0.0;

  for( size_type i=0; i<ptrows.size(); ++i ) {
    size_type k = cols_*ptrows[i]+ptcols[i];
    p_[k] = 0.0;
  }

  vector<Real> band_L0(Ntanks_);       vector<Real> band_R0(Ntanks_);
  vector<Real> band_L1(Ntanks_-1);     vector<Real> band_R1(Ntanks_-1);
  vector<Real> band_Lc(Ntanks_-cols_); vector<Real> band_Rc(Ntanks_-cols_);

  Real alpha_L = (1-theta_)*kappa_;    Real alpha_R = theta_*kappa_;

  band_L0[0] = 1.0-2.0*alpha_L*p_[0];
  band_R0[0] = 1.0-2.0*alpha_R*p_[0];

  for( size_type l=1; l<cols_; ++l ) {
    band_L0[l]   = 1.0-2.0*alpha_L*p_[l];
    band_R0[l]   = 1.0-2.0*alpha_R*p_[l];

    if( l>=1 ) {
      band_L1[l-1] = alpha_L*w[l]*p_[l] * (l%cols_!=0);
      band_R1[l-1] = alpha_R*w[l]*p_[l] * (l%cols_!=0);

      if( l>=cols_ ) {
        band_Lc[l-cols_] = alpha_L*w[l]*p_[l];
        band_Rc[l-cols_] = alpha_R*w[l]*p_[l];
      }
    } 
  } // end for
} // end Constructor

template<typename Real>
void TankState<Real>::value( vector<Real>& c, const vector<Real>* u_old, 
                             const vector<Real>& u_new, const vector<Real>& z ) const {

  for( size_type i=0; i<rows_; ++i ) {
    for( size_type j=0; j<cols_; ++j ) {

      size_type l = cols_*i+j;

      auto h_val    = h(c,i,j);    auto h_new    = h(u_new,i,j);    auto h_old    = h(u_old,i,j);
      auto Qout_val = Qout(c,i,j); auto Qout_new = Qout(u_new,i,j); auto Qout_old = Qout(u_old,i,j);
      auto Qin_val  = Qin(c,i,j);  auto Qin_new  = Qin(u_new,i,j);  auto Qin_old  = Qin(u_old,i,j);

      h_val = h_new - h_old - p_[l]*( betaL_*(Qin_new-Qout_new) +
                                      betaR_*(Qin_old-Qout_old) );

      Qout_val = Qout_new - coeff1_*h_val;

      Qin_val = Qin_new - z[l];

      if( i>0 ) Qin_val -= 0.5*Qout(u_new,i-1,j);
      if( j>0 ) Qin_val -= 0.5*Qout(u_new,i,j-1);

    }
  }  
}



template<typename Real>
void TankState<Real>::compute_flow(       vector<Real>& u, 
                                    const vector<Real>& f ) const {
  for( size_type i=0; i<rows_; ++i ) {
    for( size_type j=0; j<cols_; ++j ) {
      size_type l = cols_*i+j;
      Qout(u,i,j) = coeff1_*h(u,i,j);   
      Qin(u,i,j)  = f(i,j);
      if( i>0 ) Qin(u,i,j) += 0.5*Qout(u,i-1,j);
      if( j>0 ) Qin(u,i,j) += 0.5*Qout(u,i,j-1);
    }
  }
} // compute_flow



template<typename Real>
void TankState<Real>::solve_level(       vector<Real>& c, 
                                         vector<Real>& u_new, 
                                   const vector<Real>& u_old, 
                                   const vector<Real>& f     ) const {
  // c += R*u_new
  R_->apply(c,u_old,1.0,0,Ntanks_);

  for( size_type l=0; l<Ntanks_; ++l )  c[l] += dt_*p_[l]*f[l]/A_;

  L_->solve(u_new,c,1.0,0,Ntanks_);
} // solve_level



} // namespace details


#endif // TANKSTATE_IMPL_HPP
