#pragma once
#include <QVariantList>
#include <QVariantMap>
#include <kvs/Matrix44>
#include <kvs/Vector3>

class ObjectEditor;
class TimeStepControlToolBar;
namespace kvs { class StochasticRenderingCompositor; }
#include "Screen.h"
#include "VizMode.h"

namespace MCPState
{
QVariantList vector( const kvs::Vec3& value );
QVariantList matrix( const kvs::Mat4& value );
QVariantMap collect( kvs::qt::jaea::Screen* screen,
                     kvs::StochasticRenderingCompositor* compositor,
                     Viz::Mode mode, const ObjectEditor* objects,
                     const TimeStepControlToolBar* time );
}
