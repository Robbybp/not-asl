#include <assert.h>

int _diff_sum(struct Node * args, int nargs, double * deriv);
int _diff_product(struct Node * args, int nargs, double * deriv);
int _diff_subtraction(struct Node * args, int nargs, double * deriv);
int _diff_division(struct Node * args, int nargs, double * deriv);
int _diff_power(struct Node * args, int nargs, double * deriv);
int _diff_neg(struct Node * args, int nargs, double * deriv);
int _diff_sqrt(struct Node * args, int nargs, double * deriv);
int _diff_exp(struct Node * args, int nargs, double * deriv);
int _diff_log(struct Node * args, int nargs, double * deriv);
int _diff_sin(struct Node * args, int nargs, double * deriv);
int _diff_cos(struct Node * args, int nargs, double * deriv);
int _diff_tan(struct Node * args, int nargs, double * deriv);

// N_OPERATORS defined in expr.h
int (* DIFF_OP[N_OPERATORS])(struct Node *, int, double *) = {
  _diff_sum,
  _diff_product,
  _diff_subtraction,
  _diff_division,
  _diff_power,
  _diff_neg,
  _diff_sqrt,
  _diff_exp,
  _diff_log,
  _diff_sin,
  _diff_cos,
  _diff_tan,
};

int _diff_sum(struct Node * args, int nargs, double * deriv){
  // NOTE: OperatorNodes are not evaluated here. If I add side-effects to evaluate,
  // need to make sure this happens in this case.
  for (int j=0; j<nargs; j++){
    deriv[j] = 1.0;
  }
  return 0;
}

int _diff_product(struct Node * args, int nargs, double * deriv){
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

int _diff_subtraction(struct Node * args, int nargs, double * deriv){
  if (nargs != 2){
    printf("ERROR: Wrong number of nodes for subtraction node\n");
    exit(-1);
  }
  deriv[0] = 1.0;
  deriv[1] = -1.0;
  return 0;
}

int _diff_division(struct Node * args, int nargs, double * deriv){
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

int _diff_power(struct Node * args, int nargs, double * deriv){
  if (nargs != 2){
    printf("ERROR: Wrong number of arguments for power node\n");
  }
  double base = evaluate(args[0]);
  double exponent = evaluate(args[1]);
  deriv[0] = exponent * pow(base, (exponent - 1.0));
  if (base == 0.0){
    // TODO: Handle base < 0 somehow?
    deriv[1] = 0.0;
  } else{
    deriv[1] = pow(base, exponent) * log(base);
  }
  return 0;
}

int _diff_neg(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = -1;
  return 0;
}

int _diff_sqrt(struct Node * args, int nargs, double * deriv){
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

int _diff_exp(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = exp(evaluate(args[0]));
  return 0;
}

int _diff_log(struct Node * args, int nargs, double * deriv){
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

int _diff_sin(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = cos(evaluate(args[0]));
  return 0;
}

int _diff_cos(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = -sin(evaluate(args[0]));
  return 0;
}

int _diff_tan(struct Node * args, int nargs, double * deriv){
  assert(nargs == 1);
  deriv[0] = 1.0 / pow(cos(evaluate(args[0])), 2.0);
  return 0;
}
