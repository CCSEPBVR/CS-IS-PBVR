#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "colorundoredocommand.h"

/**
 * @brief colorUndoRedoCommand::colorUndoRedoCommand
 * @param cmapp
 * @param fromColorMap
 * @param toColorMap
 * @param parent
 */
colorUndoRedoCommand::colorUndoRedoCommand(QGlue::ColorMapPalette *cmapp,const kvs::ColorMap fromColorMap,const kvs::ColorMap toColorMap,QUndoCommand *parent):
    QUndoCommand(parent),
    m_cmapp(cmapp),
    m_fromColorMap(fromColorMap),
    m_toColorMap(toColorMap)
{
}
/**
 * @brief colorUndoRedoCommand::undo
 */
void colorUndoRedoCommand::undo()
{
    qInfo("Undo");
    m_cmapp->setColorMap(m_fromColorMap);
    setText(QString("Undo"));
}
/**
 * @brief colorUndoRedoCommand::redo
 */
void colorUndoRedoCommand::redo()
{
    qInfo("Redo");
    m_cmapp->setColorMap(m_toColorMap);
    setText(QString("Redo"));
}
