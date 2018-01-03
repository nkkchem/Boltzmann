/* alloc0.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <float.h>
#include <signal.h>

#include "boltzmann_structs.h"


#include "alloc0.h"
int alloc0(struct state_struct **state) {
  /*
    Allocate space for boltzmann_state data and the associated state
    file names. And the parameter file input buffer line, and
    the random number generator state.
    Called by: boltzmann_init
    Calls    : calloc, fprintf, fflush (intrinsic)
  */
  struct state_struct bltzs;
  struct state_struct *statep;
  struct vgrng_state_struct vss;
  int64_t max_file_name_len;
  int64_t max_param_line_len;
  int64_t ask_for;
  int64_t one_l;
  int64_t usage;
  int success;
  int num_state_files;
  ask_for           = (int64_t)sizeof(bltzs);
  one_l             = (int64_t)1;
  max_file_name_len = (int64_t)4096;
  max_param_line_len = (int64_t)4096;
  success           = 1;
  num_state_files   = 13;
  usage             = ask_for;
  statep            = (struct state_struct *)calloc(one_l,ask_for);
  *state            = statep;
  if (statep == NULL) {
    success = 0;
    fprintf(stderr,
	      "boltzmann: unable to allocate %ld bytes for state structure.\n",
	      ask_for);
    fflush(stderr);
  }
  if (success) {
    statep->max_filename_len = max_file_name_len;
    ask_for = ((int64_t)num_state_files) * max_file_name_len;
    usage   += ask_for;
    statep->params_file = (char *)calloc(one_l,ask_for);
    if (statep->params_file == NULL) {
      success = 0;
      fprintf(stderr,
	      "alloc0: unable to allocate %ld bytes for state file names.\n",
	      ask_for);
      fflush(stderr);
    }
  }
  if (success) {
    /*
	Caution Address arthmetic follows:
    */
    statep->reaction_file      = statep->params_file + max_file_name_len;
    statep->init_conc_file     = statep->reaction_file + max_file_name_len;
    statep->input_dir          = statep->init_conc_file + max_file_name_len;
    statep->output_file        = statep->input_dir + max_file_name_len;
    statep->log_file           = statep->output_file + max_file_name_len;
    statep->output_dir         = statep->log_file + max_file_name_len;
    statep->concs_out_file     = statep->output_dir + max_file_name_len;
    statep->rxn_lklhd_file     = statep->concs_out_file + max_file_name_len;
    statep->free_energy_file   = statep->rxn_lklhd_file + max_file_name_len;
    statep->restart_file       = statep->free_energy_file + max_file_name_len;
    statep->rxn_view_file      = statep->restart_file + max_file_name_len;
    statep->max_param_line_len = max_param_line_len;
    statep->max_filename_len   = max_file_name_len;
    ask_for                    = max_param_line_len << 1;
    usage                      += ask_for;
    statep->param_buffer       = (char *)calloc(one_l,ask_for);
    if (statep->param_buffer == NULL) {
      success = 0;
      fprintf(stderr,
	      "alloc0: unable to allocate %ld bytes for state->param_buffer.\n",
	      ask_for);
      fflush(stderr);
    } else {
      statep->param_key = statep->param_buffer + max_param_line_len;
      statep->param_value = statep->param_key + (max_param_line_len>>1);
    }
    statep->usage = usage;
  }
  if (success) {
    ask_for = (int64_t)sizeof(vss);
    statep->vgrng_state = (struct vgrng_state_struct *)calloc(one_l,ask_for);
    if (statep->vgrng_state == NULL) {
      success = 0;
      fprintf(stderr,
	      "alloc0: unable to allocate %ld bytes for state->vgrng_state.\n",
	      ask_for);
      fflush(stderr);
    }
  }
  return(success);
}
