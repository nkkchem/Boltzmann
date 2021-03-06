#include "boltzmann_structs.h"
#include "boltzmann_cvodes_headers.h"
#include "cvodes_params_struct.h"
#include "get_counts.h"
#include "conc_to_pow.h"
#include "update_regulations.h"
#include "lr9_gradient.h"

int lr9_gradient(struct state_struct *state, 
		 double *concs,
		 double *flux, 
		 int choice) {
  /*
    Compute approximations to concentration changes wrt time, 
    Based on read in rate constants and molecule counts
    using concs to compute tr, tp, pt, rt instead of counts.

    Get reference from Bill Cannon
    Called by: gradient
    Calls:     get_counts,
               update_regulations,

                                TMF
    state                       *SI   Boltzmant state structure.
                                      uses number_reactions,
				           unique_moleules,
					   sorted_molecules,
					   sorted_compartments,
                                           molecules_matrix,
					   ke, rke,
					   product_term as scratch.
					   and lfp,
				      

    concs			D1I   molecule concentrations vector of length 
                                      nunique_moleucles

    flux                        D1O   vector of length  unique_molecules
                                      of concentration change per unit time.
				      Set by this routine.

    choice                      IOI   Not used by this routine.

  */
  struct  cvodes_params_struct *cvodes_params;
  struct  molecule_struct *molecules;
  struct  molecule_struct *molecule;
  struct  compartment_struct *compartments;
  struct  compartment_struct *compartment;
  struct  molecules_matrix_struct *molecules_matrix;
  struct  reactions_matrix_struct *rxn_matrix;
  double  *activities;
  double  *forward_lklhd;
  double  *reverse_lklhd;
  double  *forward_rc;
  double  *reverse_rc;
  double  *rfc;
  double  *ke;
  double  *rke;
  double  *rcoefficients;
  double  *coeff_sum;
  double  *kq;
  double  *kqi;
  double  *skq;
  double  *skqi;
  int64_t *molecules_ptrs;
  int64_t *rxn_indices;
  int64_t *rxn_ptrs;
  int64_t *molecule_indices;
  /*
  double  *counts;
  double  *conc_to_count;
  */
  double  flux_scaling;
  double  pt;
  double  rt;
  double  tr;
  double  tp;
  double  conc_mi;
  double  recip_volume;
  double  volume;
  double  multiplier;
  double  keq_adj;
  double  rkeq_adj;
  /*
  double  thermo_adj;
  double  recip_avogadro;
  */
  double  fluxi;
  double  klim;
  double  factorial;
  double  *coefficients;


  int num_species;
  int num_rxns;

  int rxn;
  int success;

  int i;
  int j;

  int mi;
  int ci;


  int use_regulation;
  int count_or_conc;

  int compute_sensitivities;
  int ode_solver_choice;

  int padi;


  FILE *lfp;
  FILE *efp;
  /*
#define DBG 1
  */
  /*
    Check that base_rxn is in range.
  */
  success          = 1;
  num_rxns         = state->number_reactions;
  num_species      = state->nunique_molecules;
  molecules        = state->sorted_molecules;
  compartments     = state->sorted_compartments;
  activities       = state->activities;
  forward_lklhd    = state->ode_forward_lklhds;
  reverse_lklhd    = state->ode_reverse_lklhds;
  coeff_sum        = state->coeff_sum;
  molecules_matrix = state->molecules_matrix;
  molecules_ptrs   = molecules_matrix->molecules_ptrs;
  rxn_indices      = molecules_matrix->reaction_indices;
  coefficients     = molecules_matrix->coefficients;
  rxn_matrix       = state->reactions_matrix;
  rxn_ptrs         = rxn_matrix->rxn_ptrs;
  molecule_indices = rxn_matrix->molecules_indices;
  rcoefficients    = rxn_matrix->coefficients;
  forward_rc       = state->forward_rc;
  reverse_rc       = state->reverse_rc;
  ke               = state->ke;
  rke              = state->rke;
  rfc              = state->product_term;
  kq               = state->ode_kq;
  kqi              = state->ode_kqi;
  skq              = state->ode_skq;
  skqi             = state->ode_skqi;
  /*
  counts           = state->ode_counts;
  conc_to_count    = state->conc_to_count;
  */
  use_regulation   = state->use_regulation;
  ode_solver_choice = state->ode_solver_choice;
  compute_sensitivities = state->compute_sensitivities;
  factorial           = 0.0;
  if ((ode_solver_choice == 1) && compute_sensitivities) {
    cvodes_params = state->cvodes_params;
    ke = cvodes_params->p;
    rke = cvodes_params->rp;
    for (i=0;i<num_rxns;i++) {
      rke[i] = 1.0/ke[i];
    }
  }
  /*
  recip_avogadro   = state->recip_avogadro;
  */
  /*
  flux_scaling     = compute_flux_scaling(state,concs);
  get_counts(num_species,concs,conc_to_count,counts);
  */
  flux_scaling     = 1.0;
  lfp      = state->lfp;
  /*
    As per discusion with Bill Cannon, we want to update the activities
    if reguation is in play. So do that here.
  */
  if (use_regulation) {
    count_or_conc = 0;
    update_regulations(state,concs,count_or_conc);
  }
  /*
    Compute the reaction flux contributions for each reaction:

    rfc   = k_f * product of reactants^stoichiometric_coef -
            k_r * product of products^stoichiometric_coef,

	 where k_f = forward rate constant

	 and   k_r  = reverse rate constant.

	 and thermo_product = 
	 product( species_conc + |stoichiometric_coef|/volume)^|stoichiometric coef|.

	 Then if a molecule is a  in rxn i,

	 rxn i contributes 1/stoichiometric_coef * rfc[i] to the molcule's flux.
         here we include the sign with the stoichiometric_coef, so for
	 reactants the rfc contribution is subtracted, and for products it
	 is added.
  */
  /*
    First check to see if forward and reverse rate constants have
    been supplied for all reactions, if not fail.
  */
  for (i=0;i<num_rxns;i++) {
    if (forward_rc[i] < 0.0) {
      if (lfp) {
	success = 0;
	fprintf(lfp,"lr9_gradient: Error forward rate constant not supplied for reaction %d\n",i);
	fflush(lfp);
      }
    }
    if (reverse_rc[i] < 0.0) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"lr9_gradient: Error reverse rate constant not supplied for reaction %d\n",i);
	fflush(lfp);
      }
    }
  }
  if (success) {   
    for (i=0;i<num_rxns;i++) {
      pt = 1.0;
      rt = 1.0;
      tr = 1.0;
      tp = 1.0;
      multiplier = 1.0;
      for (j=rxn_ptrs[i];j<rxn_ptrs[i+1];j++) {
	mi = molecule_indices[j];
	molecule = (struct molecule_struct *)&molecules[mi];
	ci = molecules->c_index;
	compartment = (struct compartment_struct *)&compartments[ci];
	volume  = compartment->volume;
	recip_volume = compartment->recip_volume;
	/*
	*/
	klim = coefficients[j];
	conc_mi = concs[mi];
	if (klim < 0.0) {
	  klim = 0.0 - klim;
	  rt = rt * conc_to_pow(conc_mi,klim,factorial);
	  multiplier = multiplier * conc_to_pow(volume,klim,factorial);
	  /*
	  for (k=0;k<(-klim);k++) {
	    rt = rt * conc_mi;
	    multiplier *= volume;
	  }
	  */
	} else {
	  if (klim > 0.0) {
	    pt = pt * conc_to_pow(conc_mi,klim,factorial);
	    multiplier = multiplier * conc_to_pow(recip_volume,klim,
						  factorial);
	    /*
	    for (k=0;k<klim;k++) {
	      pt = pt * conc_mi;
	      multiplier *= recip_volume;
	    }
	    */
	  }
	}
      } /* end for (j ... ) */
      keq_adj = multiplier;
      rkeq_adj = 1.0/keq_adj;
      /*
	Save likelihoods for printing.
      */
      if (pt != 0.0) {
	forward_lklhd[i] = ke[i] * keq_adj * (rt/pt);
      } else {
	if (rt != 0) {
	  forward_lklhd[i] = 1.0;
	} else {
	  forward_lklhd[i] = 0.0;
	}
      }
      if (rt != 0.0) {  
	reverse_lklhd[i] = rke[i] * rkeq_adj * (pt/rt);
      } else {
	if (pt != 0) {
	  reverse_lklhd[i] = 1.0;
	} else {
	  reverse_lklhd[i] = 0.0;
	}
      }
      /*
	rfc[i] = (ke[i] * (rt/tp)) - (rke[i] * (pt/tr));
	NB if use_activities is not set activities[i] will be 1.0 for all i.
      */
      rfc[i] = (forward_rc[i] * rt - reverse_rc[i] * pt) * activities[i];
      kq[i] = forward_rc[i] * rt;
      kqi[i] = reverse_rc[i] * pt;
      skq[i] = kq[i] * activities[i];
      skqi[i] = kqi[i] * activities[i];
    } /* end for (i..._) */
    molecule = molecules;
    for (i=0;i<num_species;i++) {
      fluxi = 0.0;
      if (molecule->variable == 1) {
	for (j=molecules_ptrs[i];j<molecules_ptrs[i+1];j++) {
	  rxn = rxn_indices[j];
	  if (coefficients[j] != 0.0) {
	    fluxi += (rfc[rxn]*coefficients[j]);
	  }
	} /* end for(j...) */
	flux[i] = flux_scaling * fluxi;
      } else {
	flux[i] = 0.0;
      }
      molecule += 1; /* Caution address arithmetic here. */
    } /* end for (i...) */
  } /* end if success */
#ifdef DBG
    if (lfp) {
    fprintf(lfp,"Mol_index\t   conc   \t    flux\n");
    for (i=0;i<num_species;i++) {
      fprintf(lfp,"%d\t%le\t%le\n",
	      i,concs[i],flux[i]);
    }
    fflush(lfp);
  }
#endif
  return (success);
}
