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
import vtk
from common import make_quad_mesh


def generate_triangle_mesh():
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(3, 4, 5)

    geometry_filter = vtk.vtkGeometryFilter()
    geometry_filter.SetInputData(image_data)

    triangle_filter = vtk.vtkTriangleFilter()
    triangle_filter.SetInputConnection(geometry_filter.GetOutputPort())

    writer = vtk.vtkXMLPolyDataWriter()
    writer.SetFileName(os.path.join('..', 'Polygon', 'triangle.vtp'))
    writer.SetInputConnection(triangle_filter.GetOutputPort())
    writer.Update()

    stl_writer = vtk.vtkSTLWriter()
    stl_writer.SetFileName(os.path.join('..', 'Polygon', 'triangle.stl'))
    stl_writer.SetInputConnection(triangle_filter.GetOutputPort())
    stl_writer.Update()


def generate_quad_mesh():
    polygon = make_quad_mesh([3, 4, 5], [0, 0, 0])

    writer = vtk.vtkXMLPolyDataWriter()
    writer.SetFileName(os.path.join('..', 'Polygon', 'quad.vtp'))
    writer.SetInputData(polygon)
    writer.Update()


if __name__ == '__main__':
    generate_triangle_mesh()
    generate_quad_mesh()
