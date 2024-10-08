#ifndef CUSTOMOBJECTMANAGER_H
#define CUSTOMOBJECTMANAGER_H

#include <kvs/ObjectManager>

class CustomObjectManager : public kvs::ObjectManager
{
public:
    using BaseClass = kvs::ObjectManager;
    CustomObjectManager();
    virtual ~CustomObjectManager() {}
    void setNormalize(kvs::Vec3 normalize) { BaseClass::setNormalize(normalize); }

};

#endif // CUSTOMOBJECTMANAGER_H
