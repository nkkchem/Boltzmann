/* read_initial_concentrations.c
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

#include "molecules_lookup.h"
#include "compartment_lookup.h"
#include "upcase.h"

#include "read_initial_concentrations.h"
int read_initial_concentrations(struct state_struct *state) {
  /*
    Read the concs.in file for initial concentrations, and
    set the concentrations array.
    Called by: species_init
    Calls:     molecules_lookup
               compartment_lookup,
	       upcase,
               fopen, fgets, fclose, fprintf, fflush (intrinsic)
  */
  struct  molecule_struct *sorted_molecules;
  struct  molecule_struct *molecule;
  struct  compartment_struct *sorted_compartments;
  struct  compartment_struct *compartment;
  double  volume;
  double  recip_volume;
  double  default_volume;
  double  min_conc;
  double  conc_units;
  double  conc;
  double  count;
  double  multiplier;
  double  avogadro;
  double  units_avo;
  double  recip_avogadro;
  double  ph;
  double  ionic_strength;
  /*
  double  half;
  */
  double  e_val;
  double  u_val;
  double  *bndry_flux_counts;
  double  ntotal_opt;
  double  ntotal_exp;
  double  opt_count;
  double  exp_count;
  double  *counts;
  double  *kss_e_val;
  double  *kss_u_val;
  int64_t molecules_buff_len;
  int64_t one_l;
  char *molecules_buffer;
  char *molecule_name;
  char *compartment_name;
  char *variable_c;
  char *compute_c;
  char *solvent_string;
  char *fgp;

  int nu_molecules;
  int i;

  int nscan;
  int variable;

  int si;
  int ci;

  int mol_len;
  int cmpt_len;

  int num_fixed_concs;
  int solvent;

  int compute_conc;
  int nu_compartments;
  
  int success;
  int use_bulk_water;

  int line_count;
  int padi;

  char vc[2];
  char cc[2];
  
  FILE *conc_fp;
  FILE *lfp;

  lfp                 = state->lfp;
  ph                  = state->ph;
  ionic_strength      = state->ionic_strength;
  avogadro            = state->avogadro;
  recip_avogadro      = state->recip_avogadro;
  /*
  half                = 0.5;
  */
  nu_molecules        = state->nunique_molecules;
  nu_compartments     = state->nunique_compartments;
  molecules_buff_len  = state->max_param_line_len;
  molecules_buffer    = state->param_buffer;
  molecule_name       = molecules_buffer + state->max_param_line_len;
  compartment_name    = molecule_name + (state->max_param_line_len>>1);
  sorted_molecules    = state->sorted_molecules;
  sorted_compartments = state->sorted_compartments;
  counts              = state->current_counts;
  kss_e_val           = state->kss_e_val; 
  kss_u_val           = state->kss_u_val; 
  default_volume      = state->default_volume;
  solvent_string      = state->solvent_string;
  use_bulk_water      = state->use_bulk_water;
  bndry_flux_counts   = (double *)state->bndry_flux_counts;
  success = 1;
  one_l = (int64_t)1;

  variable_c = (char *)&vc[0];
  compute_c  = (char *)&cc[0];
  ntotal_opt = 0.0;
  ntotal_exp = 0.0;
  for (i=0;i<nu_molecules;i++) {
    counts[i] = -1.0;
    bndry_flux_counts[i] = -1.0;
  }
  num_fixed_concs = 0;
  conc_fp = fopen(state->init_conc_file,"r");
  min_conc     = state->min_conc;
  if (conc_fp) {
    /*
      Read the required volume line.
    */
    fgp = fgets(molecules_buffer,molecules_buff_len,conc_fp);
    line_count = 1;
    if (fgp) {
      if (strncmp(molecules_buffer,"VOLUME",6) != 0) {
	success = 0;
	if (lfp) {
	  fprintf(lfp,
		  "read_intial_concentrations Error: Concentrations input file "
		  "does not start with a VOLUME line.\n");
	  fflush(lfp);
	}
      } else {
	nscan = sscanf((char*)&molecules_buffer[6],"%le",&volume);
	if (nscan != 1) {
	  success = 0;
	  if (lfp) {
	    fprintf(lfp,
		  "read_intial_concentrations Error: invalid volume spec.\n");
	    fflush(lfp);
	  }
	} else {
	  if (volume <= 0.0) {
	    volume = default_volume;
	  }
	  state->default_volume = volume;
	  recip_volume = 1.0/volume;
	  state->recip_default_volume = recip_volume;
	}
      }
    } else {
      success = 0;
      if (lfp) {
	fprintf(lfp,
	      "read_initial_concentrations Error: Empty concentrations "
	      "file.\n");
	fflush(lfp);
      }
    }
  } else {
    success = 0;
    if (lfp) {
      fprintf(lfp,
	    "read_intial_concentrations Error: Unable to open inital "
	    "concentrations file, %s\n",state->init_conc_file);
      fflush(lfp);
    }
  }
  if (success) {
    fgp = fgets(molecules_buffer,molecules_buff_len,conc_fp);
    line_count += 1;
    if (fgp) {
      if (strncmp(molecules_buffer,"CONC_UNITS",10) != 0) {
	success = 0;
	if (lfp) {
	  fprintf(lfp,
		"read_intial_concentrations Error: Concentratiosn input file "
		"does not have a second line with CONC_UNITS setting.\n");
	  fflush(lfp);
	}
      } else {
	nscan = sscanf((char*)&molecules_buffer[10],"%le",&conc_units);
	if (nscan != 1) {
	  success = 0;
	  if (lfp) {
	    fprintf(lfp,
		    "read_intial_concentrations Error: invalid conc_units spec.\n");
	    fflush(lfp);
	  }
	} else {
	  state->conc_units = conc_units;
	}
      }
    } else {
      success = 0;
      if (lfp) {
	fprintf(lfp,
	      "read_initial_concentrations Error: No CONC_UNITS line in "
	      "concentrations file.\n");
	fflush(lfp);
      }
    }
  }
  if (success) {
    /*
      Initialize compartment volume, recip_volume if unset and 
      and also ph and ionic_strength. If volume is 0, the assumpition
      is that these fields were not set in read_comparment_sizes
      (called in species_init before this routine).
      ntotal_exp, ntotal_opt, conc_to_count, and count_to_conc fields.
    */
    units_avo = conc_units * avogadro;
    compartment = (struct compartment_struct *)&sorted_compartments[0];
    for (i=0;i<nu_compartments;i++) {
      if (compartment->volume <= 0.0) {
	compartment->volume         = volume;
	compartment->recip_volume   = recip_volume;
	compartment->ph             = ph;
	compartment->ionic_strength = ionic_strength;
	volume                      = default_volume;
      } else {
	volume = compartment->volume;
	/*
	compartment->recip_volume = 1.0/compartment->volume;
	*/
      }
      compartment->ntotal_exp   = 0.0;
      compartment->ntotal_opt   = 0.0;
      multiplier                = units_avo * volume;
      compartment->conc_to_count = multiplier;
      if (multiplier > 0.0) {
	compartment->count_to_conc   = 1.0/multiplier;
      } else {
	success = 0;
	if (lfp) {
	  fprintf(lfp,"read_intial_conecntrations: Error 0 or negative volume for compartment %d\n",i);
	  fflush(lfp);
	}
      }
      /*
	Because of the way we use u_val and e_val below I think
	we want to just have min_conc = count_to_conc.
      compartment->min_conc     = compartment->recip_volume * recip_avogadro;
      */
      compartment += 1; /* Caution address arithmetic */
    }
  }	
  if (success) {
    compartment = (struct compartment_struct *)&sorted_compartments[0];
    compartment->volume = volume;
    compartment->recip_volume = recip_volume;
    while (!feof(conc_fp)) {
      fgp = fgets(molecules_buffer,molecules_buff_len,conc_fp);
      line_count += 1;
      if (fgp) {
	e_val = 0.0;
	u_val = 0.0;
	nscan = sscanf(molecules_buffer,"%s %le %1s %1s %le %le",
		       molecule_name, &conc, variable_c, compute_c, 
		       &e_val, &u_val);
	variable = 1;
	solvent  = 0;
	if (nscan >= 3) {
	  /*
	    A variable or constant specifier was givven.
	  */
	  /*
	    Upper case the variable character.
          */
	  vc[0] = vc[0] & 95;

	  if (strncmp(variable_c,"F",one_l) == 0) {
	    variable = 0;
	    num_fixed_concs += 1;
	  }
	  if (strncmp(variable_c,"V",one_l) == 0) {
	    variable = 1;
	  }
	}
	compute_conc = 0;
	if (nscan >= 4) {
	  /*
	    A fixed or compute value was given for the initial concentration.
	    Upper case the compute value character.
	  */
	  cc[0] = cc[0] & 95;
	  if (strncmp(compute_c,"U",one_l) == 0) {
	    compute_conc = 0;
	  } else {
	    if (strncmp(compute_c,"I",one_l) == 0) {
	      compute_conc = 1;
	    } else {
	      if (strncmp(compute_c,"C",one_l) == 0) {
		compute_conc = 2;
	      } else {
		compute_conc =0;
	      }
	    }
	  }
	}
	mol_len = strlen(molecule_name);
	compartment_name = molecule_name;
	for (i=0;i<mol_len-1;i++) {
	  if (molecule_name[i] == ':') {
	    compartment_name = (char *)&molecule_name[i+1];
	    molecule_name[i] = '\0';
	    cmpt_len = mol_len - i - 1;
	    mol_len = i;
	    break;
	  }
	}
	if (compartment_name == molecule_name) {
	  ci = 0;
	} else {
	  upcase(cmpt_len,compartment_name);
	  ci = compartment_lookup(compartment_name,state);
	  if (ci < 0) {
	    success = 0;
	    if (lfp) {
	      fprintf(lfp,"read_initial_concentrations: Error, line %d, %s was not a compartment found in the reactions file\n",line_count,compartment_name);
	      fflush(lfp);
	    }
	  }
	}
	compartment = (struct compartment_struct *)&sorted_compartments[ci];
	/*
	*/
	volume       = compartment->volume;
	recip_volume = compartment->recip_volume;
	min_conc     = compartment->count_to_conc;
	multiplier   = compartment->conc_to_count;
	if (e_val <= 0.0) {
	  /*
	    Here if we are in a particular compartment
	    we may want to use that compartment volume instead
	    of the default volume.
	  */
	  e_val = min_conc;
	}
	if (u_val <= 0.0) {
	  u_val = min_conc;
	}
	if (nscan >= 2) {
	  upcase(mol_len,molecule_name);
	  si = molecules_lookup(molecule_name,ci,state);
	  if (strcmp(molecule_name,solvent_string) == 0) {
	    solvent = 1;
	  }
	  if ((si >=0) && si < nu_molecules) {
	    /*
	      The following uses the nearest integer to (conc*multiplier)
	      for the count field stored in the counts array, where
	      multiplier is volume * units * Avogadro's number.
	    */
	    /*
	    Continuous case, we want to allow fractional counts.
	    counts[si] = conc * multiplier;
	    opt_count  = u_val * multiplier;
	    exp_count  = e_val * multiplier;
	    */
	    /*
	      Stochastic case, only whole molecules allowed.
	      Stochastic conversion will be handled in deq_run
	      and boltzman_run
	    counts[si] = (double)((int64_t)((conc * multiplier) + half));
	    opt_count  = (double)((int64_t)((u_val * multiplier) + half));
	    exp_count  = (double)((int64_t)((e_val * multiplier) + half));
	    */
	    counts[si] = conc * multiplier;
	    opt_count  = u_val * multiplier;
	    exp_count  = e_val * multiplier;
	    /*
	    if (opt_count < 1.0) {
	      opt_count = 1.0;
	    }
	    if (exp_count < 1.0) {
	      exp_count = 1.0;
	    }
	    */
	    compartment->ntotal_opt += opt_count;
	    compartment->ntotal_exp += exp_count;
	    kss_e_val[si] = e_val;
	    kss_u_val[si] = u_val;
	    molecule = (struct molecule_struct *)&sorted_molecules[si];
	    if (solvent) {
	      if (use_bulk_water) {
		variable = 0;
	      }
	    }
	    molecule->variable = variable;
	    molecule->compute_init_conc = compute_conc;
	    molecule->solvent           = solvent;
	  } else {
	    success = 0;
	    if (lfp) {
	      if (ci == 0) {
		fprintf(lfp,"read_initial_concentrations: Error "
			"unrecognized molecule in %s line %d was %s\n",
			state->init_conc_file,line_count,molecule_name);
	      } else {
		fprintf(lfp,"read_initial_concentrations: Error "
			"unrecognized molecule in %s line %d was %s:%s\n",
			state->init_conc_file,line_count,
			molecule_name,compartment_name);
	      }
	      fflush(lfp);
	    }
	    break;
	  }
	} else {
	  success = 0;
	  if (lfp) {
	    fprintf(lfp,"read_initial_concentrations: Error "
		  "poorly formated line was\n%s\n",molecules_buffer);
	    fflush(lfp);
	  }
	}
      } /* end if (fgp) */
    } /* end while (!feof(conc_fp)) */
    fclose(conc_fp);
  } else {
    success = 0;
    if (lfp) {
      fprintf(lfp,
	    "read_initial_concentrations: Warning unable to open %s\n",
	    state->init_conc_file);
      fflush(lfp);
    }
  }
  if (success) {
    count = state->default_initial_count;
    for (i=0;i<nu_molecules;i++) {
      if (counts[i] < 0.0) {
	counts[i] = count;
      }
    }
    for (i=0;i<nu_molecules;i++) {
      bndry_flux_counts[i] = counts[i];
    }
    state->num_fixed_concs = num_fixed_concs;
    /*
      Print the initial counts to the counts output file.
      Moved to echo_inputs.
    */
  }
  return(success);
}
