/* read_params.c
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
#include "read_params.h"
int read_params (char *param_file_name, struct state_struct *state) {
  /*
    Read paramaters for the boltzmann code to determine equilbrium
    concentrations of a set of reactions via Monte Carlo methods.

    Called by: boltzmann_init
    Calls:     fopen, fprintf, fgets, feof, sscanf, strncmp (intrinsic)
  */
  struct vgrng_state_struct *vgrng_state;
  struct vgrng_state_struct *vgrng2_state;
  double rt;
  double m_r_rt;
  double m_rt;
  double ideal_gas_r;
  double temp_kelvin;
  double min_conc;
  double default_volume_candidate;
  int64_t max_param_line_len;
  char *param_buffer;
  char *key;
  char *value;
  char *rtp;
  int success;
  int sscan_ok;
  FILE *in_fp;
  success = 1;
  vgrng_state  = state->vgrng_state;
  vgrng2_state = state->vgrng2_state;
  if (param_file_name == NULL) {
    fprintf(stdout,"read_params: Warning no param_file_name given, looking for ./boltzmann.in input file.\n");
    fflush(stdout);
    strcpy(state->params_file,"./boltzmann.in");
  } else {
    strcpy(state->params_file,param_file_name);
  }
  in_fp = fopen(state->params_file,"r");
  if (in_fp == NULL) {
    fprintf(stderr,"read_params: Error opening %s.\n",state->params_file);
    fflush(stderr);
    success = 0;
  }
  if (success) {
    /*
      Set defaults.
    */
    vgrng_state->fib_seed[0] = (int64_t)6765109461;
    vgrng_state->fib_seed[1] = (int64_t)1771128657;
    vgrng_state->lcg_seed    = (int64_t)4636875025;

    vgrng2_state->fib_seed[0] = (int64_t)1649015676;
    vgrng2_state->fib_seed[1] = (int64_t)7568311771;
    vgrng2_state->lcg_seed    = (int64_t)5205784363;
    /*
      We used to use the same seeds for both generators.
    vgrng2_state->fib_seed[0] = (int64_t)6765109461;
    vgrng2_state->fib_seed[1] = (int64_t)1771128657;
    vgrng2_state->lcg_seed    = (int64_t)4636875025;
    */

    strcpy(state->reaction_file,"./rxns.dat");
    strcpy(state->ms2js_file,"./modelseed_2_json.srt");
    strcpy(state->pseudoisomer_file,"./pseudoisomer_dg0f.txt");
    /*
    strcpy(state->init_conc_file,"./rxns.concs");
    strcpy(state->log_file,"./boltzmann.log");
    strcpy(state->output_file,"./boltzmann.out");
    strcpy(state->counts_out_file,"./counts.out");
    strcpy(state->rxn_lklhd_file,"./rxns.lklhd");
    strcpy(state->free_energy_file,"./rxns.fe");
    strcpy(state->restart_file,"./restart.concs");
    strcpy(state->rxn_view_file,"./rxns.view");
    strcpy(state->bndry_flux_file,"./boundary_flux.txt");
    */
    state->init_conc_file[0]   	= '\0';
    state->log_file[0]         	= '\0';
    state->output_file[0]      	= '\0';
    state->counts_out_file[0]  	= '\0';
    state->ode_concs_file[0]   	= '\0';
    state->rxn_lklhd_file[0]   	= '\0';
    state->free_energy_file[0] 	= '\0';
    state->restart_file[0]     	= '\0';
    state->rxn_view_file[0]    	= '\0';
    state->bndry_flux_file[0]  	= '\0';
    state->compartment_file[0] 	= '\0';
    state->sbml_file[0]        	= '\0';
    state->rxn_echo_file[0]    	= '\0';
    state->rxn_mat_file[0]     	= '\0';
    state->dg0ke_file[0]       	= '\0';
    state->dictionary_file[0]  	= '\0';
    state->ode_dconcs_file[0]   = '\0';
    state->ode_bflux_file[0]    = '\0';
    state->ode_lklhd_file[0]    = '\0';
    state->net_lklhd_file[0]    = '\0';
    state->nl_bndry_flx_file[0] = '\0';
    state->concs_out_file[0]    = '\0';
    /*
      Following line Added by DGT on 4/18/2013
     */
    strcpy(state->solvent_string,"H2O");
    strcpy(state->input_dir,"./");
    strcpy(state->output_dir,"./");
    state->align_len        = (int64_t)64;
    state->max_filename_len = (int64_t)128;
    state->max_param_line_len = (int64_t)128;
    state->align_mask       = state->align_len - (int64_t)1;
    /*
    state->ideal_gas_r      = 0.00198858775;
    */
    state->ideal_gas_r      = 0.008314;
    state->temp_kelvin      = 298.15;
    state->kf_base_reaction = 1.0;
    /*
      Following 2 lines added by DGT on 4/15/2013
    */
    state->ph                  = 7.5;
    state->ionic_strength      = 0.15;
    state->joules_per_cal      = 4.184;
    state->epsilon             = 0.0000001;
    state->avogadro            = 6.022214179e23;
    state->recip_avogadro      = 1.0/state->avogadro;
    state->cals_per_joule      = 1.0/state->joules_per_cal;
    state->default_volume      = 1.0e-15;
    state->recip_default_volume = 1.0e15;
    state->ode_t_final         = 10.0;
    /*
    state->max_log_g0_sum      = 704.0;
    */
    state->max_log_g0_sum      = 100.0;
    state->dg0_scale_factor    = .001;
    state->flux_scaling        = 0.0;
    /*
    state->min_conc            = 1.0e-52;
    */
    state->min_conc            = 0.0;
    state->warmup_steps        = (int64_t)1000;
    state->record_steps        = (int64_t)1000;
    state->free_energy_format  = (int64_t)0;
    state->rxn_view_freq       = (int64_t)0;
    state->ode_rxn_view_freq   = (int64_t)0;
    state->count_view_freq     = (int64_t)0;
    state->lklhd_view_freq     = (int64_t)0;
    state->fe_view_freq        = (int64_t)0;
    state->use_activities      = (int64_t)0;
    state->use_deq             = (int64_t)0;
    state->no_round_from_deq   = (int64_t)0;
    state->adjust_steady_state = (int64_t)0;
    state->print_output        = (int64_t)0;
    state->use_pseudoisomers   = (int64_t)1;
    state->use_deltag0         = (int64_t)0;
    state->use_metropolis      = (int64_t)1;
    state->use_regulation      = (int64_t)1;
    state->max_regs_per_rxn    = (int64_t)4;
    state->base_reaction       = (int64_t)0;
    state->ode_solver_choice   = (int64_t)0;
    state->delta_concs_choice  = (int64_t)0;
    state->concs_or_counts     = (int64_t)3;

    state->default_initial_count = (int64_t)0;
    param_buffer       = state->param_buffer;
    max_param_line_len = state->max_param_line_len;
    key                = param_buffer + max_param_line_len; /* address arithmetic */
    value              = key + (max_param_line_len >> 1);
    /*
      read in parameters.
    */
    sscan_ok = 0;
    rtp = fgets(param_buffer,max_param_line_len,in_fp);
    if (rtp) {
      sscan_ok = sscanf(param_buffer,"%s %s",key,value);
    }
    while ((!feof(in_fp)) && (sscan_ok == 2)) {
      if (strncmp(key,"RXN_FILE",8) == 0) {
	sscan_ok = sscanf(value,"%s",state->reaction_file);
      } else if (strncmp(key,"RXN_LIST_FILE",13) == 0) {
	sscan_ok = sscanf(value,"%s",state->reaction_file);
      } else if (strncmp(key,"INIT_FILE",9) == 0) {
	sscan_ok = sscanf(value,"%s",state->init_conc_file);
      } else if (strncmp(key,"CONC_FILE",9) == 0) {
	sscan_ok = sscanf(value,"%s",state->init_conc_file);
      } else if (strncmp(key,"START_STOP_FILE",15) == 0) {
	sscan_ok = sscanf(value,"%s",state->init_conc_file);
      } else if (strncmp(key,"IN_DIR",6) == 0) {
	sscan_ok = sscanf(value,"%s",state->input_dir);

      } else if (strncmp(key,"OUT_DIR",7) == 0) {
	sscan_ok = sscanf(value,"%s",state->output_dir);
      } else if (strncmp(key,"OUT_FILE",8) == 0) {
	sscan_ok = sscanf(value,"%s",state->output_file);
      } else if (strncmp(key,"COUNTS_OUT_FILE",15) == 0) {
	sscan_ok = sscanf(value,"%s",state->counts_out_file);
      } else if (strncmp(key,"CONCS_OUT_FILE",15) == 0) {
	sscan_ok = sscanf(value,"%s",state->concs_out_file);
      } else if (strncmp(key,"ODE_CONCS_FILE",14) == 0) {
	sscan_ok = sscanf(value,"%s",state->ode_concs_file);
      } else if (strncmp(key,"RXN_LKLHD_FILE",14) == 0) {
	sscan_ok = sscanf(value,"%s",state->rxn_lklhd_file);
      } else if (strncmp(key,"FREE_ENERGY_FILE",16) == 0) {
	sscan_ok = sscanf(value,"%s",state->free_energy_file);
      } else if (strncmp(key,"RESTART_FILE",12) == 0) {
	sscan_ok = sscanf(value,"%s",state->restart_file);
      } else if (strncmp(key,"RXN_VIEW_FILE",13) == 0) {
	sscan_ok = sscanf(value,"%s",state->rxn_view_file);
      } else if (strncmp(key,"BNDRY_FLUX_FILE",15) == 0) {
	sscan_ok = sscanf(value,"%s",state->bndry_flux_file);
      /*
	Following 2 lines added by DGT on 4/18/2013, Modified by DJB 6/2/2013
      */	
      } else if (strncmp(key,"PSEUDOISOMER_FILE",17) == 0) {
	sscan_ok = sscanf(value,"%s",state->pseudoisomer_file);
      } else if (strncmp(key,"COMPARTMENT_FILE",16) == 0) {
	sscan_ok = sscanf(value,"%s",state->compartment_file);
      } else if (strncmp(key,"SBML_FILE",9) == 0) {
	sscan_ok = sscanf(value,"%s",state->sbml_file);
      } else if (strncmp(key,"MS2JS_FILE",9) == 0) {
	sscan_ok = sscanf(value,"%s",state->ms2js_file);
      } else if (strncmp(key,"USE_PSEUDOISOMERS",17) == 0) {
	sscan_ok = sscanf(value,"%ld",&state->use_pseudoisomers);
      } else if (strncmp(key,"USE_DELTAG0",11) == 0) {
	sscan_ok = sscanf(value,"%ld",&state->use_deltag0);
      } else if (strncmp(key,"USE_METROPOLIS",14) == 0) {
	sscan_ok = sscanf(value,"%ld",&state->use_metropolis);
      } else if (strncmp(key,"USE_REGULATION",14) == 0) {
	sscan_ok = sscanf(value,"%ld",&state->use_regulation);
      } else if (strncmp(key,"LOG_FILE",8) == 0) {
	sscan_ok = sscanf(value,"%s",state->log_file);
      } else if (strncmp(key,"SOLVENT",7) == 0) {
	sscan_ok = sscanf(value,"%s",state->solvent_string);
      } else if (strncmp(key,"ALIGN_LEN",9) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->align_len));
	if (state->align_len < 0) {
	  state->align_len = 16;
	}
	state->align_mask = state->align_len - (int64_t)1;
      } else if (strncmp(key,"MAX_REGS_PER_RXN",18) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->max_regs_per_rxn));
	if (state->max_regs_per_rxn <= 0) {
	  state->max_regs_per_rxn = 4;
	}
      } else if (strncmp(key,"EPSILON",7) == 0) {
	sscan_ok = sscanf(value,"%le",&state->epsilon);
      } else if (strncmp(key,"IDEAL_GAS_R",11) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->ideal_gas_r));
      } else if (strncmp(key,"TEMP_KELVIN",11) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->temp_kelvin));
      } else if (strncmp(key,"KF_BASE_REACTION",16) == 0) {
        /*
          Note that flux_scaling is K_f(base_rxn_reaction)*(product of reactant 
          concentrations in base reaction).
        */
	sscan_ok = sscanf(value,"%le",&(state->kf_base_reaction));
      /*
        Bill says we don't let users modify Avogadro's number
      } else if (strncmp(key,"AVOGADRO",8) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->avogadro));
      */
      } else if (strncmp(key,"FLUX_SCALING",12) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->flux_scaling));
      /*
	Following four lines addd by DGT on 4/15/2013
      */
      } else if (strncmp(key,"PH",2) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->ph));
      } else if (strncmp(key,"IONIC_STRENGTH",14) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->ionic_strength));
      } else if (strncmp(key,"DEFAULT_VOLUME",14) == 0) {
	sscan_ok = sscanf(value,"%le",&default_volume_candidate);
	if (default_volume_candidate > 0.0) {
	  state->default_volume = default_volume_candidate;
	  state->recip_default_volume = 1.0/default_volume_candidate;
	} 
      } else if (strncmp(key,"ODE_T_FINAL",11) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->ode_t_final));
      } else if (strncmp(key,"DG0_SCALE_FACTOR",16) == 0) {
	sscan_ok = sscanf(value,"%le",&(state->dg0_scale_factor));
      } else if (strncmp(key,"MIN_CONC",8) == 0) {
	sscan_ok = sscanf(value,"%le",&min_conc);
	if (min_conc >= 0.0) {
	  state->min_conc = min_conc;
	}
      } else if (strncmp(key,"RSEED0",6) == 0) {
	sscan_ok = sscanf(value,"%ld",&(vgrng_state->fib_seed[0]));
      } else if (strncmp(key,"RSEED1",6) == 0) {
	sscan_ok = sscanf(value,"%ld",&(vgrng_state->fib_seed[1]));
      } else if (strncmp(key,"RSEED2",6) == 0) {
	sscan_ok = sscanf(value,"%ld",&(vgrng_state->lcg_seed));
      } else if (strncmp(key,"RSEED3",6) == 0) {
	sscan_ok = sscanf(value,"%ld",&(vgrng2_state->fib_seed[0]));
      } else if (strncmp(key,"RSEED4",6) == 0) {
	sscan_ok = sscanf(value,"%ld",&(vgrng2_state->fib_seed[1]));
      } else if (strncmp(key,"RSEED5",6) == 0) {
	sscan_ok = sscanf(value,"%ld",&(vgrng2_state->lcg_seed));
      } else if (strncmp(key,"WARMUP_STEPS",12) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->warmup_steps));
      } else if (strncmp(key,"RXN_VIEW_FREQ",13) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->rxn_view_freq));
	if (state->rxn_view_freq < 0) {
	  state->rxn_view_freq = 0;
	}
      } else if (strncmp(key,"ODE_RXN_VIEW_FREQ",17) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->ode_rxn_view_freq));
	if (state->ode_rxn_view_freq < 0) {
	  state->ode_rxn_view_freq = 0;
	}
      } else if (strncmp(key,"COUNT_VIEW_FREQ",15) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->count_view_freq));
	if (state->count_view_freq < 0) {
	  state->count_view_freq = 1;
	}
      } else if (strncmp(key,"CONC_VIEW_FREQ",14) == 0) {
	/*
	  NB counts and concs print frequency is really controled by
	  the count_view_freq field and the CONCS_OR_COUNTS field.
	  If CONCS_OR_COUNTS is 2 or 3 concentrations are printed 
	  every count_view_freq steps, if CONCS_OR_COUNTS is 1 or 3
	  counts are printed every count_view_freq steps.
	*/
	sscan_ok = sscanf(value,"%ld",&(state->count_view_freq));
	if (state->count_view_freq < 0) {
	  state->count_view_freq = 1;
	}
      } else if (strncmp(key,"LKLHD_VIEW_FREQ",15) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->lklhd_view_freq));
	if (state->lklhd_view_freq < 0) {
	  state->lklhd_view_freq = 1;
	}
      } else if (strncmp(key,"CONCS_OR_COUNTS",15) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->concs_or_counts));
      } else if (strncmp(key,"FE_VIEW_FREQ",12) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->fe_view_freq));
	if (state->fe_view_freq < 0) {
	  state->fe_view_freq = 0;
	}
      } else if (strncmp(key,"USE_ACTIVITIES",14) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->use_activities));
	if (state->use_activities < 0) {
	  state->use_activities = 0;
	}
      } else if (strncmp(key,"USE_ENZYME_LEVELS",17) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->use_activities));
	if (state->use_activities < 0) {
	  state->use_activities = 0;
	}
      } else if (strncmp(key,"USE_DEQ",7) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->use_deq));
	if (state->use_deq < 0) {
	  state->use_deq = 0;
	}
      } else if (strncmp(key,"NO_ROUND_FROM_DEQ",17) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->no_round_from_deq));
      } else if (strncmp(key,"USE_STEADY_STATE",19) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->adjust_steady_state));
	state->use_metropolis = state->adjust_steady_state;
	if (state->adjust_steady_state < 0) {
	  state->adjust_steady_state = 0;
	}
	if (state->adjust_steady_state) {
	  state->use_metropolis = 1;
	}
      } else if (strncmp(key,"ADJUST_STEADY_STATE",19) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->adjust_steady_state));
	if (state->adjust_steady_state < 0) {
	  state->adjust_steady_state = 0;
	}
	if (state->adjust_steady_state) {
	  state->use_metropolis = 1;
	}
      } else if (strncmp(key,"BASE_REACTION",13) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->base_reaction));
      } else if (strncmp(key,"ODE_SOLVER_CHOICE",17) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->ode_solver_choice));
      } else if (strncmp(key,"DELTA_CONCS_CHOICE",18) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->delta_concs_choice));
      } else if (strncmp(key,"PRINT_OUTPUT",12) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->print_output));
      } else if (strncmp(key,"RECORD_STEPS",12) == 0) {
	sscan_ok = sscanf(value,"%ld",&(state->record_steps));
      } else if (strncmp(key,"FREE_ENERGY_FORMAT",12) == 0) {
	if (strncmp(value,"NONE",4) == 0) {
	  state->free_energy_format = (int64_t)0;
	} else if (strcmp(value,"NEG_LOG_LKLHD") == 0) {
	  state->free_energy_format = (int64_t)1;
	} else if (strcmp(value,"KJ/MOL") == 0) {
	  state->free_energy_format = (int64_t)2;
	} else if (strcmp(value,"KCAL/MOL") == 0) {
	  state->free_energy_format = (int64_t)3;
	} else {
	  sscan_ok = sscanf(value,"%ld",&(state->free_energy_format));
	  if (sscan_ok != 1) {
	    fprintf(stderr,"read_params: invalid value for free_energy "
		    "format using 0\n");
	    fflush(stderr);
	    state->free_energy_format = 0;
	  }
	}
      }
      sscan_ok = 0;
      rtp = fgets(param_buffer,max_param_line_len,in_fp);
      if (rtp) {
	sscan_ok = sscanf(param_buffer,"%s %s",key,value);
      }
    }
  }
  if (success) {
    /*
      If we were to need temp_kelvin to change during the computation,
      it would be useful to have the following in a routine say
      compute_m_r_rt to set the rt,m_rt, and m_r_rt fields of state.
    */
    ideal_gas_r = state->ideal_gas_r;
    temp_kelvin = state->temp_kelvin;
    rt          = ideal_gas_r * temp_kelvin;
    state->rt   = rt;
    if (rt > 0.0) {
      m_r_rt = -1.0/rt;
      m_rt   = 0.0 - rt;
      state->m_r_rt = m_r_rt;
      state->m_rt   = m_rt;
    } else {
      success = 0;
      fprintf (stderr,
	       "read_params: Error at temp_kelvin = 0 Ke  = 0, "
	       "nothing happens.");
      fflush(stderr);
    }
  }
  return (success);
}
