/* molecules_lookup.c
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

#include "molecules_lookup.h"

int molecules_lookup(char *molecule_name, struct state_struct *state) {
  /*
    Return the index of the molecule_name in the unique_molecules sorted list
    or -1 if not found.
    Called by: read_initial_concentrations.
  */
  struct istring_elem_struct *sorted_molecules;
  char *molecules;
  int index;
  int n;

  int left;
  int right;

  int mid;
  int crslt;
  index = -1;
  sorted_molecules = state->sorted_molecules;
  n     = state->unique_molecules;
  crslt = strcmp(molecule_name,sorted_molecules[0].string);
  if (crslt >= 0) {
    if (crslt == 0) {
      index = 0;
    } else {
      left = 0;
      crslt = strcmp(molecule_name,sorted_molecules[n-1].string);
      if (crslt <= 0) {
	if (crslt == 0) {
	  index = n-1;
	} else {
	  right = n-1;
	  mid = (left + right) >> 1;
	  while (mid != left) {
	    crslt = strcmp(molecule_name,sorted_molecules[mid].string);
	    if (crslt == 0) {
	      index = mid;
	      break;
	    } else {
	      if (crslt  < 0) {
		right = mid;
	      } else {
		left  = mid;
	      }
	      mid = (left + right) >> 1;
	    }
	  }
	}
      }
    }
  }
  return(index);
}
