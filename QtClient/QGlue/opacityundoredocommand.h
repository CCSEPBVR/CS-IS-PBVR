#ifndef OPACITYUNDOREDOCOMMAND_H
#define OPACITYUNDOREDOCOMMAND_H
#include "QGlue/opacitymappalette.h"
#include <QUndoCommand>
#include <kvs/ColorMap>

class opacityUndoRedoCommand :public QUndoCommand
{
public:
    opacityUndoRedoCommand(QGlue::OpacityMapPalette *omapp, const kvs::OpacityMap fromOpacityMap, const kvs::OpacityMap toOpacityMap, QUndoCommand *parent = 0);
public:
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    QGlue::OpacityMapPalette *m_omapp;

    kvs::OpacityMap m_fromOpacityMap;
    kvs::OpacityMap m_toOpacityMap;
};

#endif // OPACITYUNDOREDOCOMMAND_H
