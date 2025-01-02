#include <assert.h>

/*
 * Differentiate an expression with respect to provided variables at the current
 * values of all variables. Derivative values are stored in a provided array.
 *
 * struct Node expr:
 *
 *     Root node of expression to forward_diff.
 *
 * struct VarListNode * wrt:
 *
 *     Linked list. Each node contains a pointer to a variable. These are the variables
 *     with respect to which we forward_diff. We need this so we can eventually
 *     construct a sparse derivative matrix. TODO: Should we construct these during
 *     the AD algorithm?
 *
 * double * values:
 *
 *     Array of derivative values corresponding to each variable. Only indices
 *     corresponding to variables in `wrt` have meaningful values.
 *
 * int nvar:
 *
 *     Number of variables (total, not just in this expression).
 *
 */
// TODO: The linked list data structure is convenient for construction,
// but not that easy to work with. Should I update this function to return
// an array.
int forward_diff(struct Node expr, struct VarListNode * wrt, double * values, int nvar);

struct CSRMatrix forward_diff_expression(struct Node expr, int nvar);

// Need intermediate storage
// Can I do this with O(1) storage?
//
int _forward_diff_constant(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _forward_diff_variable(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _forward_diff_expression(struct Node expr, struct VarListNode * wrt, double * values, int nvar);

int forward_diff(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  switch(expr.type){
    case CONST_NODE:
      return _forward_diff_constant(expr, wrt, values, nvar);
    case VAR_NODE:
      return _forward_diff_variable(expr, wrt, values, nvar);
    case OP_NODE:
      return _forward_diff_expression(expr, wrt, values, nvar);
  }
}

int _forward_diff_constant(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  for (int i=0; i<nvar; i++){
    values[0] = 0.0;
  }
  return nvar;
}

int _forward_diff_variable(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  struct VarListNode * node = wrt;
  // If I didn't have wrt, I could just say values[expr.data.var->index] = 1.0
  while (node){
    int idx = expr.data.var->index;
    if (idx == node->variable->index){
      if (values[idx] == 0){
        values[idx] = 1.0;
      } else {
        printf("ERROR: Derivative WRT var %d already has a value", idx);
        exit(-1);
      }
    }
    node = node->next;
  }
  return nvar;
}

int _forward_diff_expression(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  // expr = f(arg1, arg2, ...)
  // df/d(wrt) = f'(arg1(wrt), arg2(wrt), ...) * (d(arg1)/d(wrt) + d(arg2)/d(wrt) + ...)
  double deriv_op[expr.data.expr->nargs];
  // Evaluate the derivative of the operator. This is a vector of multipliers
  // for the derivatives of each argument.
  DIFF_OP[expr.data.expr->op](expr.data.expr->args, expr.data.expr->nargs, deriv_op);

  for (int i=0; i<expr.data.expr->nargs; i++){
    // This is inefficient. I allocate an array of size nvar for every
    // argument of every expression.
    // This is forward differentiation. But presumably I could store these
    // as vectors in the space of `wrt` variables?
    double arg_values[nvar];
    for (int j=0; j<nvar; j++){arg_values[j] = 0.0;}

    forward_diff(expr.data.expr->args[i], wrt, arg_values, nvar);

    for (int j=0; j<nvar; j++){
      values[j] += deriv_op[i] * arg_values[j];
    }
  }
  return 0;
}

/*
 * Differentiate an expression and return a single CSR matrix
 * (here, a sparse vector).
 */
struct CSRMatrix forward_diff_expression(struct Node expr, int nvar){
  int nrow = 1;
  int eidx = 0;

  int in_expr[nvar];
  double deriv_values[nvar];

  for (int i=0; i<nvar; i++){
    in_expr[i] = -1;
    deriv_values[i] = 0.0;
  }

  struct VarListNode * varlist = NULL;
  int nnz = identify_variables(expr, eidx, in_expr, nvar, &varlist);
  int ret = forward_diff(expr, varlist, deriv_values, nvar);

  // NOTE: These arrays will have to be freed later.
  int * indices = malloc(sizeof(int) * nnz);
  int * indptr = malloc(sizeof(int) * 2);
  double * csr_values = malloc(sizeof(double) * nnz);

  // We only have one row, so indptr is trivial
  indptr[0] = 0;
  indptr[1] = nnz;

  struct VarListNode * tempnode = varlist;
  for (int i=0; i<nnz; i++){
    indices[i] = tempnode->variable->index;
    csr_values[i] = deriv_values[indices[i]];
    tempnode = tempnode->next;
  }
  // Make sure nnz is the length of our linked list.
  assert(tempnode == NULL);

  struct CSRMatrix deriv_matrix = {
    .nnz = nnz,
    .nrow = 1,
    .ncol = nvar,
    .indptr = indptr,
    .indices = indices,
    .values = csr_values,
  };

  free_varlist(&varlist);

  return deriv_matrix;
}
