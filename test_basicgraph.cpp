#include "basic_graph.h"
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
using namespace std;
#define TERMINATE(x) {cout<<"Wrong in Case "<<t<<": "<<x<<endl; exit(0);}

string ItoA(int x){
  stringstream O;
  O<<x;
  string s;
  O>>s;
  return s;
}

int maxN=100;
vector<vector<int> > edge, edge_in, edge_inter, edge_union;
double edge_factor=0.3;

void TestImpl(int t, string name, vector<vector<int> > &edge){
  ifstream ind_stream(name+".ind");
  FilePath::CheckForOpen(name+".ind", ind_stream);
  ifstream bou_stream(name+".bou");
  FilePath::CheckForOpen(name+".bou", bou_stream);
  ifstream tar_stream(name+".tar");
  FilePath::CheckForOpen(name+".tar", tar_stream);
  int n;
  ind_stream>>n;
  for(int i=0, m=0, x; i<n; i++){
    int y=m;
    bou_stream>>x;
    m=x;
    if ( edge[i].size() != x-y ){
      TERMINATE("Wrong degree of node "+ItoA(i)+" in "+name);
    }
  }
  for(int i=0; i<n; i++){
    vector<int> neighbor;
    for(int j=0, x; j!=edge[i].size(); i++){
      tar_stream>>x;
      neighbor.push_back(x);
    }
    sort(neighbor.begin(), neighbor.end());
    for(int j=0; j!=edge[i].size(); j++)
      if (neighbor[j] != edge[i][j]){
        TERMINATE("Wrong neigbor for node "+ItoA(i)+" : expecting "+ItoA(edge[i][j])+" but got "+ItoA(neighbor[j])+" in "+name);
      }
  }
}

void Test(int t){
  mProcess test_process("Testing "+ItoA(t)+"th case", 1, 1);
  test_process.Start();
  system("rm -f result.*");

  int n=rand()%maxN+3, m=0;
  edge.clear();
  edge.resize(n);
  edge_in=edge_inter=edge_union=edge;
  for(int i=0; i<n; i++)
    for(int j=0; j<n; j++)
      if (i != j && RandUnity() < edge_factor){
        edge[i].push_back(j);
        edge_in[j].push_back(i);
        m++;
      }
  for(int i=0; i<n; i++)
    for(int j=0; j<n; j++)
      if (i!=j){
        if (find(edge[i].begin(), edge[i].end(), j)!=edge[i].end() &&
            find(edge[j].begin(), edge[j].end(), i)!=edge[j].end())
          edge_inter[i].push_back(j);
        if (find(edge[i].begin(), edge[i].end(), j)!=edge[i].end() ||
            find(edge[j].begin(), edge[j].end(), i)!=edge[j].end())
          edge_union[i].push_back(j);
      }
  string name="test"+ItoA(t);
  ofstream ind_stream(name+".ind");
  ind_stream<<n<<endl<<m<<endl;
  ofstream out_ind_stream(name+".imp_out.ind");
  out_ind_stream<<n<<endl<<m<<endl;
  ofstream out_bou_stream(name+".imp_out.bou");
  ofstream out_tar_stream(name+".imp_out.tar");
  for(int i=0, k=0; i<n; i++){
    k+=edge[i].size();
    out_bou_stream<<k<<endl;
    for(auto j: edge[i])
      out_tar_stream<<j<<endl;
  }
  BasicGraph my_g(0);
  my_g.Load(name);
  my_g.Save("result", kIndex+kIn+kOut+kIntersect+kUnion);
  //no need to test mapping
  //  my_g.Dump();
  
  //check index
  name="result";
  ifstream check_ind_stream(name+".ind");
  FilePath::CheckForOpen(name+".ind", check_ind_stream);
  int nn, mm;
  check_ind_stream>>nn>>mm;
  if (nn!=n || mm!=m){
    TERMINATE("Wrong index file");
  }
  //

  //check Out
  TestImpl(t,name+".imp_out",edge);
  //

  //check In
  TestImpl(t,name+".imp_in",edge_in);
  //

  //check Intersect
  TestImpl(t,name+".imp_inter",edge_inter);
  //

  //heck Union
  TestImpl(t,name+".imp_union",edge_union);
  //

  test_process.Stop();

  system( string("rm -f "+name+".*").c_str() );
  system("rm -f result.*");
}

int main(){

  while(1){
    cout<<"srand(time)? Y/N";
    string s;
    cin>>s;
    if (s=="Y")
      srand(time(0));
    else
      if (s=="N")
        ;
      else
        continue;
    break;
  }
  
  int test_times=0;

  cout<<"Times=";
  cin>>test_times;

  for(int t=1; t<=test_times; t++){
    Test(t);
  }

  return 0;
}
