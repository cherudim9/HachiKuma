#include "graph_share.h"

void Clear(){
  if (!set_)
    return;
  set_=0;
  if (created_by_me_){
    shmdt(shm_);
    
  }else{
    shmdt(shm_);
  }
}

SharedGraphResult SharedGraph::LoadSharedGraph(int shmkey, int shmsize){
  Clear();
  created_by_me_=0;
  set_=1;
  shmid_=shmget(shmkey, shmsize, 0666);
  if (shmid_<0)
    return LoadingWrongKey;
  shm_=shmat(shmid_, NULL, 0);
  char *shm_chr=static_cast<char*>( shm_ );
  if ( shm_chr == (char*)-1 )
    return LoadingWrongMemory;
  graph_=reinterpret_cast<BasicGraph*>(shm_chr);
}

SharedGraphResult SharedGraph::CreateSharedGraph(BasicGraph *g){
  Clear();
  graph_=g;
  set_=1;
  created_by_me_=1;
  int key=(int)g;
  shmid_=shmget(key, sizeof(*graph_), IPC_CREATE | 0666);
  if (shmid_<0)
    return WritingWrongKey;
  shm_=static_cast<char*>(shmat(shmid_, NULL, 0));
  if (shm_ == (char*)-1)
    return WritingWrongMemory;
  memcpy(shm_, graph_, sizeof(*graph_));
}
