#!/usr/bin/python
import vtk
import os
from common import (
    TETRA_BOTTOM_IDS,
    make_image_data,
    make_hexahedron_only_mesh,
    make_tetrahedron_only_mesh,
    make_pyramid_only_mesh,
    make_wedge_only_mesh,
    make_quad_mesh)


def generate_multi_block():
    time_step = 0
    multi_block = vtk.vtkMultiBlockDataSet()
    image_data = make_image_data(
        [3, 4, 5], [0, 0, 0], time_step)
    hex_only = make_hexahedron_only_mesh(
        [3, 4, 5], [2, 0, 0], time_step)
    quad_mesh = make_quad_mesh(
        [3, 4, 5], [4, 0, 0])

    multi_block.SetBlock(0, image_data)
    multi_block.SetBlock(1, hex_only)
    multi_block.SetBlock(2, quad_mesh)

    writer = vtk.vtkXMLMultiBlockDataWriter()
    writer.SetFileName(
        os.path.join('..', 'MultiBlock', f'multiblock_{time_step}.vtm'))
    writer.SetInputData(multi_block)
    writer.Update()


def generate_unstructured_multi_block():
    for time_step in range(10):
        multi_block = vtk.vtkMultiBlockDataSet()
        hex_only = make_hexahedron_only_mesh([3, 4, 5], [0, 0, 0], time_step)
        tetra_only = make_tetrahedron_only_mesh(
            [3, 4, 5], [2, 0, 0], time_step)
        tetra_only = make_tetrahedron_only_mesh(
            [3, 4, 5], [2, 0, 0], time_step)
        pyramid_only = make_pyramid_only_mesh(
            [3, 4, 5], [4, 0, 0], time_step)
        wedge_only = make_wedge_only_mesh(
            [3, 4, 5], [6, 0, 0], time_step)

        multi_block.SetBlock(0, hex_only)
        multi_block.SetBlock(1, tetra_only)
        multi_block.SetBlock(2, pyramid_only)
        multi_block.SetBlock(3, wedge_only)

        writer = vtk.vtkXMLMultiBlockDataWriter()
        writer.SetFileName(os.path.join('..', 'UnstructuredMultiBlock',
                           f'multiblock_{time_step}.vtm'))
        writer.SetInputData(multi_block)
        writer.Update()


if __name__ == '__main__':
    generate_multi_block()
    generate_unstructured_multi_block()
