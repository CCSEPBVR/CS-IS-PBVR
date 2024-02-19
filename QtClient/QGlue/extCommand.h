#ifndef ExtCommand_H
#define ExtCommand_H

#include <Client/Command.h>
#include <Client/Argument.h>
#include <Client/ComThread.h>

#include <kvs/PolygonImporter>
#ifdef ENABLE_FEATURE_DRAW_TEXTURED_POLYGON
#include <TexturedPolygonImporter.h>
#endif // ENABLE_FEATURE_DRAW_TEXTURED_POLYGON


class ExtCommand : public kvs::visclient::Command
{
public:
    ExtCommand(int argc, char *argv[]);
    kvs::visclient::Argument argument;
    kvs::visclient::ComThread comthread; 
    kvs::visclient::ParticleServer m_server;
    bool las_registerd = false;

    void initCommand();
    void inSituInit();
    void clientServerInit();
    void parseArguments();
    void CallBackApply( const int i );
    void toggleGlyph() { m_is_glyph = !m_is_glyph;}
    //OSAKI
#ifdef GPU_MODE
    void registerPolygonModel(std::string,int currentIndex,double opacity,kvs::RGBColor color, bool isSTL );
    void registerEmptyPolygonModel(std::string,int currentIndex,double opacity,kvs::RGBColor color);
    void deletePolygonModel(int currentIndex);
    void registerGlyph( int port );
    void updateGuestGlyph();
#endif
//    void replacePolygonModel(std::string,int currentIndex,double opacity,kvs::RGBColor color);
    //OSAKI
//    int  getServerParticleInfomation();

protected:
    //OSAKI
    bool registerPolygonModel();
#ifdef ENABLE_LIBLAS
    void registerLASModel();
#endif
private:
    bool m_is_glyph = true;
};
extern ExtCommand* extCommand;

#endif // ExtCommand_H
