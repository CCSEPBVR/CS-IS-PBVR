#ifndef PLOTOVERLINEITEM_H
#define PLOTOVERLINEITEM_H

#include <QStandardItem>

class PlotOverLineItem
{
public:
    enum PlotOverLineItemRole
    {
        IsEnable    = Qt::UserRole + 1,
        Resolution  = Qt::UserRole + 2,
        Variable    = Qt::UserRole + 3,
        StartPoints = Qt::UserRole + 4,
        EndPoints   = Qt::UserRole + 5,
    };
};

#endif // PLOTOVERLINEITEM_H
