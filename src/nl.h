#include "nl_opcodes.h"

struct NLHeader {
  bool binary; // As opposed to ASCII. Should this be an enum instead?
  int nvar;
  int ncon;
  int nobj;
  int jnnz;
  int gnnz;
  int nexpr;
};

struct NLHeader read_nl_header(FILE * fp);
int read_nl_variables(FILE * fp, struct Variable * variables, int nvar);
int read_nl_constraints(FILE * fp, struct Node * constraint_expressions, int ncon, struct Variable * variables, int nvar);
struct Node read_nl_expression(FILE * fp, struct Variable * variables, int nvar);
struct Node _read_nl_constant(FILE * fp, char * line, struct Variable * variables, int nvar);
struct Node _read_nl_variable(FILE * fp, char * line, struct Variable * variables, int nvar);
struct Node _read_nl_expression(FILE * fp, char * line, struct Variable * variables, int nvar);
int read_nl_header_line(FILE * fp, int * data, int ndata);
int read_to_eol(FILE * fp);

int read_to_eol(FILE * fp){
  int c = fgetc(fp);
  int count = 0;
  // TODO: Recognize carriage return as well?
  while (c != '\n' && c != EOF){
    c = fgetc(fp);
    count += 1;
  }
  return count;
}

// Note that data must be an array of length (at least) 6.
// Only the first ndata entries will be populated.
int read_nl_header_line(FILE * fp, int * data, int ndata){
  // Construct a format string to match the number of integers
  // we expect in this line of the file.
  // The format string must match the number of integers so we don't
  // absorb EOL whitespace. (If we absorb EOL whitespace, read_to_eol
  // will read the next line.)
  //
  // Construct a format string such as "%d %d %d"
  // TODO: Assert ndata <= 6
  int fmtlen = 3 * ndata - 1;
  char * fmt = malloc(fmtlen * sizeof(char));
  for (int i = 0; i < ndata; i++){
    int fmtidx = 3 * i;
    fmt[fmtidx] = '%';
    fmt[fmtidx+1] = 'd';
    if (i != ndata - 1){
      fmt[fmtidx+2] = ' ';
    }
  }

  // Read the specified number of integers from the file into the first
  // ndata entries of data.
  fscanf(fp, fmt, &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
  free(fmt);
  // Read and scrap any remaining characters in the line (including '\n')
  read_to_eol(fp);
  return 0;
}

struct NLHeader read_nl_header(FILE * fp){
  int cint = fgetc(fp);
  // Is this the right way to convert the return code to a character
  char c;
  if (cint != EOF){
    c = cint;
  }
  else{
    printf("ERROR: Empty nl file\n");
    exit(-1);
  }

  bool binary;
  if (c == 'g'){
    binary = false;
  }else if (c == 'b'){
    binary = true;
  }else{
    printf("ERROR: Unrecognized binary/ASCII indicator char.\n");
    printf("       'g' or 'b' was expected.\n");
    exit(-1);
  }

  struct NLHeader header = {.binary = binary};

  int linecount = 1;
  // Allocate 6 integers, which is the maximum amount of information in a line
  int data[6] = {-1, -1, -1, -1, -1, -1};

  // Line 1: Not sure what this means
  read_nl_header_line(fp, data, 4);
  //printf("Line %2d: %d %d %d %d\n", linecount, data[0], data[1], data[2], data[3]);
  linecount += 1;

  // Line 2: N. vars/cons/etc.
  read_nl_header_line(fp, data, 5);
  //printf("Line %2d: %d %d %d %d %d\n", linecount, data[0], data[1], data[2], data[3], data[4]);
  linecount += 1;

  header.nvar = data[0];
  header.ncon = data[1];
  header.nobj = data[2];

  // Line 3: N. nonlinear cons/objs. We don't use this info
  read_nl_header_line(fp, data, 6);
  //printf("Line %2d: %d %d %d %d %d %d\n", linecount, data[0], data[1], data[2], data[3], data[4], data[5]);
  linecount += 1;

  // Line 4: N. network constraints. We don't use this.
  read_nl_header_line(fp, data, 2);
  //printf("Line %2d: %d %d\n", linecount, data[0], data[1]);
  linecount += 1;

  // Line 5: N. nonlinear vars. We don't use this.
  read_nl_header_line(fp, data, 3);
  //printf("Line %2d: %d %d %d\n", linecount, data[0], data[1], data[2]);
  linecount += 1;

  // Line 6: Linear network vars. We don't use this.
  read_nl_header_line(fp, data, 4);
  //printf("Line %2d: %d %d %d %d\n", linecount, data[0], data[1], data[2], data[3]);
  linecount += 1;

  // Line 7: Discrete variables. We don't use this (for now).
  read_nl_header_line(fp, data, 5);
  //printf("Line %2d: %d %d %d %d %d\n", linecount, data[0], data[1], data[2], data[3], data[4]);
  linecount += 1;

  // Line 8: Nonzeros
  read_nl_header_line(fp, data, 2);
  //printf("Line %2d: %d %d\n", linecount, data[0], data[1]);
  linecount += 1;

  header.jnnz = data[0];
  header.gnnz = data[1];

  // Line 9: Max name lengths. We don't use this (for now).
  read_nl_header_line(fp, data, 2);
  //printf("Line %2d: %d %d\n", linecount, data[0], data[1]);
  linecount += 1;

  // Line 10: Common subexpressions
  read_nl_header_line(fp, data, 5);
  //printf("Line %2d: %d %d %d %d %d\n", linecount, data[0], data[1], data[2], data[3], data[4]);
  linecount += 1;

  // These data are of a partition of subexpressions. We just sum them for now.
  header.nexpr = data[0] + data[1] + data[2] + data[3] + data[4];

  return header;
}

const int MAX_LINELEN = 82;

int read_nl_variables(FILE * fp, struct Variable * variables, int nvar){
  // Read the first 10 lines (the header)
  for (int i = 0; i < 10; i++){read_to_eol(fp);}

  // Read lines until we find the primal variable initialization section
  // Note that this linelen, while reasonable for individual lines of the
  // nl file, may not be reasonable if we allow comments.
  // TODO: Allow arbitrary-length lines in the nl file body.
  char line[MAX_LINELEN];
  fgets(line, MAX_LINELEN, fp);
  while (line[0] != 'x' && line[0] != EOF){
    fgets(line, MAX_LINELEN, fp);
  }

  int segment_nvar;
  // Parse the line we just read (offset to start after the 'x' character)
  // into the int variable segment_nvar
  sscanf(line+1, "%d", &segment_nvar);
  // TODO: Assert nvar == segment_nvar

  for (int i = 0; i < nvar; i++){
    int vidx;
    double value;
    fscanf(fp, "%d %lf", &vidx, &value);
    // TODO: assert vidx < nvar
    read_to_eol(fp);
    variables[vidx].value = value;
  }

  // Unclear what the return value from this function should be
  return 0;
}

int read_nl_constraints(
  FILE * fp,
  struct Node * constraint_expressions,
  int ncon,
  struct Variable * variables,
  int nvar
){
  // TODO: Read the linear part of each constraint
  //
  // Read first 10 lines (the header)
  for (int i = 0; i < 10; i++){read_to_eol(fp);}

  char line[MAX_LINELEN];
  fgets(line, MAX_LINELEN, fp);

  // This is a flag that determines whether we skip a line read, i.e.
  // our line buffer already contains the information we need to start
  // processing the constraint
  int already_encountered_constraint = 0;

  // Note that we still enter the loop if we just read the last line of
  // the file, as long as we haven't hit EOF yet.
  while (!feof(fp)){
    if (line[0] == 'C'){
      // We have encountered a constraint
      // Read second character of line as constraint index
      //
      // TODO: Make sure, at the end of this loop, that every constraint
      // has been accounted for.
      int cidx;
      sscanf(line+1, "%d", &cidx);

      // TODO: Potentially pass in the line number so we can print reasonable
      // debugging information.
      struct Node exprnode = read_nl_expression(fp, variables, nvar);

      // Populate the constraint expression
      constraint_expressions[cidx] = exprnode;
    }

    if (!already_encountered_constraint){
      // If a constraint's expression is empty, we will read the next C line
      // while trying to read the expression. If this has happened, we need to
      // skip this line read.
      fgets(line, MAX_LINELEN, fp);
    }
    already_encountered_constraint = 0;

  }
  return 0;
}

/*
 * read_nl_expression
 *
 * Construct a Node corresponding to the expression defined by the file in
 * nl-Polish prefix notation.
 *
 * We assume we are in the process of reading the file, and the next character
 * is the start of the root of the expression.
 *
 * This function allocates the expression pointed to by the returned node
 * (unless it is a variable or constant node) and all non-leaf subexpressions.
 *
 */
struct Node read_nl_expression(
  FILE * fp,
  struct Variable * variables,
  int nvar
){
  char line[MAX_LINELEN];
  fgets(line, MAX_LINELEN, fp);

  switch(line[0]){
    case 'n':
      return _read_nl_constant(fp, line, variables, nvar);
    case 'v':
      return _read_nl_variable(fp, line, variables, nvar);
    case 'o':
      return _read_nl_expression(fp, line, variables, nvar);
    default:
      printf("ERROR: %s", line);
      printf("ERROR: ^ Unexpected character %c\n", line[0]);
      exit(-1);
  }
}

struct Node _read_nl_constant(FILE * fp, char * line, struct Variable * variables, int nvar){
  double val;
  sscanf(line+1, "%lf", &val);
  union NodeData nodedata = {.value=val};
  struct Node node = {CONST_NODE, nodedata};
  return node;
}

struct Node _read_nl_variable(FILE * fp, char * line, struct Variable * variables, int nvar){
  int vidx;
  sscanf(line+1, "%d", &vidx);
  union NodeData nodedata = {.var=&(variables[vidx])};
  struct Node node = {VAR_NODE, nodedata};
  return node;
}

struct Node _read_nl_expression(FILE * fp, char * line, struct Variable * variables, int nvar){
  int opnum;
  sscanf(line+1, "%d", &opnum);
  int optype = OP_LOOKUP[opnum];
  // TODO: Check that looking up this operator does not yield -1, our convention
  // for a not-supported operator. Note that we don't attempt to guard against
  // an out-of-bounds array access in OP_LOOKUP
  if (optype == -1){
    printf("ERROR: Unsupported operator code o%d\n", opnum);
    exit(-1);
  }

  // Look up the number of arguments expected by this operator
  int nargs = OPERATOR_DATA[optype].nargs;
  struct Node * args = malloc(nargs*sizeof(struct Node));
  for (int i=0; i<nargs; i++){
    // Read argument expressions/nodes from nl file
    args[i] = read_nl_expression(fp, variables, nvar);
  }

  // Heap-allocate this expression so we can access it outside of this function
  struct OperatorNode * expr = malloc(sizeof(struct OperatorNode));
  expr->op = optype;
  expr->nargs = nargs;
  expr->args = args;

  union NodeData nodedata = {.expr=expr};
  struct Node node = {OP_NODE, nodedata};
  return node;
}
