#include "boltzmann_structs.h"

#include "update_rxn_likelihoods.h"

#include "ce_approximate_delta_concs.h"

int ce_approximate_delta_concs(struct state_struct *state, double *counts,
			       double *forward_rxn_likelihoods,
			       double *reverse_rxn_likelihoods, 
			       double *flux, double multiplier,
			       int base_rxn,
			       int choice) {
  /*
    Compute approximations to delta concentrations wrt to time for 
    coupled_enzyme.dat reaction file , explicitly to match the 
    coupledenzymefunc.m matlab function:
    For testing ode23tb.
    Called by: ode23tb, ode_num_jac, ode_it_solve
    Calls:     update_rxn_likelihoods

                                TMF
    state                       *SI   Boltzmant state structure.
                                      uses number_reactions,
				           unique_moleules,
                                           molecules_matrix,
					   and lfp,
				      

    counts			D1I   molecule counts vector of length 
                                      nunique_moleucles
    forward_rxn_likelihoods     D1W   scratch vector of length number_reactions
    reverse_rxn_likelihoods     D1W   scratch vector of length number_reactions

    flux                        D1O   vector of length  unique_molecules
                                      of concentration change per unit time.
				      Set by this routine.

    multiplier                  D0I   forward rate constant for base
                                      reaction multplied by base reaction
				      reactant concentration prodeuct.
				      
    base_rxn                    I0I   Base reaction number (usually 0)
    choice                      IOI   Not used by this routine.         

    Note that multiplier is K_f(base_rxn_reaction)*(product of reactant 
    concentrations in base reaction).
	    molecule = (struct molecule_struct *)&sorted_molecules[si];
  */
  struct molecule_struct *molecules;
  struct molecule_struct *molecule;
  struct molecules_matrix_struct *molecules_matrix;
  struct rxn_matrix_struct *rxn_matrix;
  double k1a,k1_a, k2a,k2_a,k3a,k3_a,k1b,k1_b,k2b,k2_b,k3b,k3_b;
  double frb;
  double lrb;
  double recip_frb;
  double forward;
  double backward;
  double *product_term;
  double *reactant_term;
  double *count_to_conc;
  double y[9];
  double  pt;
  double  rt;
  int64_t *molecules_ptrs;
  int64_t *rxn_indices;
  int64_t *coefficients;
  int64_t *rxn_ptrs;
  int64_t *molecule_indices;
  int64_t *rcoefficients;
  int num_species;
  int num_rxns;
  int rxn;

  int i;
  int j;

  int mi;
  int success;

  FILE *lfp;
  FILE *efp;
  /*
#define DBG 1
  */
  /*
    Check that base_rxn is in range.
  */
  success = 1;
  num_rxns = state->number_reactions;
  num_species = state->nunique_molecules;
  molecules   = state->sorted_molecules;
  molecules_matrix = state->molecules_matrix;
  molecules_ptrs   = molecules_matrix->molecules_ptrs;
  rxn_indices      = molecules_matrix->rxn_indices;
  coefficients     = molecules_matrix->coefficients;
  rxn_matrix       = state->reactions_matrix;
  rxn_ptrs         = rxn_matrix->rxn_ptrs;
  molecule_indices = rxn_matrix->molecules_indices;
  rcoefficients    = rxn_matrix->coefficients;
  product_term     = state->product_term;
  reactant_term    = state->reactant_term;

  if (success) {
    success = update_rxn_likelihoods(state,counts,forward_rxn_likelihoods,
				     reverse_rxn_likelihoods);
  }
  count_to_conc = state->count_to_conc;
  for (i=0;i<9;i++) {
    y[i] = counts[i] * count_to_conc[i];
  }
  k1a = 1.0e9;
  k1_a = 1000.0;
  k2a  = 1000.0;
  k2_a = 5000.0;
  k3a  = 1000.0;
  k3_a = 1.0e9;
  k1b  = 1.0e9;
  k1_b = 1000.0;
  k2b  = 1000.0;
  k2_b = 2.0;
  k3b  = 1000.0;
  k3_b = 1.0e9;
  /*
    y[0] =  sa      (A)
    y[1] =  esa     (AE1)
    y[2] =  sb = pa (B)
    y[3] =  epa     (BE1)
    y[4] =  esb     (BE2)
    y[5] =  pb      (C)
    y[6] =  epb     (CE2)
    y[7] =  ea      (E1)
    y[8] =  eb      (E2)


    From Bill's coupledenzymefunc.m
    #d sa/dt = -k1a*sa*ea + k1_a*esa;
    d sa/dt = 0.0
    flux[0] = 0.0;

    d esa/dt = k1a *ea*sa - k1_a*esa - k2a *esa + k2_a *epa; % species esa/AE1
               ---------------------
    flux[1] = (k1a * y[0] * y[7]) - (k1_a * y[1]) - (k2a*y[1]) + (k2_a*y[3]);
              -----------------------------------


    d epa/dt = k2a*esa - k2_a *epa - k3a*epa + k3_a*ea*pa;   % species epa/BE1
    flux[3] = (k2a * y[1]) - (k2_a * y[3]) - (k3a * y[3]) + (k3_a * y[7] * y[2]);
  

    d pa/dt  = k3a*epa - k3_a *ea*pa - k1b*sb*eb + k1_b*esb; % species pa/B
    flux[2] = (k3a * y[3]) - (k3_a * y[2] * y[7]) - (k1b * y[2] * y[8]) + (k1_b * y[4]);

    d ea/dt  = -k1a*sa*ea + k1_a*esa + k3a*epa - k3_a *ea*pa;% species ea/E1
               ---------------------
    flux[7]  = -(k1a*[y[0]*y[7]) + (k1_a * y[1]) + (k3a*y[3]) - (k3_a * y[7] * y[2]);
               ---------------------------------

    d esb/dt = k1b *eb*sb - k1_b*esb - k2b *esb + k2_b *epb; % species esb/BE2
    flux[4] = (k1b * y[8] * y[2]) - (k1_b * y[4])  - (k2b * y[4]) + (k2_b * y[6]);

    d epb/dt = k2b*esb - k2_b *epb - k3b*epb + k3_b*eb*pb;   % species epb/CE2
                                    ----------------------
    flux[6] = (k2b * y[4]) - (k2_b * y[6]) - (k3b * y[6]) + (k3_b * y[5] * y[8]);
                                            ------------------------------------          

    %d pb/dt = k3b*epb - k3_b *eb*pb;
    d pb/dt = 0;                                             % species pb/C
    flux[5] = 0.0;

     d eb/dt = -k1b*sb*eb + k1_b*esb + k3b*epb - k3_b *eb*pb;% species eb/E2 
                                       ----------------------

     flux[8] = - (k1b * y[2] * y[8]) + (k1_b * y[4]) + (k3b * y[6]) - (k3_b * y[8] * y[5]);
                                                  -----------------------------------
  */
  flux[0] = 0.0;
  flux[1] = (k1a * y[0] * y[7]) - (k1_a * y[1]) - (k2a*y[1]) + (k2_a*y[3]);

  flux[2] = (k3a * y[3]) - (k3_a * y[2] * y[7]) - (k1b * y[2] * y[8]) + (k1_b * y[4]);

  flux[3] = (k2a * y[1]) - (k2_a * y[3]) - (k3a * y[3]) + (k3_a * y[7] * y[2]);

  flux[4] = (k1b * y[8] * y[2]) - (k1_b * y[4])  - (k2b * y[4]) + (k2_b * y[6]);
  flux[5] = 0.0;
  flux[6] = (k2b * y[4]) - (k2_b * y[6]) - (k3b * y[6]) + (k3_b * y[5] * y[8]);

  flux[7]  = -(k1a * y[0] * y[7]) + (k1_a * y[1]) + (k3a * y[3]) - (k3_a * y[7] * y[2]);
  flux[8] = - (k1b * y[2] * y[8]) + (k1_b * y[4]) + (k3b * y[6]) - (k3_b * y[8] * y[5]);
  return (success);
}