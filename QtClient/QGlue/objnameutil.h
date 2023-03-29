#ifndef OBJNAMEUTIL_H
#define OBJNAMEUTIL_H

#include <string>
#include <set>

#define OBJ_NAME_BOUNDING_BOX               "OPBVR_BoundingBox"
#define OBJ_NAME_BOUNDING_BOX_SUBMESH       "OPBVR_BoundingBox_SubMesh"
#define OBJ_NAME_BOUNDING_BOX_SCALE_LABELS  "OPBVR_BoundingBox_ScaleLabels"
#define OBJ_NAME_BOUNDING_BOX_SCALE_BOX     "OPBVR_BoundingBox_ScaleBox"
#define OBJ_NAME_CONTROL_SPHERE             "OPBVR_ControlSphere"
#define OBJ_NAME_VR_HANDS                   "OPBVR_VRHands"

#define OBJ_NAME_CGMODEL                    "OPBVR_CGModel"
#define OBJ_NAME_PARTICLE                   "OPBVR_Particle"

namespace kvs {
namespace oculus {
namespace jaea {
namespace pbvr {

class ObjectNameUtil {
private:
    const static std::set<std::string> ignoreBoundingBoxNames;
    const static std::set<std::string> ignoreModelingMatrixNames;
    const static std::set<std::string> ignoreTranslationFactorNames;

public:
    static bool isToIgnoreBoundingBox(std::string);
    static bool isToIgnoreTranslationFactor(std::string name);
    static bool isToIgnoreModelingMatrix(std::string);
}; // class ObjectNameUtil

} // namespace pbvr
} // namespace jaea
} // namespace oculus
} // namespace kvs

#endif // OBJNAMEUTIL_H
