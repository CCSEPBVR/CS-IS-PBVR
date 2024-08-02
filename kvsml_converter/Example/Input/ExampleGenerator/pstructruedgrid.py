#!/usr/bin/python
# Created by Japan Atomic Energy Agency
#
# To the extent possible under law, the person who associated CC0 with
# this file has waived all copyright and related or neighboring rights
# to this file.
#
# You should have received a copy of the CC0 legal code along with this
# work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
import os

from mpi4py import MPI
import vtk
from common import make_image_data

mpi_controller = vtk.vtkMPIController()
mpi_controller.Initialize()
nranks = mpi_controller.GetNumberOfProcesses()
rank = mpi_controller.GetLocalProcessId()


def set_ghost(grid, x_center):
    cell_ghost_array = grid.AllocateCellGhostArray()
    point_ghost_array = grid.AllocatePointGhostArray()

    for i in range(grid.GetNumberOfPoints()):
        point = grid.GetPoint(i)
        point_ghost_array.SetTuple(i, [
            1 if point[0] > x_center else 0
        ])

    for i in range(grid.GetNumberOfCells()):
        cell = grid.GetCell(i)

        is_ghost_cell = False
        for j in range(cell.GetNumberOfPoints()):
            pid = cell.GetPointId(j)
            point = grid.GetPoint(pid)

            is_ghost_cell = is_ghost_cell or point[0] > x_center

        cell_ghost_array.SetTuple(i, [1 if is_ghost_cell else 0])

    return grid


def set_non_ghost(grid):
    cell_ghost_array = grid.AllocateCellGhostArray()
    point_ghost_array = grid.AllocatePointGhostArray()

    for i in range(grid.GetNumberOfPoints()):
        point_ghost_array.SetTuple(i, [0])

    for i in range(grid.GetNumberOfCells()):
        cell_ghost_array.SetTuple(i, [0])

    return grid


if __name__ == '__main__':
    time_step = 0
    writer = vtk.vtkXMLPStructuredGridWriter()
    writer.SetController(mpi_controller)

    if rank == 0:
        image_data = make_image_data([3, 4, 5], [0, 0, 0])

        converter = vtk.vtkImageToStructuredGrid()
        converter.SetInputData(image_data)
        converter.Update()
        structured = converter.GetOutput()
        set_ghost(structured, 1)
    elif rank == 1:
        image_data = make_image_data([3, 4, 5], [1, 0, 0])

        converter = vtk.vtkImageToStructuredGrid()
        converter.SetInputData(image_data)
        converter.Update()
        structured = converter.GetOutput()
        structured.SetExtent(1, 3, 0, 3, 0, 4)
        set_ghost(structured, 2)
    elif rank == 2:
        image_data = make_image_data([3, 4, 5], [2, 0, 0])

        converter = vtk.vtkImageToStructuredGrid()
        converter.SetInputData(image_data)
        converter.Update()
        structured = converter.GetOutput()
        structured.SetExtent(2, 4, 0, 3, 0, 4)
        set_ghost(structured, 3)
    else:
        image_data = make_image_data([3, 4, 5], [3, 0, 0])

        converter = vtk.vtkImageToStructuredGrid()
        converter.SetInputData(image_data)
        converter.Update()
        structured = converter.GetOutput()
        structured.SetExtent(3, 5, 0, 3, 0, 4)
        set_non_ghost(structured)

    writer.SetInputData(structured)

    writer.SetNumberOfPieces(nranks)
    writer.SetStartPiece(rank)
    writer.SetEndPiece(rank)
    writer.SetFileName(os.path.join(
        '..', 'Pvts', f'example_{time_step}.pvts'))
    writer.SetUseSubdirectory(True)
    writer.Update()
    writer.Write()

    print("Generated. "
          "But you need to edit 'WholeExtent' and non-ghost 'Piece Extent' in *.pvtu directly")
