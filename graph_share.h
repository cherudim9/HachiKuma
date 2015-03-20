#ifndef GRAPH_SHARE_
#define GRAPH_SHARE_

#include "utility.h"
#include "basic_graph.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <map>

enum SharedGraphResult{
  LoadingWrongKey,
  LoadingWrongMemory,
  WritingWrongKey,
  WritingWrongMemory
};

class SharedGraph{
  
 public:

  SharedGraph(bool verbose=0)
    :verbose_(verbose){}

  ~SharedGraph(){}

  const BasicGraph*

  BasicGraph* GetGraph(){
    return graph_;
  }

  BasicGraph const* GetGraph(){
    return graph_;
  }

  void Clear();

  SharedGraphResult LoadSharedGraph(int shmkey, int shmsize);

  SharedGraphResult CreateSharedGraph(BasicGraph* g);

 private:
  
  bool verbose_;

  BasicGraph* graph_;

  int shmid_;

  auto shm_;

  bool set_;

  bool created_by_me_;

};

#endif
