#ifndef BASIC_GRAPH_
#define BASIC_GRAPH_

#include "utility.h"
#include <string>
#include <ctime>
#include <map>
#include <vector>

const int kBinary = 1 << 0;
const int kIndex = 1 << 1;
const int kIn = 1 << 2;
const int kOut = 1 << 3;
const int kIntersect = 1 << 4;
const int kUnion = 1 << 5;
const int kMapping = 1 << 6;
const int kALL = ( 1 << 7 ) - 1;

static double RandUnity(){  return rand() * 1.0 / RAND_MAX; }

enum GraphType{
  OUT,
  IN,
  INTERSECTION,
  UNION,
  BAD
};

static std::string CONVERT_TO_STRING(GraphType type){
  switch (type){
  case OUT:
    return "out";
  case IN:
    return "in";
  case INTERSECTION:
    return "inter";
  case UNION:
    return "union";
  default:
    return "NONE";
  }
}

static GraphType GetDualGraphType(GraphType type){
  switch (type){
  case OUT:
    return IN;
  case IN:
    return OUT;
  case INTERSECTION:
    return UNION;
  case UNION:
    return INTERSECTION;
  default:
    return BAD;
  }
}

class BasicGraph{

 public:

  explicit BasicGraph(bool verbose = 0);
  explicit BasicGraph(const std::string& base_path, bool verbose = 0);
  //explicit BasicGraph(const BasicGraph& graph, const std::vector<int> subgraph_ids, bool raw = 1, bool verbose_ = 0);
  ~BasicGraph();

  int GetNumberVertex() const { return number_vertex_; }

  int GetNumerEdges(GraphType type = OUT) const {
    return graphs_[ static_cast<int>(type) ].number_edges;
  }

  bool IsVerbose() const { return verbose_; }
  void SetVerbose(bool verbose) { verbose_ = verbose; }
  
  void Clear();
  void Load(const std::string& base_path); 
  void Save(const std::string& base_path, const int parameter = kALL) const;
  void LoadMapping(const std::string &base_string);
  void SaveMapping(const std::string& base_string, const int parameter) const;
  void GenerateRMATGraph(int n_scale=10, double edge_factor=0.9, double a=0.60, double b=0.20, double c=0.15);

  //mapping
  //^
  void Dump(GraphType type = OUT, int range = 10)const;

  int GetDegree(int vertex_id, GraphType type = OUT) const;
  std::vector<int> GetNeighbors(int vertex_id, GraphType type = OUT) const;
  std::pair<const int*, const int*> GetNeighborsIterators(int vertex_id, GraphType type = OUT) const;

  int FromRawId(int raw_id) const; 
  int ToRawId(int id) const;
  std::vector<int> FromRawIds(const std::vector<int> &raw_ids) const;
  std::vector<int> ToRawIds(const std::vector<int> &ids) const;

 private:
  
  struct BasicGraphImpl{

  BasicGraphImpl(): generated(0){}
    ~BasicGraphImpl(){}
    
    bool generated;
    int number_edges;
    std::vector<int> boundaries;
    std::vector<int> targets;

    void Clear();
  };

  void LoadImpl(const std::string& base_path, const GraphType type, const int parameter);
  void SaveImpl(const std::string& base_path, const GraphType type, const int parameter)const;
  void Generate(GraphType type);
  void Reverse();
  void Intersect();
  void Union();
  void CalcMapping();

  int number_vertex_;
  int number_edges_;

  std::vector<BasicGraphImpl> graphs_;

  bool has_mapping_;
  std::vector<int> to_raw_mapping_;
  std::map<int,int> from_raw_mapping_;
  
  bool verbose_;

};

#endif

