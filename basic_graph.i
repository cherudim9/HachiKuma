%module basic_graph

%{
  #include "basic_graph.h"
%}

%include "std_vector.i"

namespace std{
  %template(vector_int) vector<int>;
   %template(vector_ii) vector<vector<int> >;
}

%include "basic_graph.h"
