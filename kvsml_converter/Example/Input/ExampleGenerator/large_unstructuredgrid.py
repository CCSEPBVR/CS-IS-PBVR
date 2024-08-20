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
    make_hexahedron_only_mesh,
    make_tetrahedron_only_mesh,
    make_pyramid_only_mesh,
    make_wedge_only_mesh,
    make_image_data,
    append_point_data,
    append_cell_data)


def generate_large_mesh(f, dst, prefix):
    old_cell_count = 0
    for time_step in range(8):
        for domain in range(8):
            unstructured_grid = f(
                [26, 26, 26], [2.5*domain, 0, 0], time_step, [0.1, 0.1, 0.1])
            append_point_data(unstructured_grid, time_step)
            append_cell_data(unstructured_grid, time_step)
            old_cell_count += unstructured_grid.GetNumberOfCells()

            writer = vtk.vtkXMLUnstructuredGridWriter()
            writer.SetFileName(os.path.join('..', dst,
                                            f'{prefix}.{domain}.{time_step}.vtu'))
            writer.SetInputData(unstructured_grid)
            writer.Update()


def generate_mixed_large_mesh():
    old_cell_count = 0
    fs = [
        make_tetrahedron_only_mesh,
        make_hexahedron_only_mesh,
        make_pyramid_only_mesh,
        make_wedge_only_mesh
    ]
    step_count = 4

    for time_step in range(step_count):
        os.makedirs(os.path.join('..', 'Large', 'Mixed', f'mixed.{time_step}'), exist_ok=True)
        for z_domain in range(8):
            for y_domain in range(4):
                for x_domain in range(4):
                    domain = x_domain + y_domain * 4 + z_domain * 16
                    unstructured_grid = fs[domain % 4](
                        [51, 101, 51], [5.0*x_domain, 10.0*y_domain, 5.0*z_domain], time_step, [0.1, 0.1, 0.1])
                    append_point_data(unstructured_grid, time_step)
                    append_cell_data(unstructured_grid, time_step)
                    old_cell_count += unstructured_grid.GetNumberOfCells()

                    writer = vtk.vtkXMLUnstructuredGridWriter()
                    writer.SetFileName(os.path.join('..', 'Large', 'Mixed', f'mixed.{time_step}',
                                                    f'mixed.{domain}.{time_step}.vtu'))
                    writer.SetInputData(unstructured_grid)
                    writer.Update()

    with open(os.path.join('..', 'converter2.xml'),'w') as f:
        f.write('<kvsmlConverter version="0">\n')
        f.write('  <target>\n')
        f.write(f'    <input meshDeformation="0">\n')
        for i in range(step_count):
            f.write(f'      <distributed stepId="{i}" lastTimeStepId="{step_count-1}">\n')
            f.write(f'        <directoryPath>../Example/Input/Large/Mixed/mixed.{i}</directoryPath>\n')
            f.write(f'        <wildcard>mixed.*.{i}.vtu</wildcard>\n')
            f.write('      </distributed>\n')
        f.write('    </input>\n')
        f.write('    <output>\n')
        f.write('      <directoryPath>/tmp/mixed</directoryPath>\n')
        f.write('      <prefix>mixed</prefix>\n')
        f.write('    </output>\n')
        f.write('  </target>\n')
        f.write('</kvsmlConverter>\n')


def generate_image_data():
    for time_step in range(8):
        for domain in range(8):
            image_data = make_image_data(
                [9, 51, 51], [0.8*domain, 0, 0], time_step, [0.1, 0.1, 0.1])
            append_point_data(image_data, time_step)
            append_cell_data(image_data, time_step)

            writer = vtk.vtkXMLImageDataWriter()
            writer.SetFileName(os.path.join('..', 'Large', 'LargeVti',
                                            f'union.{domain}.{time_step}.vti'))
            writer.SetInputData(image_data)
            writer.Update()


if __name__ == '__main__':
    generate_large_mesh(make_tetrahedron_only_mesh,
                        'Large/LargeTetra', 'large_tetra')
    generate_large_mesh(make_hexahedron_only_mesh,
                        'Large/LargeHexa', 'large_hexa')
    generate_large_mesh(make_pyramid_only_mesh,
                        'Large/LargePyramid', 'large_pyramid')
    generate_large_mesh(make_wedge_only_mesh,
                        'Large/LargePrism', 'large_prism')
    generate_mixed_large_mesh()
    generate_image_data()
