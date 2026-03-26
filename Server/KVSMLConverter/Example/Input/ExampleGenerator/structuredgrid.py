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
