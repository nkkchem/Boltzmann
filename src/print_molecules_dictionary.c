/* print_molecules_dictionary.c
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

#include "print_molecules_dictionary.h"
int print_molecules_dictionary(struct state_struct *state) {
  /*
    Print the  unique molecules in sorted order and the 
    header line for the counts.out and the concs.out file
    Called by: echo_inputs
    Calls:     fopen, fprintf, fclose (intrinsic)
  */
  struct molecule_struct *cur_molecule;
  struct compartment_struct *cur_cmpts;
  struct compartment_struct *cur_cmpt;
  double *molecule_dg0tfs;
  char *compartment_text;
  char *molecules_text;
  char *cmpt_string;
  char *molecule;

  int i;
  int oi;

  int ci;
  int success;

  int nu_molecules;
  int padi;

  FILE *dict_fp;
  FILE *counts_fp;
  FILE *concs_fp;
  FILE *lfp;
  success = 1;
  nu_molecules     = state->nunique_molecules;
  cur_molecule     = state->sorted_molecules;
  cur_cmpts        = state->sorted_compartments;
  counts_fp        = state->counts_out_fp;
  concs_fp         = state->concs_out_fp;
  molecule_dg0tfs  = state->molecule_dg0tfs;
  molecules_text   = state->molecules_text;
  compartment_text = state->compartment_text;
  dict_fp = fopen(state->dictionary_file,"w+");
  if (dict_fp == NULL) {
    fprintf(stderr,
	    "print_molecules_dictionary: Error could not open %s\n",
	    state->dictionary_file);
    fflush(stderr);
    success = 0;
  }
  cmpt_string = NULL;
  oi          = -1;
  if (success) {
    if (counts_fp) {
      fprintf(counts_fp,"iter");
    }
    fprintf(dict_fp,"number\tname\tfree_energy_of_formation\n");
    for (i=0;i<nu_molecules;i++) {
      ci = cur_molecule->c_index;
      molecule    = (char *)&molecules_text[cur_molecule->string];
      if (ci != oi) {
	oi = ci;
	if (ci > 0) {
	  cur_cmpt = (struct compartment_struct *)&(cur_cmpts[ci]);
	  cmpt_string = (char *)&compartment_text[cur_cmpt->string];
	}
      }
      if (ci > 0) {
	fprintf(dict_fp,"%d\t%s:%s\t%le\n",i,molecule,cmpt_string,molecule_dg0tfs[i]);
	if ((cur_molecule->solvent == 0) || (cur_molecule->variable == 1)) {
	  if (counts_fp) {
	    fprintf(counts_fp,"\t%s:%s",molecule,cmpt_string);
	  }
	  if (concs_fp) {
	    fprintf(concs_fp,"\t%s:%s",molecule,cmpt_string);
	  }
	}
      } else {
	fprintf(dict_fp,"%d\t%s\t%le\n",i,molecule,molecule_dg0tfs[i]);
	if ((cur_molecule->solvent == 0) || (cur_molecule->variable == 1)) {
	  if (counts_fp) {
	    fprintf(counts_fp,"\t%s",molecule);
	  }
	  if (concs_fp) {
	    fprintf(concs_fp,"\t%s",molecule);
	  }
	}
      }
      cur_molecule += 1; /* Caution address arithmetic. */
    }
    if (counts_fp) {
      fprintf(counts_fp,"\n");
    }
    if (concs_fp) {
      fprintf(concs_fp,"\n");
    }
    fclose(dict_fp);
  }
  return(success);
}
