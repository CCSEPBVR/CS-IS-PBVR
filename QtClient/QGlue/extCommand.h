#ifndef ExtCommand_H
#define ExtCommand_H

#include <Client/Command.h>
#include <Client/Argument.h>
#include <Client/ComThread.h>


class ExtCommand : public kvs::visclient::Command
{
public:
    ExtCommand(int argc, char *argv[]);
    kvs::visclient::Argument argument;
    kvs::visclient::ComThread comthread; 
    kvs::visclient::ParticleServer m_server; 

    void initCommand();
    void inSituInit();
    void clientServerInit();
    void parseArguments();
    void CallBackApply( const int i );

    void registerPolygonModel(std::string,int currentIndex,double opacity,kvs::RGBColor color, bool isSTL );
    void registerEmptyPolygonModel(std::string,int currentIndex,double opacity,kvs::RGBColor color);
    void deletePolygonModel(int currentIndex);

//    void replacePolygonModel(std::string,int currentIndex,double opacity,kvs::RGBColor color);
    //OSAKI
//    int  getServerParticleInfomation();

    const static kvs::Vec3 referenceMinCoord;
    const static kvs::Vec3 referenceMaxCoord;

protected:
    void registerCGModel();

#ifdef ENABLE_BENCHMARK_POLYGON
    void registerBenchmarkModel();
#endif // ENABLE_BENCHMARK_POLYGON

};
extern ExtCommand* extCommand;

#endif // ExtCommand_H
