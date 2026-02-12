#ifndef JSONKEYS_H
#define JSONKEYS_H
#include <string_view>
namespace Protocol
{
namespace Key
{
inline constexpr std::string_view Event                     = "Event";
inline constexpr std::string_view UserID                    = "UserID";
inline constexpr std::string_view IsOperator                = "IsOperator";

// Transfer Operator
inline constexpr std::string_view TargetID                  = "TargetID";

// Transfer Operator
inline constexpr std::string_view OldOperatorID             = "OldOperatorID";
inline constexpr std::string_view NewOperatorID             = "NewOperatorID";

// Volume Data File Path
inline constexpr std::string_view VolumeDataFilePath        = "VolumeDataFilePath";

// Transfer Function File Path
inline constexpr std::string_view TransferFunctionFilePath  = "TransferFunctionFilePath";

// Initialize Parameter
inline constexpr std::string_view TransferFunctionParameter = "TransferFunctionParameter";
inline constexpr std::string_view GlyphParameter            = "GlyphParameter";
inline constexpr std::string_view PlotOverLineParameter     = "PlotOverLineParameter";
inline constexpr std::string_view PlotOverTimeParameter     = "PlotOverTimeParameter";

// Update IS TimeStep
inline constexpr std::string_view UpdateMaxTimeStep         = "UpdateMaxTimeStep";

// Chat
inline constexpr std::string_view Text                      = "Text";

// Share View
inline constexpr std::string_view Matrix                    = "Matrix";

// Share Point
inline constexpr std::string_view X                         = "X";
inline constexpr std::string_view Y                         = "Y";
inline constexpr std::string_view Z                         = "Z";
inline constexpr std::string_view Dx                        = "Dx";
inline constexpr std::string_view Dy                        = "Dy";
inline constexpr std::string_view Dz                        = "Dz";

// ObjectEditor
inline constexpr std::string_view Objects                   = "Objects";
inline constexpr std::string_view UUID                      = "UUID"; // FIXME:リモートファイルダイアログと重複している。
inline constexpr std::string_view TmpIsDisplay              = "TmpIsDisplay";
inline constexpr std::string_view IsDisplay                 = "IsDisplay";
inline constexpr std::string_view TmpIsKeepInitial          = "TmpIsKeepInitial";
inline constexpr std::string_view IsKeepInitial             = "IsKeepInitial";
inline constexpr std::string_view TmpIsKeepFinal            = "TmpIsKeepFinal";
inline constexpr std::string_view IsKeepFinal               = "IsKeepFinal";
inline constexpr std::string_view TmpIsFocus                = "TmpIsFocus";
inline constexpr std::string_view IsFocus                   = "IsFocus";
inline constexpr std::string_view TmpParticleLimit          = "TmpParticleLimit";
inline constexpr std::string_view ParticleLimit             = "ParticleLimit";
// inline constexpr std::string_view TmpExtraOpacityFactor     = "TmpExtraOpacityFactor";
// inline constexpr std::string_view ExtraOpacityFactor        = "ExtraOpacityFactor";
inline constexpr std::string_view TmpCoordinateX            = "TmpCoordinateX";
inline constexpr std::string_view CoordinateX               = "CoordinateX";
inline constexpr std::string_view TmpCoordinateY            = "TmpCoordinateY";
inline constexpr std::string_view CoordinateY               = "CoordinateY";
inline constexpr std::string_view TmpCoordinateZ            = "TmpCoordinateZ";
inline constexpr std::string_view CoordinateZ               = "CoordinateZ";
inline constexpr std::string_view IsExport                  = "IsExport";
inline constexpr std::string_view TmpPolygonColor           = "TmpPolygonColor";
inline constexpr std::string_view PolygonColor              = "PolygonColor";
inline constexpr std::string_view TmpPolygonOpacity         = "TmpPolygonOpacity";
inline constexpr std::string_view PolygonOpacity            = "PolygonOpacity";
inline constexpr std::string_view CurrentMinObjectCoord     = "CurrentMinObjectCoord";
inline constexpr std::string_view CurrentMaxObjectCoord     = "CurrentMaxObjectCoord";
inline constexpr std::string_view ResultMinObjectCoords     = "ResultMinObjectCoords";
inline constexpr std::string_view ResultMaxObjectCoords     = "ResultMaxObjectCoords";
inline constexpr std::string_view CurrentImportedTimeStep   = "CurrentImportedTimeStep";
inline constexpr std::string_view NeedSameTimeStepReplace   = "NeedSameTimeStepReplace";

// TransferFunctionEditor
inline constexpr std::string_view NumberOfTransferFunction  = "NumberOfTransferFunction";
inline constexpr std::string_view Index                     = "Index";
inline constexpr std::string_view Data                      = "Data";
inline constexpr std::string_view ColorSynthesizer          = "ColorSynthesizer";
inline constexpr std::string_view OpacitySynthesizer        = "OpacitySynthesizer";
inline constexpr std::string_view ColorFunction             = "ColorFunction";
inline constexpr std::string_view ColorVariable             = "ColorVariable";
inline constexpr std::string_view ColorRangeMode            = "ColorRangeMode";
inline constexpr std::string_view ColorUserRangeMin         = "ColorUserRangeMin";
inline constexpr std::string_view ColorUserRangeMax         = "ColorUserRangeMax";
inline constexpr std::string_view ColorServerRangeMin       = "ColorServerRangeMin";
inline constexpr std::string_view ColorServerRangeMax       = "ColorServerRangeMax";
inline constexpr std::string_view ColorMap                  = "ColorMap";
inline constexpr std::string_view R                         = "R";
inline constexpr std::string_view G                         = "G";
inline constexpr std::string_view B                         = "B";
inline constexpr std::string_view ColorHistogram            = "ColorHistogram";
inline constexpr std::string_view OpacityFunction           = "OpacityFunction";
inline constexpr std::string_view OpacityVariable           = "OpacityVariable";
inline constexpr std::string_view OpacityRangeMode          = "OpacityRangeMode";
inline constexpr std::string_view OpacityUserRangeMin       = "OpacityUserRangeMin";
inline constexpr std::string_view OpacityUserRangeMax       = "OpacityUserRangeMax";
inline constexpr std::string_view OpacityServerRangeMin     = "OpacityServerRangeMin";
inline constexpr std::string_view OpacityServerRangeMax     = "OpacityServerRangeMax";
inline constexpr std::string_view OpacityMap                = "OpacityMap";
inline constexpr std::string_view OpacityHistogram          = "OpacityHistogram";

// GlyphEditor
inline constexpr std::string_view Type                      = "Type"; // FIXME:リモートファイルダイアログと重複している。
inline constexpr std::string_view ScaleFactor               = "ScaleFactor";
inline constexpr std::string_view Direction1                = "Direction1";
inline constexpr std::string_view Direction2                = "Direction2";
inline constexpr std::string_view Direction3                = "Direction3";
inline constexpr std::string_view SizeMode                  = "SizeMode";
inline constexpr std::string_view SizeVariables             = "SizeVariables";
inline constexpr std::string_view DistributionMode          = "DistributionMode";
inline constexpr std::string_view NumberOfSamplePoints      = "NumberOfSamplePoints";
inline constexpr std::string_view Seed                      = "Seed";
inline constexpr std::string_view Stride                    = "Stride";
inline constexpr std::string_view ColorDataMode             = "ColorDataMode";
inline constexpr std::string_view ColorDataVariables        = "ColorDataVariables";

// PlotOverLineEditor
inline constexpr std::string_view Enable                    = "Enable";
inline constexpr std::string_view Resolution                = "Resolution";
inline constexpr std::string_view Target                    = "Target";
inline constexpr std::string_view StartCoords               = "StartCoords";
inline constexpr std::string_view EndCoords                 = "EndCoords";
inline constexpr std::string_view ValueOnLine               = "ValueOnLine";
inline constexpr std::string_view XAxis                     = "XAxis";
inline constexpr std::string_view Mask                      = "Mask";

// PlotOverTimeEditor
inline constexpr std::string_view Coords                    = "Coords";
inline constexpr std::string_view ValueOnTime               = "ValueOnTime";
inline constexpr std::string_view Samples                   = "Samples";

inline constexpr std::string_view TimeStep                  = "TimeStep";

// RemoteFileDialog
inline constexpr std::string_view Path                      = "Path";
inline constexpr std::string_view Page                      = "Page";
inline constexpr std::string_view PerPage                   = "PerPage";
inline constexpr std::string_view Files                     = "Files";
inline constexpr std::string_view Name                      = "Name";
inline constexpr std::string_view IsParent                  = "IsParent";
inline constexpr std::string_view HasNext                   = "HasNext";

// SelectedFile
inline constexpr std::string_view File                      = "File";
inline constexpr std::string_view Extension                 = "Extension";
inline constexpr std::string_view Directory                 = "Directory";
inline constexpr std::string_view Format                    = "Format";
inline constexpr std::string_view MinObjectCoord            = "MinObjectCoord";
inline constexpr std::string_view MaxObjectCoord            = "MaxObjectCoord";
inline constexpr std::string_view MinExternalCoord          = "MinExternalCoord";
inline constexpr std::string_view MaxExternalCoord          = "MaxExternalCoord";

inline constexpr std::string_view NumberOfVector            = "NumberOfVector";
inline constexpr std::string_view NumberOfElements          = "NumberOfElements";
inline constexpr std::string_view NumberOfSubvolume         = "NumberOfSubvolume";
inline constexpr std::string_view NumberOfNodes             = "NumberOfNodes";
inline constexpr std::string_view ElementType               = "ElementType";
inline constexpr std::string_view FileType                  = "FileType";
inline constexpr std::string_view StepNumber                = "StepNumber";

inline constexpr std::string_view NextTimeStep              = "NextTimeStep";
inline constexpr std::string_view MinLimit                  = "MinLimit";
inline constexpr std::string_view MaxLimit                  = "MaxLimit";
}

namespace Events
{
inline constexpr std::string_view Join                          = "Join";
inline constexpr std::string_view Left                          = "Left";
inline constexpr std::string_view ID                            = "ID";
inline constexpr std::string_view Operator                      = "Operator";
inline constexpr std::string_view TransferOperator              = "TransferOperator";
inline constexpr std::string_view Initialize                    = "Initialize";
inline constexpr std::string_view Chat                          = "Chat";
inline constexpr std::string_view ShareView                     = "ShareView";
inline constexpr std::string_view SharePoint                    = "SharePoint";
inline constexpr std::string_view FileList                      = "FileList";
inline constexpr std::string_view SelectedFile                  = "SelectedFile";
inline constexpr std::string_view ObjectDelete                  = "ObjectDelete";
inline constexpr std::string_view ObjectInfoParameter           = "ObjectInfoParameter";
inline constexpr std::string_view ServerSideSameTimeStepReplace = "ServerSideSameTimeStepReplace";
inline constexpr std::string_view TransferFunctionParameter     = "TransferFunctionParameter";
inline constexpr std::string_view GlyphParameter                = "GlyphParameter";
inline constexpr std::string_view PlotOverLineParameter         = "PlotOverLineParameter";
inline constexpr std::string_view PlotOverTimeParameter         = "PlotOverTimeParameter";
inline constexpr std::string_view RequestDataAt                 = "RequestDataAt";
inline constexpr std::string_view TimeStepControlParameter      = "TimeStepControlParameter";
inline constexpr std::string_view LatestTimeStep                = "LatestTimeStep";
}
}
#endif // JSONKEYS_H
