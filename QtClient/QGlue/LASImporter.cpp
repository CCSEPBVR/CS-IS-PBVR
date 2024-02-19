#include "LASImporter.h"

LASImporter::LASImporter( std::string filename )
{
    std::ifstream ifs( filename, std::ios::in | std::ios::binary);

    liblas::ReaderFactory f;
    liblas::Reader reader = f.CreateWithStream(ifs);

    liblas::Header const& header = reader.GetHeader();
    std::cout << "Compressed: " << header.Compressed() << std::endl;
    std::cout << "Signature: " << header.GetFileSignature() << std::endl;
    std::cout << "Point Format: " << header.GetDataFormatId() << std::endl;
    std::cout << "Points count: " << header.GetPointRecordsCount() << std::endl;
    std::cout << "Record length: " << header.GetDataRecordLength() << std::endl;
    std::cout << "Record count: " << header.GetRecordsCount() << std::endl;
    std::cout << "scale factor: "
              << header.GetScaleX() << "," << header.GetScaleY() << "," << header.GetScaleZ() << std::endl;
    std::cout << "min coord.: "
              << header.GetMinX() << "," << header.GetMinY()  << "," << header.GetMinZ() << std::endl
              << "max coord.: "
              << header.GetMaxX() << "," << header.GetMaxY()  << "," << header.GetMaxZ() << std::endl;

    const int npoints = header.GetPointRecordsCount();
    kvs::ValueArray<kvs::Real32> coordinates( 3*npoints );
    kvs::ValueArray<kvs::UInt8>  colors     ( 3*npoints );
    //kvs::ValueArray<kvs::Real32> normals    ( 3*npoints );

    int i = 0;
    while (reader.ReadNextPoint())
    {
        liblas::Point const& p = reader.GetPoint();

        coordinates[3*i  ] = (float)p.GetX();
        coordinates[3*i+1] = (float)p.GetY();
        coordinates[3*i+2] = (float)p.GetZ();

        colors[3*i  ] = this->int2byte( p.GetColor().GetRed() );
        colors[3*i+1] = this->int2byte( p.GetColor().GetBlue() );
        colors[3*i+2] = this->int2byte( p.GetColor().GetGreen() );

        //normals[3*i  ] = 1;
        //normals[3*i+1] = 0;
        //normals[3*i+2] = 0;

        i++;
    }

    this->setCoords ( coordinates );
    this->setColors ( colors );
    //this->setNormals( normals );
    this->updateMinMaxCoords();
}

kvs::UInt8 LASImporter::int2byte( uint16_t value )
{
    return value >> 8;
}
