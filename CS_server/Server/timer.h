#ifndef PBVR_TIMER_INCLUDE
#define PBVR_TIMER_INCLUDE
#include <map>
#include <iostream>
#include <kvs/Timer>
#ifndef CPU_VER
#include "mpi.h"
#endif
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

#ifdef _OPENMP
#  include <omp.h>
#endif

class Timer_CS 
{

private:
//    std::vector<kvs::Timer> m_timer_array;
    kvs::Timer m_timer;
//    std::map<std::string,double> m_cp; 
    std::vector<std::string> m_cp; 
//    std::vector<char*> m_cp; 
    std::vector<double> m_num_cp; 
//    std::vector<float> m_time; 
    std::map<std::string,std::vector<int>> m_cp_id; 
//    std::vector<std::vector<double>> m_time;
    int m_section_id; 
    int m_chapter_id;
    int m_part_id;
//    std::string timer_title;
//    std::string output_csv_file_path;
    std::ofstream ofs_csv_file;

    double m_time;
    bool m_chapter_flag = false;
    bool m_part_flag = false;

    std::vector<int> m_class; // component_number > 0 

void cal_statistic_mpi(const std::vector<double> &all_num_cp, std::vector<std::string> m_cp, const int mpi_size);
void cal_statistic_threads(const std::vector<double> &all_num_cp, std::vector<std::string> m_cp, const int nthreads);

public:

    Timer_CS();
    //Timer_CS(std::string output_csv_file_path);

void time_sta();

void time_end();    

void time_wrapp();

void time_clear();

void insert_blank();

//void time_stamp(const std::string name, const int thid = 0, const std::string function_name = "" );
void time_stamp(const std::string name, const std::string function_name = "" );
//void time_stamp(const std::string name, const int thid = 0);

void set_section(const int section_id);

void set_param(const int class0, const int class1 = 0, const int class2 =0, const int class3 =0, const int class4 =0, const int class5 =0);

void set_time(const double time);

double get_time();

void next_section();

void next_chapter();

void next_part(); 

void export_csv(const std::string filename);

void openfile(const std::string output_csv_file_path);

void closefile();

};

#endif
