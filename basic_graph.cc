#include "basic_graph.h"
#include <fstream>

const int kBinaryLoadingRate=65536;

template<class T>
static void LoadBinaryFile(const std::string& path, std::ifstream& stream, int size, std::vector<T> &container, bool verbose){
  mProcess load_process("Load of binary file " + path, size, verbose);
  load_process.Start();
  container.resize(size);
  for(int i=0; i<size; i+=kBinaryLoadingRate){
    stream.read( reinterpret_cast<char*>( container.data() + i ) , sizeof(T) * kBinaryLoadingRate );
    load_process.Update(i);
  }
  load_process.Stop();
}

template<class T>
static void SaveBinaryFile(const std::string& path, std::ofstream& stream, int size, const std::vector<T> &container, bool verbose){
  mProcess save_process("Save of binary file " + path, size, verbose);
  save_process.Start();
  for(int i=0; i<size; i+=kBinaryLoadingRate){
    int t = std::min( kBinaryLoadingRate, size - i );
    stream.write( reinterpret_cast<const 
char*> ( container.data() + i ) , sizeof(T) * t );
    save_process.Update(i);
  }
  save_process.Stop();
}

template<class T>
static void LoadTextFile(const std::string& path, std::ifstream& stream, int size, std::vector<T>& container, bool verbose){
  mProcess load_process("Load of text file " + path, size, verbose);
  load_process.Start();
  container.resize(size);
  for(int i=0; i<size; i++){
    stream >> container[i];
    load_process.Update(i);
  }
  load_process.Stop();
}

template<class T>
static void SaveTextFile(const std::string& path, std::ofstream& stream, int size, const std::vector<T>& container, bool verbose){
  mProcess save_process("Save of text file " + path, size, verbose);
  save_process.Start();
  for(int i=0; i<size; i++){
    stream << container[i] << "\n" ;
    save_process.Update(i);
  }
  save_process.Stop();
}

BasicGraph::BasicGraph(bool verbose):
  number_vertex_(0), number_edges_(0), graphs_(std::vector<BasicGraphImpl>(static_cast<int>(BAD))), has_mapping_(0), verbose_(verbose){
  
}

BasicGraph::BasicGraph(const std::string& base_path, bool verbose):
  BasicGraph(verbose){
  Load(base_path);
}

BasicGraph::~BasicGraph(){

}

void BasicGraph::Clear(){
  number_edges_=0;
  number_vertex_=0;
  has_mapping_=0;
  to_raw_mapping_.clear();
  from_raw_mapping_.clear();
  graphs_ = std::vector<BasicGraphImpl>( static_cast<int>( BAD ) );
}

void BasicGraph::Load(const std::string& base_path){
  Clear();
  
  std::string index_name( base_path + ".ind" );
  std::ifstream index_stream( index_name );
  FilePath::CheckForOpen(index_name, index_stream);
  index_stream >> number_vertex_ >> number_edges_;

  try{
    LoadImpl(base_path, OUT, 0);
    try{
      LoadImpl(base_path, IN, 0);
    } catch(std::runtime_error &e){
      Generate(IN);
    }
    try{
      LoadImpl(base_path, INTERSECTION, 0);
    } catch(std::runtime_error &e){
      Generate(INTERSECTION);
    }
    try{
      LoadImpl(base_path, UNION, 0);
    } catch(std::runtime_error &e){
      Generate(UNION);
    }

  }catch(std::runtime_error &e){

    LoadImpl(base_path, OUT, kBinary);
    try{
      LoadImpl(base_path, IN, kBinary);
    } catch(std::runtime_error &e){
      Generate(IN);
    }
    try{
      LoadImpl(base_path, INTERSECTION, kBinary);
    } catch(std::runtime_error &e){
      Generate(INTERSECTION);
    }
    try{
      LoadImpl(base_path, UNION, kBinary);
    } catch(std::runtime_error &e){
      Generate(UNION);
    }

  }

  try{
    LoadMapping(base_path);
  } catch(std::runtime_error &e){

  }
}

void BasicGraph::Save(const std::string& base_path, const int parameter)const{
  if (parameter & kIndex){
    std::string index_name( base_path + ".ind" );
    FilePath::CheckForExistence(index_name);
    std::ofstream index_stream(index_name);
    FilePath::CheckForCreation(index_name, index_stream);
    index_stream << number_vertex_ << "\n" << number_edges_ << "\n";
  }
  if (parameter & kIn){
    SaveImpl(base_path, IN, parameter);
  }
  if (parameter & kOut){
    SaveImpl(base_path, OUT, parameter);
  }
  if (parameter & kIntersect){
    SaveImpl(base_path, INTERSECTION, parameter);
  }
  if (parameter & kUnion){
    SaveImpl(base_path, UNION, parameter);
  }
  if (parameter & kMapping){
    if (!has_mapping_)
      throw std::runtime_error("No mapping");
    SaveMapping(base_path, parameter);
  }
}

void BasicGraph::CalcMapping(){
  mProcess process("Generation of reverse mapping", number_vertex_, verbose_);
  process.Start();
  from_raw_mapping_.clear();
  for(int i=0; i<number_vertex_; i++){
    from_raw_mapping_[ to_raw_mapping_[i] ] = i;
    process.Update(i);
  }
  process.Stop();
}

void BasicGraph::LoadMapping(const std::string& base_path){
  std::string mapping_file(base_path+".map");
  std::string text_name(mapping_file), bin_name(mapping_file + ".bin");
  bool text_exist=FilePath::Exist(text_name), bin_exist=FilePath::Exist(bin_name);
  if (text_exist && bin_exist){
    throw std::runtime_error("Both text and binary version of mapping exist.");
  }
  if (text_exist){
    has_mapping_=1;
    std::ifstream text_stream(text_name);
    FilePath::CheckForOpen(text_name, text_stream);
    LoadTextFile<int>(text_name, text_stream, number_vertex_, to_raw_mapping_, verbose_);
    CalcMapping();
    return;
  }
  if (bin_exist){
    has_mapping_=1;
    std::ifstream bin_stream(bin_name);
    FilePath::CheckForOpen(bin_name, bin_stream);
    LoadBinaryFile<int>(bin_name, bin_stream, number_vertex_, to_raw_mapping_, verbose_);
    CalcMapping();
    return;
  }
  throw std::runtime_error("No file for mapping.");
}

void BasicGraph::GenerateRMATGraph(int n_scale, double edge_factor, double a, double b, double c){
  srand(time(0));
  
  int n = ( 1 << n_scale );
  int m = static_cast<int> ( n * ( n - 1) * edge_factor );
  double ab=a+b, abc=ab+c;
  std::vector< std::vector<int> > adj_edge(n);
  for(int edge_num=0; edge_num < m; edge_num++){
    int s=0, t=0;
    for(int iter_times=0; iter_times < n_scale; iter_times++){
      double rand_factor=RandUnity();
      if (rand_factor<a){
        //pass
      }else
        if (rand_factor<ab){
          t += ( 1 << ( n_scale - 1 - iter_times ) );
        }else
          if (rand_factor<abc){
            s += ( 1 << ( n_scale - 1 - iter_times ) );
          }else{
            s += ( 1 << ( n_scale - 1 - iter_times ) );
            t += ( 1 << ( n_scale - 1 - iter_times ) );
          }
    }
    adj_edge[s].push_back(t);
  }
  Clear();
  number_vertex_ = n;
  number_edges_ = m;
  BasicGraphImpl& g=graphs_[ static_cast<int> ( OUT ) ];
  g.generated=1;
  g.number_edges = m;
  g.boundaries.clear();
  g.boundaries.resize(n);
  g.targets.resize(m);
  for(int i=0; i<n; i++){
    sort( adj_edge[i].begin(), adj_edge[i].end() );
    adj_edge[i].resize( unique(adj_edge[i].begin(), adj_edge[i].end()) - adj_edge[i].begin());
    int last_bound = ( i ? g.boundaries[i-1] : 0 );
    g.boundaries[i] = last_bound + adj_edge[i].size();
    for(int j=0; j != adj_edge[i].size(); j++)
      g.targets[ last_bound + j ] = adj_edge[i][j];
  }
  Generate(IN);
  Generate(INTERSECTION);
  Generate(UNION);
}

void BasicGraph::SaveMapping(const std::string& base_path, const int parameter)const{
  if (!has_mapping_){
    throw std::runtime_error("No mapping");
  }

  std::string mapping_file(base_path+".map");
  if (parameter & kBinary)
    mapping_file += ".bin";
  FilePath::CheckForExistence(mapping_file);
  std::ofstream mapping_stream(mapping_file);
  FilePath::CheckForCreation(mapping_file, mapping_stream);

  if (parameter & kBinary){
    SaveBinaryFile<int>(mapping_file, mapping_stream, number_vertex_, to_raw_mapping_, verbose_);
  }else{
    SaveTextFile<int>(mapping_file, mapping_stream, number_vertex_, to_raw_mapping_, verbose_);
  }
}

void BasicGraph::Dump(GraphType type, int range)const{
  std::cout << "n = " << number_vertex_ << ", e = " << number_edges_ << std::endl;
  for(int i = 0; i < number_vertex_; i++){
    if (i == range && range > 0)
      break;
    int d=GetDegree(i, type);
    std::cout << "v" << i;
    if (has_mapping_)
      std::cout << "(raw:" << ToRawId(i) << ")" << std::endl;
    std::cout << "d" << d << " [";
    auto nei=GetNeighbors(i, type);
    for(int j = 0; j != d; j++){
      if (j == range && range > 0)
        break;
      std::cout << nei[j] << ", ";
    }
    if (range > 0 && range < d)
      std::cout << "...";
    std::cout << "]" << std::endl;
  }
  if (range > 0 && range < number_vertex_)
    std::cout << "...";
  std::cout << std::endl; 
}

int BasicGraph::GetDegree(int vertex_id, GraphType type)const{
  const BasicGraphImpl &g = graphs_[static_cast<int>(type)];
  if (vertex_id == 0)
    return g.boundaries[0];
  else
    return g.boundaries[vertex_id] - g.boundaries[vertex_id-1];
}

std::vector<int> BasicGraph::GetNeighbors(int vertex_id, GraphType type)const{
  const BasicGraphImpl &g = graphs_[static_cast<int>(type)];
  std::vector<int> ret;
  int start= vertex_id ? g.boundaries[vertex_id-1] : 0;
  for(int j = start ; j < g.boundaries[vertex_id]; j++)
    ret.push_back(g.targets[j]);
  return ret;
}

std::pair<const int*, const int*>  BasicGraph::GetNeighborsIterators(int vertex_id, GraphType type)const{
  const BasicGraphImpl &g = graphs_[static_cast<int>(type)];
  std::vector<int> ret;
  int start= vertex_id ? g.boundaries[vertex_id-1] : 0;
  return std::make_pair( g.targets.data() + start, g.targets.data() + g.boundaries[vertex_id] );
}

int BasicGraph::FromRawId(int raw_id)const{
  if (!has_mapping_)
    return -1;
  auto tar=from_raw_mapping_.find(raw_id);
  if (tar == from_raw_mapping_.end())
    return -1;
  return tar->second;
}

int BasicGraph::ToRawId(int id)const{
  if (id<0 || id>=number_vertex_)
    return -1;
  return to_raw_mapping_[id];
}

std::vector<int> BasicGraph::FromRawIds(const std::vector<int> &raw_ids)const{
  std::vector<int> ret;
  for(const auto& i: raw_ids)
    ret.push_back(FromRawId(i));
  return ret;
}

std::vector<int> BasicGraph::ToRawIds(const std::vector<int>& ids)const{
  std::vector<int> ret;
  for(const auto& i: ids)
    ret.push_back(ToRawId(i));
  return ret;
}

void BasicGraph::LoadImpl(const std::string& base_path, const GraphType type, const int parameter){
  mProcess loadimpl_process("loading impl in "+base_path+" for type "+CONVERT_TO_STRING(type), 1, verbose_);
  loadimpl_process.Start();
  BasicGraphImpl &g=graphs_[ static_cast<int>(type) ];
  std::string base_name(base_path+".imp_" + CONVERT_TO_STRING(type));
  if (parameter & kBinary)
    base_name += "_bin";
  
  std::string index_name( base_name + ".ind" );
  std::string bound_name( base_name + ".bou" );
  std::string target_name( base_name + ".tar" );
  std::ifstream index_stream(index_name);
  std::ifstream bound_stream(bound_name);
  std::ifstream target_stream(target_name);
  std::vector<int> index_vector;

  FilePath::CheckForOpen(index_name, index_stream);
  FilePath::CheckForOpen(bound_name, bound_stream);
  FilePath::CheckForOpen(target_name, target_stream);

  if (parameter & kBinary){
    //the implementation was stored in binary files
    LoadBinaryFile<int>(index_name, index_stream, 2, index_vector, verbose_);
    g.number_edges=index_vector[1];
    g.generated=1;
    LoadBinaryFile<int>(bound_name, bound_stream, number_vertex_, g.boundaries, verbose_);
    LoadBinaryFile<int>(target_name, target_stream, g.number_edges, g.targets, verbose_);
  }else{
    //the implementation was stored in text files
    LoadTextFile<int>(index_name, index_stream, 2, index_vector, verbose_);
    g.generated=1;
    g.number_edges=index_vector[1];
    LoadTextFile<int>(bound_name, bound_stream, number_vertex_, g.boundaries, verbose_);
    LoadTextFile<int>(target_name, target_stream, g.number_edges, g.targets, verbose_);    
  }    
  loadimpl_process.Stop();
}

void BasicGraph::SaveImpl(const std::string& base_path, const GraphType type, const int parameter)const{
  const BasicGraphImpl &g=graphs_[ static_cast<int>(type) ];
  std::string base_name(base_path + ".imp_" + CONVERT_TO_STRING(type));
  if (parameter & kBinary)
    base_name += "_bin";

  std::string index_name( base_name + ".ind" );
  FilePath::CheckForExistence(index_name);
  std::string bound_name( base_name + ".bou" );
  FilePath::CheckForExistence(bound_name);
  std::string target_name( base_name + ".tar" );
  FilePath::CheckForExistence(target_name);

  std::ofstream index_stream(index_name);
  FilePath::CheckForCreation(index_name, index_stream);
  std::ofstream bound_stream(bound_name);
  FilePath::CheckForCreation(bound_name, bound_stream);
  std::ofstream target_stream(target_name);
  FilePath::CheckForCreation(target_name, target_stream);

  std::vector<int> index_vector(2);
  index_vector[0]=number_vertex_; index_vector[1]=g.number_edges;

  if (parameter & kBinary){
    SaveBinaryFile<int>(index_name, index_stream, 2, index_vector, verbose_);
    SaveBinaryFile<int>(bound_name, bound_stream, number_vertex_, g.boundaries, verbose_);
    SaveBinaryFile<int>(target_name, target_stream, g.number_edges, g.targets, verbose_);
  }else{
    SaveTextFile<int>(index_name, index_stream, 2, index_vector, verbose_);
    SaveTextFile<int>(bound_name, bound_stream, number_vertex_, g.boundaries, verbose_);    
    SaveTextFile<int>(target_name, target_stream, g.number_edges, g.targets, verbose_);    
  }
}

void BasicGraph::Generate(GraphType type){
  if (graphs_[static_cast<int>(type)].generated)
    return;
  switch (type){
  case OUT:
    //do nothing
    break;
  case IN:
    Reverse();
    break;
  case INTERSECTION:
    Intersect();
    break;
  case UNION:
    Union();
    break;
  default:
    break;
  }
}

void BasicGraph::BasicGraphImpl::Clear(){
  generated=0;
  number_edges=0;
  boundaries.clear();
  targets.clear();
}

void BasicGraph::Reverse(){
  mProcess reverse_process("Reverse graph generation", number_vertex_, verbose_, 1000);
  reverse_process.Start();
  BasicGraphImpl& origin=graphs_[static_cast<int>(OUT)];
  BasicGraphImpl& derived=graphs_[static_cast<int>(IN)];
  if (derived.generated)
    return;
  derived.Clear();
  derived.generated=1;
  derived.number_edges=origin.number_edges;
  derived.boundaries.resize(number_vertex_);
  derived.targets.resize(origin.number_edges);

  for(auto i: origin.targets){
    derived.boundaries[i]++;
  }
  for(int i=1; i<number_vertex_; i++)
    derived.boundaries[i]+=derived.boundaries[i-1];

  int j=0;
  static std::vector<int> now;
  now=derived.boundaries;
  for(int i=0; i<number_vertex_; i++){
    for(; j<origin.boundaries[i]; j++){
      int y=origin.targets[j];
      derived.targets[--now[y]]=i;
    }
    reverse_process.Update(i);
  }
  
  now.clear();
  reverse_process.Stop();
}

void BasicGraph::Intersect(){
  Reverse();
  BasicGraphImpl& origin=graphs_[static_cast<int>(OUT)];
  BasicGraphImpl& intermediate=graphs_[static_cast<int>(IN)];
  BasicGraphImpl& derived=graphs_[static_cast<int>(INTERSECTION)];
  if (derived.generated)
    return;
  derived.Clear();
  derived.generated=1;
  derived.boundaries.resize(number_vertex_);

  static std::vector<int> visited, now;
  visited.resize(number_vertex_);
  static int version=10;

  mProcess intersect_process("Intersection graph generation", 2 * number_vertex_, verbose_, 1000);
  intersect_process.Start();
  
  for(int o=0; o<1; o++){
    for(int i=0, j=0, j1=0; i<number_vertex_; i++){
      version++;
      //mark all outpoints in OUT
      for(; j<origin.boundaries[i]; j++){
        int y=origin.targets[j];
        visited[y]=version;
      }
      for(; j1<intermediate.boundaries[i]; j1++){
        int y=intermediate.targets[j1];
        if (visited[y] == version){
          //you assert that (i,j) exist both in OUT and IN, thus in INTERSECTION too
          if (o == 0){
            derived.number_edges++;
            derived.boundaries[i]++;
          }else{
            derived.targets[ --now[i] ] = y;
          }
        }
      }
      intersect_process.Update( o * number_vertex_ + i );
    }
    if (o == 0){
      for(int i=1; i<number_vertex_; i++)
        derived.boundaries[i]+=derived.boundaries[i-1];
      now=derived.boundaries;
      derived.targets.resize(derived.number_edges);
    }
  }

  now.clear();
  intersect_process.Stop();
}

void BasicGraph::Union(){
  mProcess union_process("Union graph generation", 2 * number_vertex_, verbose_, 1000);
  BasicGraphImpl& origin=graphs_[static_cast<int>(OUT)];
  BasicGraphImpl& intermediate=graphs_[static_cast<int>(IN)];
  BasicGraphImpl& derived=graphs_[static_cast<int>(UNION)];
  if (derived.generated)
    return;
  derived.Clear();
  derived.generated=1;
  derived.boundaries.resize(number_vertex_);
  
  static std::vector<int> visited, now;
  visited.resize(number_vertex_);
  static int version=10;

  union_process.Start();

  for(int o=0; o<1; o++){
    for(int i=0, j=0, j1=0; i<number_vertex_; i++){
      version++;
      //mark all outpoints in OUT
      for(; j<origin.boundaries[i]; j++){
        int y=origin.targets[j];
        visited[y]=version;
        if (o==0){
          derived.number_edges++;
          derived.boundaries[i]++;
        }else{
          derived.targets[ --now[i] ] = y;
        }
      }
      for(; j1<intermediate.boundaries[i]; j1++){
        int y=intermediate.targets[j1];
        if (visited[y] != version){
          if (o==0){
            derived.number_edges++;
            derived.boundaries[i]++;
          }else{
            derived.targets[ --now[i] ] = y;
          }
        }
      }
      union_process.Update( o * number_vertex_ + i );
    }
    if (o == 0){
      for(int i=1; i<number_vertex_; i++)
        derived.boundaries[i]+=derived.boundaries[i-1];
      now=derived.boundaries;
      derived.targets.resize(derived.number_edges);
    }
  }
  
  now.clear();
  union_process.Stop();
}
