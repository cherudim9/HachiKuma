#ifndef UTILITY_H_
#define UTILITY_H_

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>

const int default_step=1;

class mTimer{

 public:

  mTimer(bool verbose=1){
  }

  ~mTimer(){
    Stop();
  }

  void Start(){
    start_time_=std::chrono::system_clock::now();
  }

  double Stop(){
    double dtime=0.0;
    if (verbose_){
      auto stop_time_ = std::chrono::system_clock::now();
      std::chrono::duration<double> duration = stop_time_ - start_time_;
      dtime=duration.count();
      std::clog << "Duration: " << std::setprecision(2) << dtime << std::endl;
    }
    return dtime;
  }

 private:
  std::chrono::system_clock::time_point start_time_;
  bool verbose_;

};

class mProcess{

 public:

 mProcess(const std::string &process_name, int max_status, bool verbose=1, int step=default_step):
  process_name_(process_name), max_status_(max_status), verbose_(verbose), step_(step)
  {
  }
  
  ~mProcess(){
    
  }

  void Start(){
    start_time_ = std::chrono::system_clock::now();
    if (verbose_)
      Display(0);
  }

  void Update(int cur){
    if ( verbose_ && cur % step_ ==0 ){
      Display(cur);
    }
  }

  void Stop(){
    if (verbose_){
      Display(max_status_);
      std::clog << std::endl;
    }
  }
    
 private:

  std::string process_name_;
  int max_status_;
  bool verbose_;
  int step_;
  std::chrono::system_clock::time_point start_time_;
  
  double Gone(){
    auto now=std::chrono::system_clock::now();
    std::chrono::duration<double> duration = now - start_time_;
    return duration.count();
  }

  void Display(int now_status){
    double perc = 100.0 * now_status / max_status_;
    double gone=Gone();
    double time_to_go = ( now_status != 0 ) ? ( gone / perc * 100.0 - gone ) : 0;
    std::clog << "Process[" << process_name_ << "]: " << std::fixed << std::setprecision(2) << perc << "% completed (" << 
      now_status << "/" << max_status_ << "), " <<
      std::setw(10) << std::setprecision(2) << gone << " gone, "<<
      std::setw(10) << std::setprecision(2) << time_to_go << " to go.\r" << std::flush;
  }

};

class FilePath {
  
 public:

  static bool Exist(const std::string& path){
    std::ifstream file(path);
    return file.is_open();
  }
  
  static void CheckForOpen(const std::string& path, const std::ifstream& stream) throw(std::runtime_error){
    if (!stream){
      throw std::runtime_error("Failed to open " + path);
    }
  }

  static void CheckForCreation(const std::string& path, const std::ofstream& stream) throw(std::runtime_error){
    if (!stream){
      throw std::runtime_error("Failed to create " + path);
    }
  }
  
  static void CheckForExistence(const std::string& path) throw(std::runtime_error){
    if (FilePath::Exist(path)){
      throw std::runtime_error("Already exists: " + path);
    }
  }

};

#endif 
