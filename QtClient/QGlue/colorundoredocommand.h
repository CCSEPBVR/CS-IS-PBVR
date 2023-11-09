#ifndef UNDOREDOCOMMAND_H
#define UNDOREDOCOMMAND_H
#include "QGlue/colormappalette.h"
#include <QUndoCommand>
#include <kvs/ColorMap>
#include <kvs/Texture1D>

class colorUndoRedoCommand :public QUndoCommand
{
public:
    colorUndoRedoCommand(QGlue::ColorMapPalette *cmapp, const kvs::ColorMap fromColorMap, const kvs::ColorMap toColorMap, QUndoCommand *parent = 0);
public:
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    //一時的にpublicにする2/28
//private:
    QGlue::ColorMapPalette *m_cmapp;

    kvs::ColorMap m_fromColorMap;
    kvs::ColorMap m_toColorMap;
};

#endif // UNDOREDOCOMMAND_H
