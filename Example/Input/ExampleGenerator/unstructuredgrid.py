#!/usr/bin/python
import vtk
import os
from common import (
    TETRA_BOTTOM_IDS,
    make_hexahedron_only_mesh,
    make_tetrahedron_only_mesh,
    append_point_data)


def generate_hexahedron_only_mesh():
    for time_step in range(10):
        unstructured_grid = make_hexahedron_only_mesh(
            [3, 4, 5], [0, 0, 0], time_step)

        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(os.path.join('..', 'Hex',
                           f'hex_only_{time_step}.vtu'))
        writer.SetInputData(unstructured_grid)
        writer.Update()


def generate_tetrahedron_only_mesh():
    for time_step in range(10):
        unstructured_grid = make_tetrahedron_only_mesh(
            [3, 4, 5], [0, 0, 0], time_step)

        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(os.path.join('..', 'Tetra',
                           f'tetra_only_{time_step}.vtu'))
        writer.SetInputData(unstructured_grid)
        writer.Update()


def generate_tetrahedron_and_hexahedron():
    for time_step in range(10):
        image_data = vtk.vtkImageData()
        image_data.SetDimensions([3, 4, 5])

        itos = vtk.vtkImageToStructuredGrid()
        itos.SetInputData(image_data)
        itos.Update()

        structured_grid = itos.GetOutput()

        unstructured_grid = vtk.vtkUnstructuredGrid()
        coords = []

        for i in range(image_data.GetNumberOfPoints()):
            c = structured_grid.GetPoint(i)
            coords.append([c[0], c[1], c[2]])

        for i in range(image_data.GetNumberOfCells()):
            cell = structured_grid.GetCell(i)

            if i > image_data.GetNumberOfCells() / 2:
                b = vtk.vtkBoundingBox(cell.GetBounds())
                center = [0.0, 0.0, 0.0]
                b.GetCenter(center)
                center_id = len(coords)
                coords.append(center)

                for h in TETRA_BOTTOM_IDS:
                    point_ids = vtk.vtkIdList()
                    point_ids.SetNumberOfIds(4)
                    for j, x in enumerate(h):
                        point_ids.SetId(j, cell.GetPointId(x))

                    point_ids.SetId(3, center_id)

                    unstructured_grid.InsertNextCell(
                        vtk.VTK_TETRA, point_ids)
            else:
                unstructured_grid.InsertNextCell(
                    vtk.VTK_HEXAHEDRON, cell.GetPointIds())

        points = vtk.vtkPoints()
        points.SetNumberOfPoints(len(coords))
        for i, c in enumerate(coords):
            points.SetPoint(i, c[0], c[1], c[2])

        unstructured_grid.SetPoints(points)

        append_point_data(unstructured_grid, time_step)

        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(os.path.join(
            '..', 'TetraAndHex', f'tetra_and_hex_{time_step}.vtu'))
        writer.SetInputData(unstructured_grid)
        writer.Update()


if __name__ == '__main__':
    generate_tetrahedron_only_mesh()
    generate_hexahedron_only_mesh()
    generate_tetrahedron_and_hexahedron()
