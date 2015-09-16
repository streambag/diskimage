#ifndef LDI_INTERNAL_H
#define LDI_INTERNAL_H

/* Create an LDI_ERROR given the error code. */
#define ERROR(code)  ((LDI_ERROR){code, 0})

/* Create an LDI_ERROR given the error code and an error specific suberror. */
#define ERROR2(code, suberror) ((LDI_ERROR){code, suberror})

/* Shorthand for returning an error with the LDI_ERR_NOERROR error code. */
#define NO_ERROR ((LDI_ERROR){LDI_ERR_NOERROR, 0})

/* Shorthand for checking if an LDI_ERROR represents an error. */
#define IS_ERROR(err) (err.code != LDI_ERR_NOERROR)

/*
 * Default to make TESTSEAM just return the function name.
 * Unit tests will provide a TESTSEAM definition that modifies
 * the function name to provide an alternative implementation
 * of funtionname.
 */
#ifndef TESTSEAM
#define TESTSEAM(functionname) functionname
#endif

#endif					/* LDI_INTERNAL_H */
