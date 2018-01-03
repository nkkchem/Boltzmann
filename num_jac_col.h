#ifndef _NUM_JAC_COL_H_
#define _NUM_JAC_COL_H_ 1
extern int num_jac_col(struct state_struct *state,
	   	       int ny, int j,
		       double *y,
		       double *f,
		       double *delj,
		       double threshj,
		       double *y_counts,
		       double *fdel,
		       double *fdiff,
		       double *forward_rxn_likelihoods,
		       double *reverse_rxn_likelihoods,
		       double *dfdy_colj,
		       double *absfdiffmax_p,
		       double *absfvaluerm_p,
		       double *absfdelrm_p,
		       double *infnormdfdy_colj_p);
#endif