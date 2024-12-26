struct Variable {
  // For "self-sufficient" printing, a variable needs to know some "global
  // information" about itself. An index seems more versatile than a name.
  int index;
  double value;
};
