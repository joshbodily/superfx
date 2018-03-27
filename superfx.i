%module SuperFX
%{
/* Includes the header in the wrapper code */
#include "types.h"
#include "bind.h"
%}

/* Parse the header file to generate wrappers */
%include "types.h"
%include "bind.h"
