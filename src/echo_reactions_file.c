/* echo_reactions_file.c
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
#include "recover_solvent_coefficients.h"
#include "zero_solvent_coefficients.h"
#include "echo_reactions_file.h"
int echo_reactions_file(struct state_struct *state) {
  /*
    Echo the reactions file to a rxns.echo file.
    Called by: echo_inputs
    Calls:     fopen, fprintf, fclose (intrinsic)
  */
  struct reaction_struct *reaction;
  struct reactions_matrix_struct *rxns_matrix;
  struct molecule_struct *molecules;  
  struct molecule_struct *molecule;  
  struct compartment_struct *compartments;
  struct compartment_struct *compartment;

  double  *reg_constant;
  double  *reg_exponent;
  double  *reg_drctn;
  int64_t *reg_species;
  int64_t *rxn_ptrs;
  int64_t *molecules_indices;
  int64_t *compartment_indices;
  double  *coefficients;
  int64_t *molecule_name_ptr;
  int64_t metabolite;
  int64_t reg_base;
  double  coeff;

  char *rxn_title_text;
  char *pathway_text;
  char *compartment_text;
  char *molecules_text;
  char *regulation_text;

  char *title;
  char *pathway;
  char *compartment_name;
  char *molecule_name;

  int success;
  int rxns;

  int np;
  int nr;
  
  int j;
  int i;

  int max_regs_per_rxn;
  int use_regulation;

  int ci;
  int padi;

  char tab;
  char padc[7];

  FILE *rxn_echo_fp;
  FILE *lfp;

  tab  = (char)9;
  success = 1;
  rxn_echo_fp = fopen(state->rxn_echo_file,"w+");
  if (rxn_echo_fp == NULL) {
    fprintf(stderr,
	    "echo_inputs: Error could not open %s file.\n",state->rxn_echo_file);
    success = 0;
  }
  if (success) {
    /*
      Recover the solvent coefficients.
    */
    success = recover_solvent_coefficients(state);
  }
  if (success) {
    molecules         	= state->sorted_molecules;
    rxn_title_text    	= state->rxn_title_text;
    pathway_text      	= state->pathway_text;
    compartments        = state->sorted_compartments;
    compartment_text  	= state->compartment_text; 
    molecules_text    	= state->molecules_text;
    regulation_text   	= state->regulation_text;
    use_regulation    	= (int)state->use_regulation;
    reg_species       	= state->reg_species;
    reg_drctn         	= state->reg_drctn;
    reg_constant      	= state->reg_constant;
    reg_exponent      	= state->reg_exponent;
    reaction          	= state->reactions;
    rxns_matrix       	= state->reactions_matrix;
    max_regs_per_rxn  	= state->max_regs_per_rxn;
    rxn_ptrs          	= rxns_matrix->rxn_ptrs;
    molecules_indices 	= rxns_matrix->molecules_indices;
    compartment_indices = rxns_matrix->compartment_indices;
    coefficients      	= rxns_matrix->coefficients;
    molecule_name_ptr  	= rxns_matrix->text;
    reg_base          	= 0;
    for (rxns=0;rxns < (int)state->number_reactions;rxns++) {
      if (reaction->title>=0) {
	title = (char *)&rxn_title_text[reaction->title];
	fprintf(rxn_echo_fp,"REACTION\t%s\n",title);
      }
      if (reaction->pathway>=0) {
	pathway = (char *)&pathway_text[reaction->pathway];
	fprintf(rxn_echo_fp,"PATHWAY\t%s\n",pathway);
      }
      if (reaction->lcompartment>=0) {
	if (reaction->rcompartment>=0) {
	  compartment_name = (char *)&compartment_text[reaction->lcompartment];
	  fprintf(rxn_echo_fp,"LEFT_COMPARTMENT\t%s\n",compartment_name);
	  compartment_name = (char *)&compartment_text[reaction->rcompartment];
	  fprintf(rxn_echo_fp,
		  "RIGHT_COMPARTMENT\t%s\n",compartment_name);
	} else {
	  compartment_name = (char *)&compartment_text[reaction->lcompartment];
	  fprintf(rxn_echo_fp,"COMPARTMENT\t%s\n",compartment_name);
	}
      }
      fprintf(rxn_echo_fp,"LEFT\t");
      nr = 0;
      for (j=rxn_ptrs[rxns];j<rxn_ptrs[rxns+1];j++) {
	coeff = coefficients[j];
	ci    = compartment_indices[j];
	if (coeff < 0.0) {
	  if (coeff != -1.0) {
	    coeff = -coeff;
	    fprintf(rxn_echo_fp,"%le ",coeff);
	  }
	  molecule_name = (char*)&molecules_text[molecule_name_ptr[j]];
	  if (ci > 0) {
	    compartment = (struct compartment_struct *)&compartments[ci];
	    compartment_name = (char*)&compartment_text[compartment->string];
	    fprintf(rxn_echo_fp,"%s:%s",molecule_name,compartment_name);
	  } else {
	    fprintf(rxn_echo_fp,"%s",molecule_name);
	  }
	  nr += 1;
	  if (nr < reaction->num_reactants) {
	    fprintf(rxn_echo_fp," + ");
	  } else {
	    break;
	  }
	}
      }
      fprintf(rxn_echo_fp,"\n");
      fprintf(rxn_echo_fp,"RIGHT\t");
      np = 0;
      for (j=rxn_ptrs[rxns];j<rxn_ptrs[rxns+1];j++) {
	coeff = coefficients[j];
	ci    = compartment_indices[j];
	if (coeff > 0.0) {
	  if (coeff != 1.0) {
	    fprintf(rxn_echo_fp,"%le ",coeff);
	  }
	  molecule_name = (char*)&molecules_text[molecule_name_ptr[j]];
	  if (ci > 0) {
	    compartment = (struct compartment_struct *)&compartments[ci];
	    compartment_name = (char*)&compartment_text[compartment->string];
	    fprintf(rxn_echo_fp,"%s:%s",molecule_name,compartment_name);
	  } else {
	    fprintf(rxn_echo_fp,"%s",molecule_name);
	  }
	  np += 1;
	  if (np < reaction->num_products) {
	    fprintf(rxn_echo_fp," + ");
	  } else {
	    fprintf(rxn_echo_fp,"\n");
	    break;
	  }
	}
      }
      if (reaction->deltag0_computed) {
	fprintf(rxn_echo_fp,"DGZERO\t%le, computed\n",reaction->delta_g0);
      } else {
	fprintf(rxn_echo_fp,"DGZERO\t%le, input\n",reaction->delta_g0);
      }
      if (reaction->unit_i == 0) {
	fprintf(rxn_echo_fp,"DGZERO-UNITS\tkCal/mol\n");
      } else {
	fprintf(rxn_echo_fp,"DGZERO-UNITS\tkJ/mol\n");
      }
      if (state->use_activities && (state->use_regulation == 0)) {
	fprintf(rxn_echo_fp,"ACTIVITY\t%le\n",reaction->activity);
      }
      if (use_regulation) {
	for (i=0;i<max_regs_per_rxn;i++) {
	  metabolite = reg_species[reg_base+i];
	  /*
	    At this point metabolite is not the pointer into regulation text,
	    as was set in parse_reactions file,
	    but the sorted molecule number, as set by 
	    translate_regulation_metabolites.
	  */
	  if (metabolite >= 0) {
	    molecule = (struct molecule_struct *)&molecules[metabolite];
	    molecule_name  = (char *)&molecules_text[molecule->string];
	    ci = molecule->c_index;
	    compartment_name = NULL;
	    if (ci > 0) {
	      compartment = (struct compartment_struct *)&compartments[ci];
	      compartment_name = (char*)&compartment_text[compartment->string];
	    }
	    if (reg_drctn[reg_base+i] > 0.0) {
	      if (ci == 0) {
		fprintf(rxn_echo_fp,"PREGULATION\t%s\t%le\t%le\n",
			molecule_name,
			reg_constant[reg_base+i],
			reg_exponent[reg_base+i]);
	      } else {
		fprintf(rxn_echo_fp,"PREGULATION\t%s:%s\t%le\t%le\n",
			molecule_name,compartment_name,
			reg_constant[reg_base+i],
			reg_exponent[reg_base+i]);
	      }
	    } else {
	      if (ci == 0) {
		fprintf(rxn_echo_fp,"NREGULATION\t%s\t%le\t%le\n",
			molecule_name,
			reg_constant[reg_base+i],
			reg_exponent[reg_base+i]);
	      } else {
		fprintf(rxn_echo_fp,"NREGULATION\t%s:%s\t%le\t%le\n",
			molecule_name,compartment_name,
			reg_constant[reg_base+i],
			reg_exponent[reg_base+i]);
	      }
	    }
	  } else {
	    break;
	  }
	}
	reg_base += max_regs_per_rxn;
      }
      fprintf(rxn_echo_fp,"//\n");
      reaction += 1; /* Caution address arithmetic. */
    }
    fclose(rxn_echo_fp);
    /*
      Now we need to rezero the solvent coefficents.
    */
    if (success) {
      success = zero_solvent_coefficients(state);
    }
  }
  return(success);
}
