/* alloc2.c
*******************************************************************************
boltzmann

Pacific Northwest National Laboratory, Richland, WA 99352.

Copyright (c) 2010 Battelle Memorial Institute.

Publications based on work performed using the software should include 
the following citation as a reference:


Licensed under the Educational Community License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
The terms and conditions of the License may be found in 
ECL-2.0_LICENSE_TERMS.TXT in the directory containing this file.
        
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
******************************************************************************/
#include "boltzmann_structs.h"

#include "alloc2_a.h"

#include "alloc2.h"
int alloc2(struct state_struct *state, int setup) {
  /*
    Allocate space for the text strings to store all of the
    information within the reactions file. 
    Also allocate space for the reaction structure and meta data. 
    Sets the following fields of state, allocating space for those that
    are pointers: if setup = 1.

    if (setup = 0) the _text pointers and corresponding _length fields
    are not set, 
    if Setup = 2, raw_molecules_text is not allocated.

      reaction_titles_length,
      pathway_text_length,
      compartment_text_length,
      rxn_title_text,
      molecule_text_length,
      regulation_text_length,
      rxn_title_text,
      pathway_text,
      compartment_text,
      regulation_text,
      molecules_text,
      raw_molecules_text,
      reactions,
      reactions_matrix,
      reactions_matrix->rxn_ptrs,
      reactions_matrix->molecules_indices,
      reactions_matrix->compartment_indices,
      reactions_matrix->coefficients,
      reactions_matrix->solvent_coefficients,
      reactions_matrix->text,
      unsorted_molecules,
      sorted_molecules,
      unsorted_cmpts,
      sorted_compartments,
      activities,
      enzyme_level,
      forward_rc,
      reverse_rc,
      reg_constant,
      reg_exponent,
      reg_drctn,
      reg_species,
      coeff_sum,
      use_rxn,
      dg0tfs_set,
      vgrng_state,
      vgnrg2_state
      

    Called by: boltzmann_init, boltzmann_flatten_alloc1
    Calls:     alloc2_a, calloc, fprintf, fflush 
  */
  struct reaction_struct rs;
  struct reactions_matrix_struct rms;
  struct reactions_matrix_struct *reactions_matrix;
  struct molecule_struct ises;
  struct compartment_struct ces;
  int64_t usage;
  int64_t align_len;
  int64_t align_mask;
  int64_t ask_for;
  int64_t one_l;
  int64_t nze;
  int64_t num_rxns;
  int64_t num_molecules;
  int64_t num_cmpts;
  int64_t num_regulations;
  int64_t max_regs_per_rxn;
  int64_t double_size;
  int64_t int64_t_size;
  int64_t int_size;
  int success;
  int padi;
  FILE *lfp;
  FILE *efp;
  success    	     = 1;
  usage      	     = state->usage;
  one_l      	     = (int64_t)1;
  align_mask 	     = state->align_mask;
  align_len  	     = state->align_len;
  num_rxns   	     = state->number_reactions;
  num_molecules      = state->number_molecules;
  num_cmpts          = state->number_compartments;
  max_regs_per_rxn   = state->max_regs_per_rxn;
  lfp                = state->lfp;
  num_regulations    = max_regs_per_rxn * num_rxns;
  double_size        = (int64_t)sizeof(double);
  int64_t_size       = (int64_t)sizeof(int64_t);
  int_size           = (int64_t)sizeof(int);

  if (setup > 0) {
    /*
      Allocate space for the text strings in the reactions file.
    */
    success = alloc2_a(state,setup);
  }
  if (success) {
    /*
      Allocate space for the reaction meta data.
    */
    ask_for = (num_rxns+one_l) * ((int64_t)sizeof(rs));
    usage   += ask_for;
    state->reactions = (struct reaction_struct *)calloc(one_l,ask_for);
    if (state->reactions == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reaction meta data \n",
	      ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    /*
      Allocate space for the reaction matrix.
    */
    ask_for = (int64_t)sizeof(rms);
    usage   += ask_for;
    reactions_matrix  = (struct reactions_matrix_struct *)calloc(one_l,ask_for);
    if (reactions_matrix) {
      state->reactions_matrix = reactions_matrix;
    } else {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reaction meta data \n",
		ask_for);
	fflush(lfp);
      }
    }
  }
  /*
    Allocate space for the reaction matrix arrays, each having nze elements
    where nze = num_molecules
  */
  if (success) {
    ask_for = ((int64_t)(num_rxns + 1))*int64_t_size;
    usage += ask_for;
    reactions_matrix->rxn_ptrs = (int64_t*)calloc(one_l,ask_for);
    if (reactions_matrix->rxn_ptrs == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
		"for reactions_matrix->rxn_ptrs\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    nze = num_molecules;
    ask_for = nze * int64_t_size;
    usage += ask_for;
    reactions_matrix->molecules_indices = (int64_t*)calloc(one_l,ask_for);
    if (reactions_matrix->molecules_indices == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
		"for reactions_matrix->molecules_indices\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    usage += ask_for;
    reactions_matrix->compartment_indices = (int64_t*)calloc(one_l,ask_for);
    if (reactions_matrix->compartment_indices == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reactions_matrix->compartment_indices\n",ask_for);
	fflush(lfp);
      }
    }
  }
  
  if (success) {
    ask_for = nze * double_size;
    usage += ask_for;
    reactions_matrix->coefficients = (double *)calloc(one_l,ask_for);
    if (reactions_matrix->coefficients == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reactions_matrix->coefficients\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = nze * double_size;
    usage += ask_for;
    reactions_matrix->recip_coeffs = (double*)calloc(one_l,ask_for);
    if (reactions_matrix->recip_coeffs == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reactions_matrix->recip_coeffs\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = 2 * num_rxns * double_size;
    usage += ask_for;
    reactions_matrix->solvent_coefficients = (double*)calloc(one_l,ask_for);
    if (reactions_matrix->solvent_coefficients == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reactions_matrix->solvent_coefficients\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = nze * int64_t_size;
    usage += ask_for;
    reactions_matrix->text = (int64_t *)calloc(one_l,ask_for);
    if (reactions_matrix->text == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for reactions_matrix->text\n",ask_for);
	fflush(lfp);
      }
    }
  }
  /*
    Allocate space to store the sorted molecules pointers,
    and scratch space for sorting the molecules pointers.
  */
  if (success) {
    ask_for = num_molecules * ((int64_t)sizeof(ises));
    if (setup == 1) {
      ask_for = ask_for << 1;
    }
    usage += ask_for;
    state->sorted_molecules = (struct molecule_struct *)calloc(one_l,ask_for);
    if (state->sorted_molecules) {
      /*
	Caution address arithmetic follows.
	state->sorted_molecules = &state->unsorted_molecules[num_molecules];
      */
      if (setup == 1) {
	state->unsorted_molecules = state->sorted_molecules + num_molecules;
      }
    } else {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
	      "for sorted and unsorted molecules pointers\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_cmpts * ((int64_t)sizeof(ces));
    if (setup == 1) {
      ask_for = ask_for << 1;
    }
    usage += ask_for;
    state->sorted_compartments = (struct compartment_struct *)calloc(one_l,ask_for);
    if (state->sorted_compartments) {
      /*
	Caution address arithmetic follows.
	state->sorted_compartments = &state->unsorted_cmpts[num_cmpts];
      */
      if (setup == 1) {
	state->unsorted_cmpts = state->sorted_compartments + num_cmpts;
      }
    } else {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error, unable to allocate %ld bytes of space "
		"for sorted and unsorted molecules pointers\n",ask_for);
	fflush(lfp);
      }
    }
  } /* end if (success) */
  /*
    We need a compartment tracking vector of int type of length num_cmpts
    for use in unique_compartment_core and translate_compartments routines.
  */
  if (success) {
    if (setup == 1) {
      ask_for = (num_cmpts + (num_cmpts & one_l)) * ((int64_t)sizeof(int));
      usage += ask_for;
      state->cmpt_tracking = (int*)calloc(one_l,ask_for);
      if (state->cmpt_tracking == NULL) {
	success = 0;
	if (lfp) {
	  fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
		  "state->cmpt_tracking\n",ask_for);
	  fflush(lfp);
	}
      }
    }
  }
  if (success) {
    ask_for = num_rxns * double_size;
    usage += ask_for;
    state->activities= (double*)calloc(one_l,ask_for);
    if (state->activities == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->activities field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_rxns * double_size;
    usage += ask_for;
    state->enzyme_level = (double*)calloc(one_l,ask_for);
    if (state->enzyme_level == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->enzyme_level field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_rxns * double_size;
    ask_for = ask_for + ask_for;
    usage += ask_for;
    state->forward_rc = (double*)calloc(one_l,ask_for);
    if (state->forward_rc == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->forward_rc.\n",ask_for);
	fflush(lfp);
      }
    } else {
      state->reverse_rc = (double *)&state->forward_rc[num_rxns];
    }
  }
  if (success) {
    ask_for = num_rxns * max_regs_per_rxn * double_size;
    usage += ask_for;
    state->reg_constant = (double*)calloc(one_l,ask_for);
    if (state->reg_constant == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->reg_constant field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_rxns * max_regs_per_rxn * double_size;
    usage += ask_for;
    state->reg_exponent = (double*)calloc(one_l,ask_for);
    if (state->reg_exponent == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->reg_exponent field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_rxns * max_regs_per_rxn * double_size;
    usage += ask_for;
    state->reg_drctn = (double*)calloc(one_l,ask_for);
    if (state->reg_drctn == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->reg_drctn field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_rxns * max_regs_per_rxn * int64_t_size;
    usage += ask_for;
    state->reg_species = (int64_t*)calloc(one_l,ask_for);
    if (state->reg_species == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->reg_species field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    ask_for = num_rxns * double_size;
    usage+=ask_for;
    state->coeff_sum = (double*)calloc(one_l,ask_for);
    if (state->coeff_sum == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
	      "state->coeff_sum field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    usage += ask_for;
    state->use_rxn = (int *)calloc(one_l,ask_for);
    if (state->use_rxn == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
		"state->use_rxn field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  if (success) {
    usage += ask_for;
    state->dg0tfs_set = (int *)calloc(one_l,ask_for);
    if (state->dg0tfs_set == NULL) {
      success = 0;
      if (lfp) {
	fprintf(lfp,"alloc2: Error unable to allocate %ld bytes for "
		"state->dg0tfs_set field.\n",ask_for);
	fflush(lfp);
      }
    }
  }
  /* 
     vgrng_state and vgrng2_state are now allocated in alloc0!
  */
  state->usage = usage;
  return(success);
}
