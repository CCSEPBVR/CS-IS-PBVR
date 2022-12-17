import os
import vtk


def generate_triangle_mesh():
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(3, 4, 5)

    geometry_filter = vtk.vtkGeometryFilter()
    geometry_filter.SetInputData(image_data)

    triangle_filter = vtk.vtkTriangleFilter()
    triangle_filter.SetInputConnection(geometry_filter.GetOutputPort())

    writer = vtk.vtkXMLPolyDataWriter()
    writer.SetFileName(os.path.join('..', 'triangle.vtp'))
    writer.SetInputConnection(triangle_filter.GetOutputPort())
    writer.Update()

    stl_writer = vtk.vtkSTLWriter()
    stl_writer.SetFileName(os.path.join('..', 'triangle.stl'))
    stl_writer.SetInputConnection(triangle_filter.GetOutputPort())
    stl_writer.Update()


def generate_quad_mesh():
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(3, 4, 5)

    geometry_filter = vtk.vtkGeometryFilter()
    geometry_filter.SetInputData(image_data)

    writer = vtk.vtkXMLPolyDataWriter()
    writer.SetFileName(os.path.join('..', 'quad.vtp'))
    writer.SetInputConnection(geometry_filter.GetOutputPort())
    writer.Update()


if __name__ == '__main__':
    generate_triangle_mesh()
    generate_quad_mesh()
