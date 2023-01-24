#!/usr/bin/python
# Created by Japan Atomic Energy Agency
#
# To the extent possible under law, the person who associated CC0 with
# this file has waived all copyright and related or neighboring rights
# to this file.
#
# You should have received a copy of the CC0 legal code along with this
# work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
import vtk
import os
from common import (
    TETRA_BOTTOM_IDS,
    make_hexahedron_only_mesh,
    make_tetrahedron_only_mesh,
    append_point_data,
    append_cell_data)


def generate_hexahedron_only_mesh():
    old_cell_count = 0
    for time_step in range(10):
        unstructured_grid = make_hexahedron_only_mesh(
            [3, 4, 5], [0, 0, 0], time_step)
        append_cell_data(unstructured_grid)
        old_cell_count = old_cell_count + unstructured_grid.GetNumberOfCells()

        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(os.path.join('..', 'Hex',
                           f'hex_only_{time_step}.vtu'))
        writer.SetInputData(unstructured_grid)
        writer.Update()


def generate_tetrahedron_only_mesh():
    old_cell_count = 0
    for time_step in range(10):
        unstructured_grid = make_tetrahedron_only_mesh(
            [3, 4, 5], [0, 0, 0], time_step)
        append_cell_data(unstructured_grid)
        old_cell_count += unstructured_grid.GetNumberOfCells()

        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(os.path.join('..', 'Tetra',
                           f'tetra_only_{time_step}.vtu'))
        writer.SetInputData(unstructured_grid)
        writer.Update()


def generate_vertex_only_mesh():
    time_step = 0
    unstructured_grid = make_tetrahedron_only_mesh(
        [3, 4, 5], [0, 0, 0], time_step)

    source = vtk.vtkGlyphSource2D()
    source.SetGlyphTypeToVertex()

    glyph = vtk.vtkGlyph3D()
    glyph.SetSourceConnection(source.GetOutputPort())
    glyph.SetInputData(unstructured_grid)

    converter = vtk.vtkAppendFilter()
    converter.AddInputConnection(glyph.GetOutputPort())
    converter.Update()

    vertex = converter.GetOutput()

    writer = vtk.vtkXMLUnstructuredGridWriter()
    writer.SetFileName(os.path.join('..', 'Vertex',
                                    f'vertex_only_{time_step}.vtu'))
    writer.SetInputData(vertex)
    writer.Update()


def generate_triangle_only_mesh():
    time_step = 0
    unstructured_grid = make_tetrahedron_only_mesh(
        [3, 4, 5], [0, 0, 0], time_step)

    surface = vtk.vtkGeometryFilter()
    surface.SetInputData(unstructured_grid)

    flip = vtk.vtkReverseSense()
    flip.SetInputConnection(surface.GetOutputPort())

    converter = vtk.vtkAppendFilter()
    converter.AddInputConnection(flip.GetOutputPort())
    converter.Update()

    vertex = converter.GetOutput()

    writer = vtk.vtkXMLUnstructuredGridWriter()
    writer.SetFileName(os.path.join('..', 'Triangle',
                                    f'triangle_only_{time_step}.vtu'))
    writer.SetInputData(vertex)
    writer.Update()


def generate_tetrahedron_and_hexahedron():
    old_cell_count = 0
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
        append_cell_data(unstructured_grid, old_cell_count)

        old_cell_count = old_cell_count + unstructured_grid.GetNumberOfCells()

        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(os.path.join(
            '..', 'TetraAndHex', f'tetra_and_hex_{time_step}.vtu'))
        writer.SetInputData(unstructured_grid)
        writer.Update()


if __name__ == '__main__':
    generate_triangle_only_mesh()
    generate_tetrahedron_only_mesh()
    generate_hexahedron_only_mesh()
    generate_tetrahedron_and_hexahedron()
    generate_vertex_only_mesh()
