#!/usr/bin/python
import vtk
import os

from common import append_point_data


def generate(dimensions, origin=[0, 0, 0], time_step=0):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

    append_point_data(image_data, time_step)

    writer = vtk.vtkXMLImageDataWriter()
    writer.SetFileName(os.path.join('..', 'Vti',
                                    f'union_{time_step}.vti'))
    writer.SetInputData(image_data)
    writer.Update()


def generate_image_data():
    for time_step in range(10):
        generate([3, 4, 5], [0, 0, 0], time_step)


if __name__ == '__main__':
    generate_image_data()
