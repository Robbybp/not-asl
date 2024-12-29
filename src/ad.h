#include <assert.h>

/*
 * Differentiate an expression with respect to provided variables at the current
 * values of all variables. Derivative values are stored in a provided array.
 *
 * struct Node expr:
 *
 *     Root node of expression to differentiate.
 *
 * struct VarListNode * wrt:
 *
 *     Linked list. Each node contains a pointer to a variable. These are the variables
 *     with respect to which we differentiate. We need this so we can eventually
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
// TODO: enum for forward/backward mode, or separate functions
// TODO: The linked list data structure is convenient for construction,
// but not that easy to work with. Should I add a wrapper function that
// returns an array?
int differentiate(struct Node expr, struct VarListNode * wrt, double * values, int nvar);

struct CSRMatrix differentiate_expression(struct Node expr, int nvar);

// Need intermediate storage
// Can I do this with O(1) storage?
//
int _differentiate_constant(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _differentiate_variable(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _differentiate_expression(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _differentiate_sum(struct Node * args, int nargs, double * deriv);
int _differentiate_product(struct Node * args, int nargs, double * deriv);
int _differentiate_subtraction(struct Node * args, int nargs, double * deriv);
int _differentiate_division(struct Node * args, int nargs, double * deriv);
int _differentiate_power(struct Node * args, int nargs, double * deriv);
int _differentiate_neg(struct Node * args, int nargs, double * deriv);
int _differentiate_sqrt(struct Node * args, int nargs, double * deriv);
int _differentiate_exp(struct Node * args, int nargs, double * deriv);
int _differentiate_log(struct Node * args, int nargs, double * deriv);
int _differentiate_sin(struct Node * args, int nargs, double * deriv);
int _differentiate_cos(struct Node * args, int nargs, double * deriv);
int _differentiate_tan(struct Node * args, int nargs, double * deriv);

// N_OPERATORS defined in expr.h
int (* DIFFERENTIATE_OP[N_OPERATORS])(struct Node *, int, double *) = {
  _differentiate_sum,
  _differentiate_product,
  _differentiate_subtraction,
  _differentiate_division,
  _differentiate_power,
  _differentiate_neg,
  _differentiate_sqrt,
  _differentiate_exp,
  _differentiate_log,
  _differentiate_sin,
  _differentiate_cos,
  _differentiate_tan,
};

int differentiate(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  switch(expr.type){
    case CONST_NODE:
      return _differentiate_constant(expr, wrt, values, nvar);
    case VAR_NODE:
      return _differentiate_variable(expr, wrt, values, nvar);
    case OP_NODE:
      return _differentiate_expression(expr, wrt, values, nvar);
  }
}

int _differentiate_constant(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  for (int i=0; i<nvar; i++){
    values[0] = 0.0;
  }
  return nvar;
}

int _differentiate_variable(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
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

int _differentiate_expression(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  // expr = f(arg1, arg2, ...)
  // df/d(wrt) = f'(arg1(wrt), arg2(wrt), ...) * (d(arg1)/d(wrt) + d(arg2)/d(wrt) + ...)
  //
  // TODO: Branch on operator to evaluate local derivative
  // TODO: This should be a vector in the space of arguments
  double * deriv_op = malloc(sizeof(double) * expr.data.expr->nargs);
  // Evaluate the derivative of the operator. This is a vector of multipliers
  // for the derivatives of each argument.
  DIFFERENTIATE_OP[expr.data.expr->op](expr.data.expr->args, expr.data.expr->nargs, deriv_op);

  for (int i=0; i<expr.data.expr->nargs; i++){
    // This is inefficient. I allocate an array of size nvar for every
    // argument of every expression.
    // This is forward differentiation. But presumably I could store these
    // as vectors in the space of `wrt` variables?
    double * arg_values = malloc(nvar * sizeof(double));
    for (int j=0; j<nvar; j++){arg_values[j] = 0.0;}

    differentiate(expr.data.expr->args[i], wrt, arg_values, nvar);

    for (int j=0; j<nvar; j++){
      values[j] += deriv_op[i] * arg_values[j];
    }

    free(arg_values);
  }
  free(deriv_op);
  return 0;
}

int _differentiate_sum(struct Node * args, int nargs, double * deriv){
  // NOTE: OperatorNodes are not evaluated here. If I add side-effects to evaluate,
  // need to make sure this happens in this case.
  for (int j=0; j<nargs; j++){
    deriv[j] = 1.0;
  }
  return 0;
}

int _differentiate_product(struct Node * args, int nargs, double * deriv){
  for (int j=0; j<nargs; j++){
    // TODO: Re-use expression values that have been previously evaluated
    deriv[j] = 1.0;
    for (int jj=0; jj<nargs; jj++){
      if (j != jj){
        deriv[j] *= evaluate(args[jj]);
      }
    }
  }
  return 0;
}

int _differentiate_subtraction(struct Node * args, int nargs, double * deriv){
  if (nargs != 2){
    printf("ERROR: Wrong number of nodes for subtraction node\n");
    exit(-1);
  }
  deriv[0] = 1.0;
  deriv[1] = -1.0;
  return 0;
}

// NOTE that I could malloc deriv and return it from these functions.
int _differentiate_division(struct Node * args, int nargs, double * deriv){
  if (nargs != 2){
    printf("ERROR: Wrong number of arguments for subtraction node\n");
    exit(-1);
  }
  double numerator = evaluate(args[0]);
  double denominator = evaluate(args[1]);
  if (denominator == 0.0){
    printf("ERROR: Evaluating derivative with denominator of zero\n");
    char buff[82];
    to_string(buff, 82, args[1]);
    printf("Expression: %s\n", buff);
    exit(-1);
  }
  deriv[0] = 1.0 / denominator;
  deriv[1] = -1.0 * numerator / (denominator * denominator);
  return 0;
}

int _differentiate_power(struct Node * args, int nargs, double * deriv){
  if (nargs != 2){
    printf("ERROR: Wrong number of arguments for power node\n");
  }
  double base = evaluate(args[0]);
  double exponent = evaluate(args[1]);
  deriv[0] = exponent * pow(base, (exponent - 1.0));
  if (base == 0.0){
    deriv[1] = 0.0;
  } else if (base == 1.0){
    deriv[1] = 0.0;
  } else{
    deriv[1] = pow(base, exponent) / log(base);
  }
  return 0;
}

int _differentiate_neg(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = -1;
  return 0;
}

int _differentiate_sqrt(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  double arg = evaluate(args[0]);
  if (arg < 0.0){
    printf("ERROR: Evaluating square root of negative number\n");
    char buff[82];
    to_string(buff, 82, args[1]);
    printf("Expression: %s\n", buff);
    exit(-1);
  }
  deriv[0] = 1.0 / (2.0 * sqrt(arg));
  return 0;
}

int _differentiate_exp(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = exp(evaluate(args[0]));
  return 0;
}

int _differentiate_log(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  double arg = evaluate(args[0]);
  if (arg <= 0.0){
    // TODO: Should probably just return NaN here and let the
    // caller handle it.
    printf("ERROR: Evaluating log of nonpositive number\n");
    char buff[82];
    to_string(buff, 82, args[1]);
    printf("Expression: %s\n", buff);
    exit(-1);
  }
  deriv[0] = 1.0 / arg;
  return 0;
}

int _differentiate_sin(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = cos(evaluate(args[0]));
  return 0;
}

int _differentiate_cos(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = -sin(evaluate(args[0]));
  return 0;
}

int _differentiate_tan(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = 1.0 / pow(cos(evaluate(args[0])), 2.0);
  return 0;
}

/*
 * Differentiate an expression and return a single CSR matrix
 * (here, a sparse vector).
 */
struct CSRMatrix differentiate_expression(struct Node expr, int nvar){
  int nrow = 1;
  int eidx = 0;

  int * in_expr = malloc(sizeof(int) * nvar);
  double * deriv_values = malloc(sizeof(double) * nvar);
  for (int i=0; i<nvar; i++){ in_expr[i] = -1; }

  struct VarListNode * varlist = NULL;
  int nnz = identify_variables(expr, eidx, in_expr, nvar, &varlist);
  free(in_expr);

  int ret = differentiate(expr, varlist, deriv_values, nvar);

  // NOTE: These arrays will have to be freed later.
  int * indices = malloc(sizeof(int) * nnz);
  int * indptr = malloc(sizeof(int) * 2);
  double * csr_values = malloc(sizeof(double) * nnz);

  // We only have one row, so indptr is trivial
  indptr[0] = 0;
  indptr[1] = nnz;

  struct Variable * vararray = malloc(sizeof(struct Variable) * nnz);
  struct VarListNode * tempnode = varlist;
  for (int i=0; i<nnz; i++){
    vararray[i] = *(tempnode->variable);
    tempnode = tempnode->next;
    indices[i] = vararray[i].index;
    csr_values[i] = deriv_values[indices[i]];
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
