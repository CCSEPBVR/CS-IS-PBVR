#!/usr/bin/python
import os

from mpi4py import MPI
import vtk
from common import (
    TETRA_BOTTOM_IDS,
    make_hexahedron_only_mesh,
    make_tetrahedron_only_mesh,
    make_pyramid_only_mesh,
    make_wedge_only_mesh)

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

            is_ghost_cell = (
                is_ghost_cell or point_ghost_array.GetTuple(pid)[0] == 1)

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


def generate_hexahedron_only_mesh(time_step):
    grid = make_hexahedron_only_mesh(
        [3, 4, 5], [time_step, 0, 0], time_step)
    return set_ghost(grid, time_step + 1)


def generate_tetrahedron_only_mesh(time_step):
    grid = make_tetrahedron_only_mesh(
        [3, 4, 5], [time_step + 1, 0, 0], time_step)
    return set_ghost(grid, time_step + 2)


def generate_pyramid_only_mesh(time_step):
    grid = make_pyramid_only_mesh(
        [3, 4, 5], [time_step + 2, 0, 0], time_step)
    return set_ghost(grid, time_step + 3)


def generate_wedge_only_mesh(time_step):
    grid = make_wedge_only_mesh(
        [3, 4, 5], [time_step + 3, 0, 0], time_step)
    return set_non_ghost(grid)


if __name__ == '__main__':
    for time_step in range(10):
        writer = vtk.vtkXMLPUnstructuredGridWriter()
        writer.SetController(mpi_controller)

        if rank == 0:
            writer.SetInputData(generate_hexahedron_only_mesh(time_step))
        elif rank == 1:
            writer.SetInputData(generate_tetrahedron_only_mesh(time_step))
        elif rank == 2:
            writer.SetInputData(generate_pyramid_only_mesh(time_step))
        else:
            writer.SetInputData(generate_wedge_only_mesh(time_step))

        writer.SetNumberOfPieces(nranks)
        writer.SetStartPiece(rank)
        writer.SetEndPiece(rank)
        writer.SetFileName(os.path.join(
            '..', 'Pvtu', f'example_{time_step}.pvtu'))
        writer.SetUseSubdirectory(True)
        writer.Write()
