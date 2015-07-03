#ifndef LDI_INTERNAL_H
#define LDI_INTERNAL_H

/* Create an LDI_ERROR given the error code. */
#define ERROR(code)  ((LDI_ERROR){code, 0})

/* Shorthand for returning an error with the LDI_ERR_NOERROR error code. */
#define NO_ERROR ((LDI_ERROR){LDI_ERR_NOERROR, 0})

#endif					/* LDI_INTERNAL_H */
