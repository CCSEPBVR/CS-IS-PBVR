#include "timer.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cstring>
#include <cmath>

////constructer
//Timer_CS::Timer_CS()
Timer_CS::Timer_CS()
{
//    m_section_id = 0;
//    m_chapter_id = 0;
    //std::string timer_title;
    //std::string output_csv_file_path = "/home/g6/a214026/scp/time.txt";
    //ofs_csv_file.open(output_csv_file_path );
    //ofs_csv_file << " section, name, time, sort_id" << std::endl;
}

/*===========================================================================*/
/**
 *  @brief  stamp
 */
/*===========================================================================*/
void Timer_CS::time_sta()
{
    m_timer.start();
}

void Timer_CS::time_end()
{
    m_timer.stop();
    m_time = m_timer.sec();
}

void Timer_CS::time_wrapp()
{
    m_timer.stop();
    m_time = m_timer.sec(); 
    m_timer.start();
}
 
void Timer_CS::set_time(const double time)
{
    m_time = time;
}

double Timer_CS::get_time()
{
    return m_time;
}

void Timer_CS::time_clear()
{
    m_time = 0;
}

void Timer_CS::set_section(const int section_id)
{
    m_section_id = section_id;
}

void Timer_CS::set_param(const int class0 , const int class1, const int class2, const int class3, const int class4, const int class5)
{
    std::vector<int> tmp = { class0, class1, class2, class3, class4, class5 };
    m_class = tmp; 
}

void Timer_CS::insert_blank()
{
    ofs_csv_file << std::endl;
}

void Timer_CS::next_section()
{ 
    m_section_id += 1;
    m_chapter_id = 0; //reset chapter_id
    m_part_id = 0;
    m_chapter_flag = false;
    m_part_flag = false;
}

void Timer_CS::next_chapter()
{
    m_chapter_id ++;
    m_part_id = 0;
    m_part_flag = false;
}

void Timer_CS::next_part()
{
    m_part_id ++;
}

//void Timer_CS::time_stamp(const std::string stamp, const int thid )
void Timer_CS::time_stamp(const std::string stamp, const std::string function_name )
//void time_stamp(const std::string name, const int thid = 0, const std::string function_name = "" );
{

    //double time = m_timer.sec();
    double time = m_time;
//    std::string time_str = std::to_string(time)   
//    char* time_char = new char [time_str.length() + 1]; 
//      std::strcpy (time_char, time_str.c_str());
#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    int rank = 0;
	int mpi_size = 1;
#endif

#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

//        std::cout << rank << std::endl;
        // 01     00    00     00     00     00      00   00 
     // class0 class1 class2 class3 class4  class5  rank thid
        //double dstamp = thid + rank*100 ;
        long dstamp = thid + rank*100 ;
 
        for(int i = 0; i < m_class.size(); i++)
        {
               dstamp = dstamp +  m_class[i] * pow(10, (7-i)*2) ; 
        }

        std::stringstream ss;
//        ss << "_";
//        ss << std::setfill('0') << std::setw(2) << m_class[0];

        for(int i = 0; i < m_class.size(); i++)
        {
            if (m_class[i] > 0)
            {
                ss << std::setfill('0') << std::setw(2) << m_class[i];
                ss << "_";
            }
        }

        //add process number
        ss << ",";
        ss << std::setfill('0') << std::setw(2) << rank;
        ss << "_";
        ss << std::setfill('0') << std::setw(2) << thid;

//        m_cp_id->first = m_class; 
//        m_cp_id->first.push_front(thid); 
//        m_cp_id->first.push_front(rank); 
        std::string new_stamp = stamp + "," + ss.str();
        char* tmp_stamp = new char [new_stamp.length() + 1]; 
        std::strcpy (tmp_stamp, new_stamp.c_str());
       
//        std::cout << std::setw(16) <<  std::to_string(dstamp) << std::endl; 
        m_num_cp.push_back(dstamp);
        m_num_cp.push_back(time);

        //MPI_Barrier(MPI_COMM_WORLD);
        ofs_csv_file << new_stamp << "," << function_name << "," << time  << "," << std::setw(16) << std::to_string(dstamp)<<std::endl; 
        m_time  = 0;
}

void Timer_CS::export_csv(const std::string filename)
{

#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    int rank = 0;
	int mpi_size = 1;
#endif

#if _OPENMP
        int nthreads = omp_get_num_threads();
//        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
//        int thid     = 0;
#endif


//   std::map<std::string, double> all_cp; 
//     std::vector<std::string> all_cp;  
     std::vector<std::string> all_cp;  
     std::vector<double> all_num_cp;  
     all_cp.resize(m_cp.size() * mpi_size);
     all_num_cp.resize(m_num_cp.size() * mpi_size);
    

//#ifndef CPU_VER
//     MPI_Allgather(&m_num_cp, m_num_cp.size(), MPI_DOUBLE, &all_num_cp, m_num_cp.size(), MPI_DOUBLE, MPI_COMM_WORLD);
//#endif
//// cal_statistic
////    cal_statistic_mpi( all_num_cp, all_cp, mpi_size );
////    cal_statistic_threads( all_num_cp, all_cp, nthreads);
////extract  by section number 
//     int size = all_num_cp.size()/2
////         std::vector<double> tmp_time[size];
////     for(int i = 0; i< size; i ++)
////     {
////         tmp_time[i].resize(mpi_size); 
////     }
//     for(int i = 0; i< all_num_cp.size(); i +=2 )
//     {
////         int ii = i % m_num_cp.size();
////         int ii2 = ii/2;
//         
//        std::stringstream ss;
////        ss << "_";
////        ss << std::setfill('0') << std::setw(2) << m_class[0];
//         for (int j = 0; j < m_class.size(); j++) 
//         {
//            
//            long div; 
//            long mod;
//            div = long (m_num_cp[ii]) * 0.0001;  
//            //div = long int (long int(m_num_cp[i]) * 0.0001);
//            mod = long (m_num_cp[ii]) % 10000;
//            std::cout << "div = " <<  div << "mod = " << mod;
//            ss << "_";
//            ss << std::setfill('0') << std::setw(2) << div;
//         }
//
//         long tmp_div; 
//         long tmp_mod;
//         tmp_div = long (all_num_cp[i]) * 0.0001;  
//         tmp_mod = long (all_num_cp[i]) % 10000;
//
//         if(div - tmp_div == 0)
//         {
//             tmp_time[ii2][rank] = all_num_cp[i+1];
//         }
//         }
//     }
//}
    std::string timer_title;
    double section_time;
    std::string output_csv_file_path = "/home/g6/a214026/cal_log/time.csv";
    std::ofstream ofs_csv_file(output_csv_file_path );

    ofs_csv_file << std::endl;
}

void Timer_CS::cal_statistic_mpi(const std::vector<double> &all_num_cp, std::vector<std::string> m_cp, const int mpi_size )
{
//#ifndef CPU_VER
//    int rank;
//    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
//#else
//    int rank = 0;
//#endif
//
//   //extract  by section number 
//   int size = m_cp.size()/2
//   std::vector<double> tmp_time[size];
//   for(int i = 0; i< size; i ++)
//   {
//       tmp_time[i].resize(mpi_size); 
//   }
//   for(int i = 0; i< all_num_cp.size(); i +=2 )
//   {
//      int ii = i % m_num_cp.size();
//      int ii2 = ii/2;
//      long div; 
//      long mod;
//      div = long (m_num_cp[ii]) * 0.0001;  
//      //div = long int (long int(m_num_cp[i]) * 0.0001);
//      mod = long (m_num_cp[ii]) % 10000;
//      std::cout << "div = " <<  div << "mod = " << mod;
//
//      long tmp_div; 
//      long tmp_mod;
//      tmp_div = long (all_num_cp[i]) * 0.0001;  
//      tmp_mod = long (all_num_cp[i]) % 10000;
//
//      if(div - tmp_div == 0)
//      {
//         tmp_time[ii2][rank] = all_num_cp[i+1];
//      }
//
//   } 
//
}

void Timer_CS::cal_statistic_threads(const std::vector<double> &all_num_cp, std::vector<std::string> m_cp, const int nthreads)
{
//    std::max();
//    std::min();
}

void Timer_CS::openfile(const std::string  output_csv_file_path)
{
    ofs_csv_file.open(output_csv_file_path);
    ofs_csv_file << " section, section_id, rank_thread, name, time(sec), sort_id" << std::endl;
}

void Timer_CS::closefile()
{
    ofs_csv_file.close();
}


