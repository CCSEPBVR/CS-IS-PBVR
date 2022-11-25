#ifndef THREAD_TIMER_H
#define THREAD_TIMER_H
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
//#include <map>
//#include <unordered_map>

#include <stdio.h>

using hresclock       = std::chrono::high_resolution_clock;
using dduration  = std::chrono::duration<double,std::milli>;

#define TIMER_DEBUG_ON
//#define TIMER_DEBUG_VERBOSE

class thread_data
{
    public:
    hresclock::time_point start;
    double time_spent=0;
    size_t count=0;

};
class TimedScope;
class TimedScopeTracker
{
    TimedScopeTracker()
    {

    }
    static TimedScopeTracker& instance(){
        static TimedScopeTracker _instance;
        return _instance;
    }
public:
    std::vector<TimedScope*> list;
    static void add(TimedScope* scope){
        instance().list.push_back(scope);
    }
    static std::vector<TimedScope*>* get_list(){
        return &(instance().list);
    }
};

class TimedScope
{


public:
    size_t n_threads;
    thread_data* threads;
    std::string name;


    TimedScope(std::string name, size_t nthreads)
    {
//#ifdef TIMER_DEBUG_VERBOSE
        std::cout<<"CREATING TIMED SCOPE:"<<name<<" size"<<nthreads<<std::endl;
//#endif
                this->name=name;
                this->threads=new thread_data[nthreads];
                this->n_threads=nthreads;
                TimedScopeTracker::add(this);
    }

    static void summary(size_t mpi_size)
    {
#ifdef TIMER_DEBUG_ON
        double time_total=0;
        std::cout<<" PRODUCING SUMMARY"<<std::endl;
        for (TimedScope* l1: *TimedScopeTracker::get_list())
        {
            size_t count_sum=0;

            double time_sum=0;
            double time_max=0;
            std::cout<< std::left <<
            std::setw(45)<<l1->name.substr(0, 44);
            for (int i= 0; i<l1->n_threads; i++){

                 time_sum+=l1->threads[i].time_spent;
                 count_sum+=l1->threads[i].count;
                 time_max = time_max>  l1->threads[i].time_spent? time_max:  l1->threads[i].time_spent;
                 l1->threads[i].count=0;
                 l1->threads[i].time_spent=0;
            }
            size_t nthreads=l1->n_threads;
            time_total+=time_sum;
            double count_avg = count_sum/nthreads;
            std::cout<<"| time avg:"<<
                std::scientific<<  (time_sum/nthreads) /1000 <<", max:"<<
                std::scientific<<  time_max /1000 << "| calls: " << std::endl;
            std::cout << mpi_size<<" x " <<
                nthreads<<" x " <<
                count_avg<<std::endl;
            /*
            std::defaultfloat << mpi_size<<" x "<<
            std::defaultfloat << nthreads<<" x "<<
            std::defaultfloat << count_avg<<std::endl;
            */
        }
        std::cout<<"##Total time spent:"<<time_total<<std::endl;
#endif
    }
};


/**
 * @brief timed_section_start  Begins counting time for a section
 * @param sd, section data pointer
 * @param thid, thread id
 */
static inline void timed_section_start(TimedScope& ts, size_t thid=0)
{
#ifdef TIMER_DEBUG_ON
#ifdef TIMER_DEBUG_VERBOSE
    std::cout<<"timed_section_start::"<<ts.name<<":"<<thid<<std::endl;
#endif
    if(thid >= ts.n_threads){
	std::cout<<" THREAD "<< thid <<" > "<< ts.n_threads<<std::endl;
	exit(1);
    } 
    ts.threads[thid].count++;
    ts.threads[thid].start=hresclock::now();
#endif
}
/**
 * @brief timed_section_end  Ends counting of time for a section
 * @param sd , section data pointer
 * @param thid , thread id
 */
static inline void timed_section_end(TimedScope& ts, size_t thid=0)
{
#ifdef TIMER_DEBUG_ON
#ifdef TIMER_DEBUG_VERBOSE
    std::cout<<"timed_section_end::"<<ts.name<<":"<<thid<<std::endl;
#endif
    if(thid >= ts.n_threads){
        std::cout<<" THREAD "<< thid <<" > "<< ts.n_threads<<std::endl;
        exit(1);
    }
    hresclock::time_point stop = hresclock::now();
    dduration dur=  stop - ts.threads[thid].start;
    ts.threads[thid].time_spent += dur.count();
#endif
}



#endif
