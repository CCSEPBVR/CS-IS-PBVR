#!/usr/bin/python
import vtk
import os

from common import make_image_data


def generate_image_data():
    for time_step in range(10):
        image_data = make_image_data([3, 4, 5], [0, 0, 0], time_step)

        writer = vtk.vtkXMLImageDataWriter()
        writer.SetFileName(os.path.join('..', 'Vti',
                                        f'union_{time_step}.vti'))
        writer.SetInputData(image_data)
        writer.Update()


if __name__ == '__main__':
    generate_image_data()
