//#include "FBX.h"

////#ifdef CGFORMATEXT4KVS_SUPPORT_FBXSDK

//#include <cassert>
//#include <iostream>
//#include <map>

//#include <fbxsdk.h>
////#include "/Applications/Autodesk/FBX SDK/2020.3.4/include/fbxsdk.h"

////#define STB_IMAGE_IMPLEMENTATION
//#define STBI_WINDOWS_UTF8
//#include "stb_image.h"

//#include "Utils.h"

//namespace kvs{
//namespace jaea{

//static fbxsdk::FbxAMatrix GetTransformMatrix(fbxsdk::FbxNode* node) {
//    using namespace fbxsdk;

//    // local geometric transform
//    FbxAMatrix geometricMatrix;
//    geometricMatrix.SetIdentity();
//    if (node->GetNodeAttribute()) {
//        geometricMatrix.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
//        geometricMatrix.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
//        geometricMatrix.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
//    }

//    // local transform
//    FbxAMatrix localMatrix = node->EvaluateLocalTransform();

//    // ancestor transform
//    FbxNode* parentNode = node->GetParent();
//    FbxAMatrix parentMatrix = parentNode->EvaluateLocalTransform();
//    while ((parentNode = parentNode->GetParent()) != NULL) {
//        parentMatrix = parentNode->EvaluateLocalTransform() * parentMatrix;
//    }

//    return parentMatrix * localMatrix * geometricMatrix;
//}

//bool FBX::read( const std::string& filename ){
//    using namespace fbxsdk;
//    setFilename(filename);

//#ifdef _WIN32
//    std::string filenameUTF8 = MultibyteToUTF8(filename);
//#else
//    // Because allmost all Linux distribution may regards that file path is encoded in UTF-8...
//    std::string filenameUTF8 = filename;
//#endif

//    // Create scene
//    FbxManager* manager = FbxManager::Create();
//    if (manager == NULL) {
//        //  this is an evaluation copy of the FBX SDK and it is expired.
//        setSuccess(false);
//        return false;
//    }
//    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
//    manager->SetIOSettings(ios);

//    // Setup importer and read file
//    FbxImporter* importer = FbxImporter::Create(manager, "");
//    if (importer == NULL) {
//        //  ???
//        manager->Destroy();
//        setSuccess(false);
//        return false;
//    }
//    if (!importer->Initialize(filenameUTF8.c_str(), -1, manager->GetIOSettings())) {

//#ifdef _DEBUG
//        FbxStatus status = importer->GetStatus();
//        std::string errorCode = "??? unknown ???";
//        switch (status.GetCode()) {
//        case FbxStatus::EStatusCode::eSuccess:
//            errorCode = "eSuccess";
//            break;
//        case FbxStatus::EStatusCode::eFailure:
//            errorCode = "eFailure";
//            break;
//        case FbxStatus::EStatusCode::eInsufficientMemory:
//            errorCode = "eInsufficientMemory";
//            break;
//        case FbxStatus::EStatusCode::eInvalidParameter:
//            errorCode = "eInvalidParameter";
//            break;
//        case FbxStatus::EStatusCode::eIndexOutOfRange:
//            errorCode = "eIndexOutOfRange";
//            break;
//        case FbxStatus::EStatusCode::ePasswordError:
//            errorCode = "ePasswordError";
//            break;
//        case FbxStatus::EStatusCode::eInvalidFileVersion:
//            errorCode = "eInvalidFileVersion";
//            break;
//        case FbxStatus::EStatusCode::eInvalidFile:
//            errorCode = "eInvalidFile";
//            break;
//        default:
//            errorCode = "?????? unkonw ?????";
//        }

//        std::string errorMessage (status.GetErrorString());
//        std::cerr << "FBX::read() error:" << std::endl;
//        std::cerr << "  code= " << errorCode << std::endl;
//        std::cerr << "   msg=" << errorMessage << std::endl;
//#endif //_DEBUG

//        importer->Destroy();
//        manager->Destroy();
//        setSuccess(false);
//        return false;
//    }

//    // Get scene
//    FbxScene* scene = FbxScene::Create(manager, "");
//    if (scene == NULL) {
//        //  ???
//        importer->Destroy();
//        manager->Destroy();
//        setSuccess(false);
//        return false;
//    }
//    if (!importer->Import(scene)) {
//        importer->Destroy();
//        manager->Destroy();
//        setSuccess(false);
//        return false;
//    }
//    importer->Destroy();

//    int numMeshes = scene->GetSrcObjectCount<FbxMesh>();
//    if (numMeshes < 1) {
//        // There is no mesh
//        manager->Destroy();
//        setSuccess(true);
//        return true;
//    }

//    // Triangulate mesh
//    for(int i=0; i<numMeshes; i++){
//        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(i);
//        if (mesh != NULL) {
//            if (!mesh->IsTriangleMesh()) {
//                FbxGeometryConverter geometryConverter(manager);
//                geometryConverter.Triangulate(scene, true);
//                break;
//            }
//        }
//    }

//    unsigned int vertexTop = 0;
//    std::vector<kvs::Real32> normals;
//    std::vector<kvs::Real32> coords;
//    std::vector<kvs::UInt32> connections;
//    std::vector<kvs::Real32> texture2DCoords;
//    std::vector<kvs::UInt32> textureIds;
//    std::vector<FbxFileTexture*> fbxFileTextures;
//    std::map<FbxFileTexture*, kvs::UInt32> textureToTextureID;
//    for(int i=0; i<numMeshes; i++){
//        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(i);
//        if (mesh == NULL) {
//            continue;
//        }
//        FbxNode* node = mesh->GetNode();
//        if (node == NULL) {
//            continue;
//        }
//        const FbxAMatrix& transformMatrix = GetTransformMatrix(node);

//        // read Polygon's vertex connection and vertices
//        int numVerticesPrev = static_cast<int>(coords.size()) / 3;
//        int numFaces = mesh->GetPolygonCount();
//        for(int face=0; face<numFaces; face++){
//            int polygonSize = mesh->GetPolygonSize(face);
//            assert(polygonSize == 3);
//            for(int local=0; local<polygonSize; local++){
//                int vertexIndex = static_cast<int>(coords.size()) / 3;
//                connections.push_back(vertexIndex);

//                int controlPointIndex = mesh->GetPolygonVertex(face, local);
//                const FbxVector4& vertex = transformMatrix.MultT(mesh->GetControlPointAt(controlPointIndex));
//                coords.push_back(static_cast<kvs::Real32>(vertex[0]));
//                coords.push_back(static_cast<kvs::Real32>(vertex[1]));
//                coords.push_back(static_cast<kvs::Real32>(vertex[2]));
//            }
//        }

//        int numVertices = (static_cast<int>(coords.size()) / 3) - numVerticesPrev;

//        // read Normal
//        FbxArray<FbxVector4> fbxNormals;
//        if(mesh->GetPolygonVertexNormals(fbxNormals)){
//            normals.insert(normals.end(), numVertices*3, 0.0);

//            int numNormals = fbxNormals.GetCount();
//            int top = vertexTop*3;
//            for(int j=0; j<numNormals; j++){
//                FbxVector4& normal = fbxNormals[j];
//                int vertex = top + (j*3);
//                normals[vertex] = static_cast<kvs::Real32>(normal[0]);
//                normals[vertex + 1] = static_cast<kvs::Real32>(normal[1]);
//                normals[vertex + 2] = static_cast<kvs::Real32>(normal[2]);
//            }
//        }

//        // read textureIds, fbxFileTextures, texture2DCoords
//        int numLayers = mesh->GetLayerCount();
//        if(0 < numLayers){
//            // uses only first layer
//            FbxLayer* layer = mesh->GetLayer(0);
//            // when numLayers >= 1, layer must NOT be NULL...
//            // if layer is NULL, index "0" is out of range of layers...
//            if (layer == NULL) {
//                continue;
//            }

//            // read textureIds, fbxFileTextures
//            FbxLayerElementMaterial* layerMaterials = layer->GetMaterials();
//            if (layerMaterials != NULL) {
//                auto materialMappingMode = layerMaterials->GetMappingMode();
//                if(materialMappingMode == FbxLayerElement::eAllSame){
//                    // int numMaterials = layerMaterials->GetIndexArray().GetCount();
//                    // assert(numMaterials == 1);

//                    //FbxSurfaceMaterial* material = node->GetMaterial(layerMaterials->GetIndexArray()[0]);
//                    FbxLayerElementArrayTemplate<int>& indexArray = layerMaterials->GetIndexArray();
//                    int numMaterials = indexArray.GetCount();
//                    if (numMaterials > 0) {
//                        FbxSurfaceMaterial* material = node->GetMaterial(indexArray[0]);
//                        // when numLMaterisls > 0, indexArray[0] must NOT be NULL...
//                        // if material is NULL, index "0" is out of range of index array...
//                        if (material != NULL) {
//                            const FbxProperty& prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
//                            int fileTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();

//                            int textureID = 0;
//                            if(0 < fileTextureCount){
//                                // uses only first file texture
//                                FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0);
//                                if(texture != NULL){
//                                    auto it = std::find(fbxFileTextures.begin(), fbxFileTextures.end(), texture);
//                                    if(it == fbxFileTextures.end()){
//                                        textureToTextureID[texture] = static_cast<kvs::UInt32>(fbxFileTextures.size());
//                                        fbxFileTextures.push_back(texture);
//                                    }
//                                    textureID = textureToTextureID[texture];
//                                }
//                            }
//                            textureIds.insert(textureIds.end(), numFaces*3, textureID);
//                        }
//                    }
//                }else if(materialMappingMode == FbxLayerElement::eByPolygon){
//                    // int numMaterials = layerMaterials->GetIndexArray().GetCount();
//                    // assert(numMaterials == numFaces);
//                    for(int face=0; face<numFaces; face++){
//                        FbxSurfaceMaterial* material = node->GetMaterial(layerMaterials->GetIndexArray()[face]);
//                        if (material != NULL) {
//                            const FbxProperty& prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
//                            int fileTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();

//                            int textureID = 0;
//                            if(0 < fileTextureCount){
//                                // uses only first file texture
//                                FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0);
//                                if(texture != NULL){
//                                    auto it = std::find(fbxFileTextures.begin(), fbxFileTextures.end(), texture);
//                                    if(it == fbxFileTextures.end()){
//                                        textureToTextureID[texture] = static_cast<kvs::UInt32>(fbxFileTextures.size());
//                                        fbxFileTextures.push_back(texture);
//                                    }
//                                    textureID = textureToTextureID[texture];
//                                }
//                            }
//                            textureIds.insert(textureIds.end(), 3, textureID);
//                        }
//                    }
//                }else{
//                    std::cerr << "Unsupported material mapping mode=" << materialMappingMode << std::endl;
//                    textureIds.insert(textureIds.end(), numFaces*3, 0);
//                }
//            }

//            // read texture2DCoords
//            FbxArray<const FbxLayerElementUV*> layerUVSets = layer->GetUVSets();
//            if(0 < layerUVSets.Size()){
//                const FbxLayerElementUV* uvSet = layerUVSets[0];
//                // when layerUVSets.Size() > 0, uvSet must NOT be NULL...
//                // if uvSet is NULL, index "0" is out of range of layerUVSets...
//                if (uvSet != NULL) {
//                    auto uvMappingMode = uvSet->GetMappingMode();
//                    auto uvReferenceMode = uvSet->GetReferenceMode();

//                    if(uvMappingMode==FbxLayerElementUV::eByPolygonVertex && (uvReferenceMode == FbxLayerElementUV::eDirect || uvReferenceMode == FbxLayerElementUV::eIndexToDirect) ){
//                        for(int face=0; face<numFaces; face++){
//                            for(int local=0; local<3; local++){
//                                int textureUVIndex = mesh->GetTextureUVIndex(face, local);
//                                const fbxsdk::FbxVector2& uv = uvSet->GetDirectArray().GetAt(textureUVIndex);
//                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[0]));
//                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[1]));
//                            }
//                        }
//                    }else if(uvMappingMode==FbxLayerElementUV::eByControlPoint && uvReferenceMode == FbxLayerElementUV::eDirect){
//                        for(int face=0; face<numFaces; face++){
//                            for(int local=0; local<3; local++){
//                                int vertexIndex = mesh->GetPolygonVertex(face, local);
//                                const fbxsdk::FbxVector2& uv = uvSet->GetDirectArray().GetAt(vertexIndex);
//                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[0]));
//                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[1]));
//                            }
//                        }
//                    }else if(uvMappingMode==FbxLayerElementUV::eByControlPoint && uvReferenceMode == FbxLayerElementUV::eIndexToDirect){
//                        for(int face=0; face<numFaces; face++){
//                            for(int local=0; local<3; local++){
//                                int vertexIndex = mesh->GetPolygonVertex(face, local);
//                                int id = uvSet->GetIndexArray().GetAt(vertexIndex);
//                                const fbxsdk::FbxVector2& uv = uvSet->GetDirectArray().GetAt(id);
//                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[0]));
//                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[1]));
//                            }
//                        }
//                    }else{
//                        std::cerr << "Unsupported UV mode. UV mapping mode=" << uvMappingMode << ", UV reference mode=" << uvReferenceMode << std::endl;
//                        texture2DCoords.insert(texture2DCoords.end(), numFaces*3*2, 0.0);
//                    }
//                }
//            }
//        }

//        vertexTop += numVertices;
//    }
//    m_normals = kvs::ValueArray<kvs::Real32>(normals);
//    m_coords = kvs::ValueArray<kvs::Real32>(coords);
//    m_connections = kvs::ValueArray<kvs::UInt32>(connections);
//    m_texture2DCoords = kvs::ValueArray<kvs::Real32>(texture2DCoords);
//    m_textureIds = kvs::ValueArray<kvs::UInt32>(textureIds);

//    // read texture image files
//    std::vector<kvs::ValueArray<kvs::UInt8>> color_arrays;
//    std::vector<kvs::UInt32> image_widths;
//    std::vector<kvs::UInt32> image_heights;
//    int numFileTextures = static_cast<int>(fbxFileTextures.size());
//    for(int i=0; i<numFileTextures; i++){
//        // textureFilename is encoded with UTF-8
//        const char* textureFilename = fbxFileTextures[i]->GetFileName();
//        if (textureFilename != NULL) {
//            // read image file from filesystem and get width, height and data.
//            const size_t bytesPerChannel = 1;
//            int width = 0;
//            int height = 0;
//            int numChannels = 0; // It will be 4 for RGBA, 3 for RGB
//            stbi_uc* data = stbi_load(textureFilename, &width, &height, &numChannels, 0);
//            if(data == NULL){
//                std::cerr << "Failed to load " << textureFilename << std::endl;
//            }else{
//                if(numChannels == 4) {
//                    kvs::ValueArray<kvs::UInt8> pixels(data, width * height * numChannels);
//                    color_arrays.push_back(pixels);
//                    image_widths.push_back(width);
//                    image_heights.push_back(height);
//                } else if(numChannels == 3) {
//                    kvs::ValueArray<kvs::UInt8> pixels(width * height * 4);
//                    for( size_t iData=0; iData < width * height; iData++){
//                        stbi_uc r = data[iData*3 + 0];
//                        stbi_uc g = data[iData*3 + 1];
//                        stbi_uc b = data[iData*3 + 2];
//                        stbi_uc a = 255;
//                        pixels[iData*4 + 0] = r;
//                        pixels[iData*4 + 1] = g;
//                        pixels[iData*4 + 2] = b;
//                        pixels[iData*4 + 3] = a;
//                    }
//                    color_arrays.push_back(pixels);
//                    image_widths.push_back(width);
//                    image_heights.push_back(height);
//                }else{
//                    std::cerr << "Unsupported numChannels=" << numChannels << ", file=" << textureFilename << std::endl;
//                }
//            }
//        }
//    }
//    m_color_arrays = kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>(color_arrays);
//    m_image_widths = kvs::ValueArray<kvs::UInt32>(image_widths);
//    m_image_heights = kvs::ValueArray<kvs::UInt32>(image_heights);

//    manager->Destroy();
//    setSuccess(true);

//    return true;
//}

//bool FBX::write( const std::string& filename ){
//    setFilename(filename);
//    setSuccess(false);
//    return false;
//}

//} // jaea
//} // kvs

////#endif // CGFORMATEXT4KVS_SUPPORT_FBXSDK

#include "FBX.h"
#include "stb_image.h"
#include <cstring>
#include <map>
#include <kvs/File>
#include <kvs/Assert>
#include <fbxsdk.h>


namespace
{
const int MaxLineLength = 256;
const char* const Delimiter = " \t\n\r";
const std::string FileTypeToString[2] = { "ascii", "binary" };
}


namespace
{
bool IsAsciiType( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Check file type.
    char buffer[ ::MaxLineLength ];
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '\n' ) continue;

        const char* head = strtok( buffer, ::Delimiter );
        if ( !strcmp( head, "solid" ) ) break;
        else if ( !strcmp( head, "facet" ) ) return true;
        else return false;
    }

    size_t counter = 0;
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( counter > 5 ) return false;
        if ( buffer[0] == '\n' ) continue;
        if ( strstr( buffer, "endsolid" ) ) return true;
        if ( strstr( buffer, "facet" ) ) return true;
        counter++;
    }

    return false;
}

} // end of namespace

namespace kvs
{
static fbxsdk::FbxAMatrix GetTransformMatrix(fbxsdk::FbxNode* node) {
    using namespace fbxsdk;

    // local geometric transform
    FbxAMatrix geometricMatrix;
    geometricMatrix.SetIdentity();
    if (node->GetNodeAttribute()) {
        geometricMatrix.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
        geometricMatrix.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
        geometricMatrix.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
    }

    // local transform
    FbxAMatrix localMatrix = node->EvaluateLocalTransform();

    // ancestor transform
    FbxNode* parentNode = node->GetParent();
    FbxAMatrix parentMatrix = parentNode->EvaluateLocalTransform();
    while ((parentNode = parentNode->GetParent()) != NULL) {
        parentMatrix = parentNode->EvaluateLocalTransform() * parentMatrix;
    }

    return parentMatrix * localMatrix * geometricMatrix;
}

bool FBX::CheckExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "fbx" )
    {
        return true;
    }

    return false;
}

FBX::FBX():
//    m_file_type( FBX::Ascii )
    m_file_type( FBX::Binary )
{
}

FBX::FBX( const std::string& filename ):
//    m_file_type( FBX::Ascii )
    m_file_type( FBX::Binary )
{
    this->read( filename );
}

FBX::~FBX()
{
}

void FBX::print( std::ostream& os, const kvs::Indent& indent ) const
{
    os << indent << "Filename : " << BaseClass::filename() << std::endl;
    os << indent << "File type : " << ::FileTypeToString[ m_file_type ] << std::endl;
    os << indent << "Number of triangles : " << m_normals.size() / 3;
}

bool FBX::read( const std::string& filename ){
    using namespace fbxsdk;
    setFilename(filename);

#ifdef _WIN32
    std::string filenameUTF8 = MultibyteToUTF8(filename);
#else
    // Because allmost all Linux distribution may regards that file path is encoded in UTF-8...
    std::string filenameUTF8 = filename;
#endif

    // Create scene
    FbxManager* manager = FbxManager::Create();
    if (manager == NULL) {
        //  this is an evaluation copy of the FBX SDK and it is expired.
        setSuccess(false);
        return false;
    }
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    // Setup importer and read file
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (importer == NULL) {
        //  ???
        manager->Destroy();
        setSuccess(false);
        return false;
    }
    if (!importer->Initialize(filenameUTF8.c_str(), -1, manager->GetIOSettings())) {

#ifdef _DEBUG
        FbxStatus status = importer->GetStatus();
        std::string errorCode = "??? unknown ???";
        switch (status.GetCode()) {
        case FbxStatus::EStatusCode::eSuccess:
            errorCode = "eSuccess";
            break;
        case FbxStatus::EStatusCode::eFailure:
            errorCode = "eFailure";
            break;
        case FbxStatus::EStatusCode::eInsufficientMemory:
            errorCode = "eInsufficientMemory";
            break;
        case FbxStatus::EStatusCode::eInvalidParameter:
            errorCode = "eInvalidParameter";
            break;
        case FbxStatus::EStatusCode::eIndexOutOfRange:
            errorCode = "eIndexOutOfRange";
            break;
        case FbxStatus::EStatusCode::ePasswordError:
            errorCode = "ePasswordError";
            break;
        case FbxStatus::EStatusCode::eInvalidFileVersion:
            errorCode = "eInvalidFileVersion";
            break;
        case FbxStatus::EStatusCode::eInvalidFile:
            errorCode = "eInvalidFile";
            break;
        default:
            errorCode = "?????? unkonw ?????";
        }

        std::string errorMessage (status.GetErrorString());
        std::cerr << "FBX::read() error:" << std::endl;
        std::cerr << "  code= " << errorCode << std::endl;
        std::cerr << "   msg=" << errorMessage << std::endl;
#endif //_DEBUG

        importer->Destroy();
        manager->Destroy();
        setSuccess(false);
        return false;
    }

    // Get scene
    FbxScene* scene = FbxScene::Create(manager, "");
    if (scene == NULL) {
        //  ???
        importer->Destroy();
        manager->Destroy();
        setSuccess(false);
        return false;
    }
    if (!importer->Import(scene)) {
        importer->Destroy();
        manager->Destroy();
        setSuccess(false);
        return false;
    }
    importer->Destroy();

    int numMeshes = scene->GetSrcObjectCount<FbxMesh>();
    if (numMeshes < 1) {
        // There is no mesh
        manager->Destroy();
        setSuccess(true);
        return true;
    }

    // Triangulate mesh
    for(int i=0; i<numMeshes; i++){
        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(i);
        if (mesh != NULL) {
            if (!mesh->IsTriangleMesh()) {
                FbxGeometryConverter geometryConverter(manager);
                geometryConverter.Triangulate(scene, true);
                break;
            }
        }
    }

    unsigned int vertexTop = 0;
    std::vector<kvs::Real32> normals;
    std::vector<kvs::Real32> coords;
    std::vector<kvs::UInt32> connections;
    std::vector<kvs::Real32> texture2DCoords;
    std::vector<kvs::UInt32> textureIds;
    std::vector<FbxFileTexture*> fbxFileTextures;
    std::map<FbxFileTexture*, kvs::UInt32> textureToTextureID;
    for(int i=0; i<numMeshes; i++){
        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(i);
        if (mesh == NULL) {
            continue;
        }
        FbxNode* node = mesh->GetNode();
        if (node == NULL) {
            continue;
        }
        const FbxAMatrix& transformMatrix = GetTransformMatrix(node);

        // read Polygon's vertex connection and vertices
        int numVerticesPrev = static_cast<int>(coords.size()) / 3;
        int numFaces = mesh->GetPolygonCount();
        for(int face=0; face<numFaces; face++){
            int polygonSize = mesh->GetPolygonSize(face);
            assert(polygonSize == 3);
            for(int local=0; local<polygonSize; local++){
                int vertexIndex = static_cast<int>(coords.size()) / 3;
                connections.push_back(vertexIndex);

                int controlPointIndex = mesh->GetPolygonVertex(face, local);
                const FbxVector4& vertex = transformMatrix.MultT(mesh->GetControlPointAt(controlPointIndex));
                coords.push_back(static_cast<kvs::Real32>(vertex[0]));
                coords.push_back(static_cast<kvs::Real32>(vertex[1]));
                coords.push_back(static_cast<kvs::Real32>(vertex[2]));
            }
        }

        int numVertices = (static_cast<int>(coords.size()) / 3) - numVerticesPrev;

        // read Normal
        FbxArray<FbxVector4> fbxNormals;
        if(mesh->GetPolygonVertexNormals(fbxNormals)){
            normals.insert(normals.end(), numVertices*3, 0.0);

            int numNormals = fbxNormals.GetCount();
            int top = vertexTop*3;
            for(int j=0; j<numNormals; j++){
                FbxVector4& normal = fbxNormals[j];
                int vertex = top + (j*3);
                normals[vertex] = static_cast<kvs::Real32>(normal[0]);
                normals[vertex + 1] = static_cast<kvs::Real32>(normal[1]);
                normals[vertex + 2] = static_cast<kvs::Real32>(normal[2]);
            }
        }

        // read textureIds, fbxFileTextures, texture2DCoords
        int numLayers = mesh->GetLayerCount();
        if(0 < numLayers){
            // uses only first layer
            FbxLayer* layer = mesh->GetLayer(0);
            // when numLayers >= 1, layer must NOT be NULL...
            // if layer is NULL, index "0" is out of range of layers...
            if (layer == NULL) {
                continue;
            }

            // read textureIds, fbxFileTextures
            FbxLayerElementMaterial* layerMaterials = layer->GetMaterials();
            if (layerMaterials != NULL) {
                auto materialMappingMode = layerMaterials->GetMappingMode();
                if(materialMappingMode == FbxLayerElement::eAllSame){
                    // int numMaterials = layerMaterials->GetIndexArray().GetCount();
                    // assert(numMaterials == 1);

                    //FbxSurfaceMaterial* material = node->GetMaterial(layerMaterials->GetIndexArray()[0]);
                    FbxLayerElementArrayTemplate<int>& indexArray = layerMaterials->GetIndexArray();
                    int numMaterials = indexArray.GetCount();
                    if (numMaterials > 0) {
                        FbxSurfaceMaterial* material = node->GetMaterial(indexArray[0]);
                        // when numLMaterisls > 0, indexArray[0] must NOT be NULL...
                        // if material is NULL, index "0" is out of range of index array...
                        if (material != NULL) {
                            const FbxProperty& prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
                            int fileTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();

                            int textureID = 0;
                            if(0 < fileTextureCount){
                                // uses only first file texture
                                FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0);
                                if(texture != NULL){
                                    auto it = std::find(fbxFileTextures.begin(), fbxFileTextures.end(), texture);
                                    if(it == fbxFileTextures.end()){
                                        textureToTextureID[texture] = static_cast<kvs::UInt32>(fbxFileTextures.size());
                                        fbxFileTextures.push_back(texture);
                                    }
                                    textureID = textureToTextureID[texture];
                                }
                            }
                            textureIds.insert(textureIds.end(), numFaces*3, textureID);
                        }
                    }
                }else if(materialMappingMode == FbxLayerElement::eByPolygon){
                    // int numMaterials = layerMaterials->GetIndexArray().GetCount();
                    // assert(numMaterials == numFaces);
                    for(int face=0; face<numFaces; face++){
                        FbxSurfaceMaterial* material = node->GetMaterial(layerMaterials->GetIndexArray()[face]);
                        if (material != NULL) {
                            const FbxProperty& prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
                            int fileTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();

                            int textureID = 0;
                            if(0 < fileTextureCount){
                                // uses only first file texture
                                FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0);
                                if(texture != NULL){
                                    auto it = std::find(fbxFileTextures.begin(), fbxFileTextures.end(), texture);
                                    if(it == fbxFileTextures.end()){
                                        textureToTextureID[texture] = static_cast<kvs::UInt32>(fbxFileTextures.size());
                                        fbxFileTextures.push_back(texture);
                                    }
                                    textureID = textureToTextureID[texture];
                                }
                            }
                            textureIds.insert(textureIds.end(), 3, textureID);
                        }
                    }
                }else{
                    std::cerr << "Unsupported material mapping mode=" << materialMappingMode << std::endl;
                    textureIds.insert(textureIds.end(), numFaces*3, 0);
                }
            }

            // read texture2DCoords
            FbxArray<const FbxLayerElementUV*> layerUVSets = layer->GetUVSets();
            if(0 < layerUVSets.Size()){
                const FbxLayerElementUV* uvSet = layerUVSets[0];
                // when layerUVSets.Size() > 0, uvSet must NOT be NULL...
                // if uvSet is NULL, index "0" is out of range of layerUVSets...
                if (uvSet != NULL) {
                    auto uvMappingMode = uvSet->GetMappingMode();
                    auto uvReferenceMode = uvSet->GetReferenceMode();

                    if(uvMappingMode==FbxLayerElementUV::eByPolygonVertex && (uvReferenceMode == FbxLayerElementUV::eDirect || uvReferenceMode == FbxLayerElementUV::eIndexToDirect) ){
                        for(int face=0; face<numFaces; face++){
                            for(int local=0; local<3; local++){
                                int textureUVIndex = mesh->GetTextureUVIndex(face, local);
                                const fbxsdk::FbxVector2& uv = uvSet->GetDirectArray().GetAt(textureUVIndex);
                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[0]));
                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[1]));
                            }
                        }
                    }else if(uvMappingMode==FbxLayerElementUV::eByControlPoint && uvReferenceMode == FbxLayerElementUV::eDirect){
                        for(int face=0; face<numFaces; face++){
                            for(int local=0; local<3; local++){
                                int vertexIndex = mesh->GetPolygonVertex(face, local);
                                const fbxsdk::FbxVector2& uv = uvSet->GetDirectArray().GetAt(vertexIndex);
                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[0]));
                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[1]));
                            }
                        }
                    }else if(uvMappingMode==FbxLayerElementUV::eByControlPoint && uvReferenceMode == FbxLayerElementUV::eIndexToDirect){
                        for(int face=0; face<numFaces; face++){
                            for(int local=0; local<3; local++){
                                int vertexIndex = mesh->GetPolygonVertex(face, local);
                                int id = uvSet->GetIndexArray().GetAt(vertexIndex);
                                const fbxsdk::FbxVector2& uv = uvSet->GetDirectArray().GetAt(id);
                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[0]));
                                texture2DCoords.push_back(static_cast<kvs::Real32>(uv[1]));
                            }
                        }
                    }else{
                        std::cerr << "Unsupported UV mode. UV mapping mode=" << uvMappingMode << ", UV reference mode=" << uvReferenceMode << std::endl;
                        texture2DCoords.insert(texture2DCoords.end(), numFaces*3*2, 0.0);
                    }
                }
            }
        }

        vertexTop += numVertices;
    }
    m_normals = kvs::ValueArray<kvs::Real32>(normals);
    m_coords = kvs::ValueArray<kvs::Real32>(coords);
    m_connections = kvs::ValueArray<kvs::UInt32>(connections);
    m_texture2DCoords = kvs::ValueArray<kvs::Real32>(texture2DCoords);
    m_textureIds = kvs::ValueArray<kvs::UInt32>(textureIds);

    // read texture image files
    std::vector<kvs::ValueArray<kvs::UInt8>> color_arrays;
    std::vector<kvs::UInt32> image_widths;
    std::vector<kvs::UInt32> image_heights;
    int numFileTextures = static_cast<int>(fbxFileTextures.size());
    for(int i=0; i<numFileTextures; i++){
        // textureFilename is encoded with UTF-8
        const char* textureFilename = fbxFileTextures[i]->GetFileName();
        if (textureFilename != NULL) {
            // read image file from filesystem and get width, height and data.
            const size_t bytesPerChannel = 1;
            int width = 0;
            int height = 0;
            int numChannels = 0; // It will be 4 for RGBA, 3 for RGB
            stbi_uc* data = stbi_load(textureFilename, &width, &height, &numChannels, 0);
            if(data == NULL){
                std::cerr << "Failed to load " << textureFilename << std::endl;
            }else{
                if(numChannels == 4) {
                    kvs::ValueArray<kvs::UInt8> pixels(data, width * height * numChannels);
                    color_arrays.push_back(pixels);
                    image_widths.push_back(width);
                    image_heights.push_back(height);
                } else if(numChannels == 3) {
                    kvs::ValueArray<kvs::UInt8> pixels(width * height * 4);
                    for( size_t iData=0; iData < width * height; iData++){
                        stbi_uc r = data[iData*3 + 0];
                        stbi_uc g = data[iData*3 + 1];
                        stbi_uc b = data[iData*3 + 2];
                        stbi_uc a = 255;
                        pixels[iData*4 + 0] = r;
                        pixels[iData*4 + 1] = g;
                        pixels[iData*4 + 2] = b;
                        pixels[iData*4 + 3] = a;
                    }
                    color_arrays.push_back(pixels);
                    image_widths.push_back(width);
                    image_heights.push_back(height);
                }else{
                    std::cerr << "Unsupported numChannels=" << numChannels << ", file=" << textureFilename << std::endl;
                }
            }
        }
    }
    m_color_arrays = kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>(color_arrays);
    m_image_widths = kvs::ValueArray<kvs::UInt32>(image_widths);
    m_image_heights = kvs::ValueArray<kvs::UInt32>(image_heights);

    manager->Destroy();
    setSuccess(true);

    return true;
}

bool FBX::write( const std::string& filename )
{
    setFilename( filename );
    setSuccess( false );
    return false;
}
}
