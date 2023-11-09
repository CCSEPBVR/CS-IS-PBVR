#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "objnameutil.h"

const std::set<std::string> ObjectNameUtil::ignoreBoundingBoxNames{
    OBJ_NAME_BOUNDING_BOX,
    OBJ_NAME_BOUNDING_BOX_SUBMESH,
    OBJ_NAME_BOUNDING_BOX_SCALE_LABELS,
    OBJ_NAME_BOUNDING_BOX_SCALE_BOX,
    OBJ_NAME_CONTROL_SPHERE,
    OBJ_NAME_VR_HANDS,
    OBJ_NAME_CGMODEL,
    OBJ_NAME_POLYGONMODEL,
};

const std::set<std::string> ObjectNameUtil::ignoreTranslationFactorNames{
    OBJ_NAME_BOUNDING_BOX,
    OBJ_NAME_BOUNDING_BOX_SUBMESH,
    OBJ_NAME_BOUNDING_BOX_SCALE_LABELS,
    OBJ_NAME_BOUNDING_BOX_SCALE_BOX,
    OBJ_NAME_CONTROL_SPHERE,
    OBJ_NAME_VR_HANDS,
    OBJ_NAME_CGMODEL,
    OBJ_NAME_POLYGONMODEL,
};
const std::set<std::string> ObjectNameUtil::ignoreModelingMatrixNames{
    OBJ_NAME_CONTROL_SPHERE,
    OBJ_NAME_VR_HANDS,
};

bool ObjectNameUtil::isToIgnoreBoundingBox(std::string name) { return ignoreBoundingBoxNames.count(name) != 0; }
bool ObjectNameUtil::isToIgnoreTranslationFactor(std::string name) { return ignoreTranslationFactorNames.count(name) != 0; }
bool ObjectNameUtil::isToIgnoreModelingMatrix(std::string name) { return ignoreModelingMatrixNames.count(name) != 0; };
