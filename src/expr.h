#include "variable.h"

// Forward-declare structs for use in function prototypes
struct Node;
struct OperatorNode;

const int N_OPERATORS = 12;
enum OperatorType {
  SUM,
  PRODUCT,
  SUBTRACTION,
  DIVISION,
  POWER,
  NEG,
  SQRT,
  EXP,
  LOG,
  SIN,
  COS,
  TAN,
};

struct OperatorData {
  enum OperatorType optype;
  int nargs;
  char * opstring;
};

// Global array of operator data?
// We will then use OperatorType to look into this array to determine whether
// our operators are binary/unary/n-ary, and what symbol(s) should be used to
// represent them.
//
// Note that the order here must be the same as the order in OperatorType above
const struct OperatorData OPERATOR_DATA[N_OPERATORS] = {
  {SUM,         2, "+"},
  {PRODUCT,     2, "*"},
  {SUBTRACTION, 2, "-"},
  {DIVISION,    2, "/"},
  {POWER,       2, "^"},
  // NOTE: If I had some data structure that contained the C functions for
  // these unary operators, I could implement just a single evaluate_unary
  // function.
  {NEG,         1, "-"},
  {SQRT,        1, "sqrt"},
  {EXP,         1, "exp"},
  {LOG,         1, "log"},
  {SIN,         1, "sin"},
  {COS,         1, "cos"},
  {TAN,         1, "tan"},
};

// Function forward declarations
double evaluate(struct Node expr);
double _evaluate_sum_node(int nargs, struct Node * args);
double _evaluate_product_node(int nargs, struct Node * args);
double _evaluate_subtraction_node(int nargs, struct Node * args);
double _evaluate_division_node(int nargs, struct Node * args);
double _evaluate_power_node(int nargs, struct Node * args);
double _evaluate_neg_node(int nargs, struct Node * args);
double _evaluate_sqrt_node(int nargs, struct Node * args);
double _evaluate_exp_node(int nargs, struct Node * args);
double _evaluate_log_node(int nargs, struct Node * args);
double _evaluate_sin_node(int nargs, struct Node * args);
double _evaluate_cos_node(int nargs, struct Node * args);
double _evaluate_tan_node(int nargs, struct Node * args);

double (* OP_EVALUATOR[N_OPERATORS])(int, struct Node *) = {
  _evaluate_sum_node,
  _evaluate_product_node,
  _evaluate_subtraction_node,
  _evaluate_division_node,
  _evaluate_power_node,
  _evaluate_sqrt_node,
  _evaluate_neg_node,
  _evaluate_exp_node,
  _evaluate_log_node,
  _evaluate_sin_node,
  _evaluate_cos_node,
  _evaluate_tan_node,
};

int to_string(char * buffer, int bsize, struct Node expr);
int _expr_to_string(char * buffer, int bsize, struct OperatorNode expr);

enum NodeType {
  CONST_NODE,
  VAR_NODE,
  OP_NODE,
};

struct OperatorNode{
  enum OperatorType op;
  int nargs;
  // Array of nodes. Each node contains a pointer to its underlying
  // variable or value.
  struct Node *args;
  double value;
};

union NodeData {
  double value;
  // Pointers to variable and expression so the same variable and expression
  // can be used in multiple locations.
  // This just enforces that the same variable and expression can be used in multiple
  // nodes. Then I further enforce that the same nodes can be used in multiple
  // OperatorNodes. But I probably want just a single node for each variable.
  //
  // Each leaf node multiplies the amount of memory needed for the variable/constant,
  // so I want to reduce the number of nodes I use.
  // I just need to make sure I operate on nodes, rather than variables (or constants).
  //
  // Or I could use an actual array of Nodes in the OperatorNode, and have each
  // node simply point to the variable...
  // With pointers to nodes, however, I can have "parameter nodes". It is unclear
  // whether I want this.
  //
  // It may be useful for a user to compare their variable to the variable
  // stored in a node. In which case they would want the node to point to the
  // variable.
  struct Variable *var;
  struct OperatorNode *expr;
};

// How would a common "node" struct work?
// - node contains value, a "variable flag", and ...
//   operator and pointers to arguments?
// - UnaryNode, BinaryNode, NaryNode? These could save memory and reduce
//   the amount of redirection.
//
// It seems a little roundabout to have both a value and arguments.
// Could I encode with an integer? Integer is the variable index,
// which can be used to access the value. But what does an integer
// refer to for the OperatorNode?
struct Node {
  enum NodeType type;
  union NodeData data;
  // We need adjoints for variables and operators, so we store them
  // on the node. We'll also compute them for constants, but we won't use
  // these for anything.
  double adjoint;
};

// Evaluation functions
double evaluate(struct Node expr){
  switch(expr.type){
    case CONST_NODE:
      return expr.data.value;
    case VAR_NODE:
      return expr.data.var->value;
    case OP_NODE:
      return OP_EVALUATOR[expr.data.expr->op](expr.data.expr->nargs, expr.data.expr->args);
  }
}

// Note that sum and product nodes are already implemented as n-ary
double _evaluate_sum_node(int nargs, struct Node * args){
  double sum = 0.0;
  for (int i = 0; i < nargs; i++){
    sum += evaluate(args[i]);
  }
  return sum;
}

double _evaluate_product_node(int nargs, struct Node * args){
  double prod = 1.0;
  for (int i = 0; i < nargs; i ++){
    prod *= evaluate(args[i]);
  }
  return prod;
}

double _evaluate_subtraction_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 2
  return evaluate(args[0]) - evaluate(args[1]);
}

double _evaluate_division_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 2
  return evaluate(args[0]) / evaluate(args[1]);
}

double _evaluate_power_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 2
  return pow(evaluate(args[0]), evaluate(args[1]));
}

double _evaluate_neg_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  return -evaluate(args[0]);
}

double _evaluate_sqrt_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  return sqrt(evaluate(args[0]));
}

double _evaluate_exp_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  // TODO: Handle overflow. Should we check for HUGE_VAL and return
  // NaN instead?
  return exp(evaluate(args[0]));
}

double _evaluate_log_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  // TODO: Handle evaluation errors
  return log(evaluate(args[0]));
}

double _evaluate_sin_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  return sin(evaluate(args[0]));
}

double _evaluate_cos_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  return cos(evaluate(args[0]));
}

double _evaluate_tan_node(int nargs, struct Node * args){
  // TODO: Assert nargs == 1
  return tan(evaluate(args[0]));
}

// Printing functions
int to_string(char * buffer, int bsize, struct Node expr){
  switch(expr.type){
    case CONST_NODE:
      return snprintf(buffer, bsize, "%1.3f", expr.data.value);
    case VAR_NODE:
      return snprintf(buffer, bsize, "v%d", expr.data.var->index);
    case OP_NODE:
      return _expr_to_string(buffer, bsize, *expr.data.expr);
  }
}

int _expr_to_string(char * buffer, int bsize, struct OperatorNode expr){
  int nargs = expr.nargs;
  struct Node * args = expr.args;

  //char opchar = OP_SYMBOL[expr.op];

  char * opstring = OPERATOR_DATA[expr.op].opstring;

  char ** substrings = malloc(nargs * sizeof(char*));
  // This is the offset at which we will insert the next operand substring
  int bidx = 0;

  for (int i = 0; i < nargs; i++){
    int remaining_space = bsize - bidx;
    // Allow 5 characters for parentheses and operator strings
    // This is just the maximum we will need to use for this operand, not
    // the maximum we will use total. It is okay to over-estimate the space
    // we will use, as we only increase the offset (bidx) by the number of
    // characters we actually use.
    substrings[i] = malloc((remaining_space - 5) * sizeof(char));
    to_string(substrings[i], remaining_space - 5, args[i]);
    // TODO: Check that substr_len <= remaining_space-5. Otherwise, the substring
    // we tried to write got cut off and we will silently print an incorrect result.
    // In this case, we can either fail, or print something like (v1 + 2 + ...).
    int ret;

    if (i == 0 && nargs == 1){
      // Unary operator. We enclose the argument substring in both parentheses
      // TODO: Depending on operator precedence, parentheses may not be necessary
      ret = snprintf(buffer + bidx, bsize - bidx, "%s(%s)", opstring, substrings[i]);
    }else if (i == 0){
      // First operand of a 2-or-more-ary operator. Add left paren.
      ret = snprintf(buffer + bidx, bsize - bidx, "(%s", substrings[i]);
    }else if (i == nargs - 1){
      // Last operand of 2-or-more-ary operator. Add operator symbol and right paren.
      ret = snprintf(buffer + bidx, bsize - bidx, " %s %s)", opstring, substrings[i]);
    }else{
      // Internal operand of 2-or-more-ary operator. Add operator symbol.
      ret = snprintf(buffer + bidx, bsize - bidx, " %s %s", opstring, substrings[i]);
    }
    // Adjust the offset by the number of characters we just wrote.
    bidx += ret;

    free(substrings[i]);
  }

  free(substrings);

  return bidx;
}

void free_expression(struct Node node);
void _free_expression(struct OperatorNode * node);

void free_expression(struct Node node){
  switch(node.type){
    case CONST_NODE:
      return;
    case VAR_NODE:
      // TODO: Option to free variable nodes
      return;
    case OP_NODE:
      return _free_expression(node.data.expr);
  }
}

void _free_expression(struct OperatorNode * expr){
  for (int i=0; i < expr->nargs; i++){
    // Free all subexpressions, if necessary
    free_expression(expr->args[i]);
  }
  // Free the expression itself
  free(expr);
}
