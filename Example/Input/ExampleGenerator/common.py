#!/usr/bin/python
import vtk
import math


TETRA_BOTTOM_IDS = [
    [0, 1, 3],
    [1, 2, 3],
    [0, 1, 4],
    [1, 5, 4],
    [1, 2, 5],
    [2, 6, 5],
    [3, 0, 4],
    [3, 4, 7],
    [2, 7, 3],
    [2, 6, 7],
    [4, 7, 5],
    [7, 6, 5]
]

PRISM_BOTTOM_IDS = [
    [0, 1, 2, 3],
    [0, 1, 5, 4],
    [1, 2, 6, 5],
    [2, 3, 7, 6],
    [3, 0, 4, 7],
    [5, 4, 7, 6]
]

WEDGE_IDS = [
    [4, 5, 7, 0, 1, 3],
    [5, 7, 6, 1, 3, 2]
]


def append_point_data(unstructured_grid, offset=0):
    dist = vtk.vtkFloatArray()
    dist.SetNumberOfTuples(unstructured_grid.GetNumberOfPoints())
    dist.SetNumberOfComponents(1)
    dist.SetName('distance')

    for i in range(unstructured_grid.GetNumberOfPoints()):
        p = unstructured_grid.GetPoint(i)
        dist.SetTuple(
            i, [math.sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]) + offset])

    unstructured_grid.GetPointData().AddArray(dist)


def make_hexahedron_only_mesh(dimensions, origin=[0, 0, 0], time_step=0):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

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

        unstructured_grid.InsertNextCell(
            vtk.VTK_HEXAHEDRON, cell.GetPointIds())

    points = vtk.vtkPoints()
    points.SetNumberOfPoints(len(coords))
    for i, c in enumerate(coords):
        points.SetPoint(i, c[0], c[1], c[2])

    unstructured_grid.SetPoints(points)

    append_point_data(unstructured_grid, time_step)

    return unstructured_grid


def make_tetrahedron_only_mesh(dimensions, origin=[0, 0, 0], time_step=0):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

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

    points = vtk.vtkPoints()
    points.SetNumberOfPoints(len(coords))
    for i, c in enumerate(coords):
        points.SetPoint(i, c[0], c[1], c[2])

    unstructured_grid.SetPoints(points)

    append_point_data(unstructured_grid, time_step)

    return unstructured_grid


def make_pyramid_only_mesh(dimensions, origin=[0, 0, 0], time_step=0):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

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
        b = vtk.vtkBoundingBox(cell.GetBounds())
        center = [0.0, 0.0, 0.0]
        b.GetCenter(center)
        center_id = len(coords)
        coords.append(center)

        for h in PRISM_BOTTOM_IDS:
            point_ids = vtk.vtkIdList()
            point_ids.SetNumberOfIds(5)
            for j, x in enumerate(h):
                point_ids.SetId(j, cell.GetPointId(x))

            point_ids.SetId(4, center_id)

            unstructured_grid.InsertNextCell(
                vtk.VTK_PYRAMID, point_ids)

    points = vtk.vtkPoints()
    points.SetNumberOfPoints(len(coords))
    for i, c in enumerate(coords):
        points.SetPoint(i, c[0], c[1], c[2])

    unstructured_grid.SetPoints(points)

    append_point_data(unstructured_grid, time_step)

    return unstructured_grid


def make_wedge_only_mesh(dimensions, origin=[0, 0, 0], time_step=0):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

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

        for h in WEDGE_IDS:
            point_ids = vtk.vtkIdList()
            point_ids.SetNumberOfIds(6)
            for j, x in enumerate(h):
                point_ids.SetId(j, cell.GetPointId(x))

            unstructured_grid.InsertNextCell(vtk.VTK_WEDGE, point_ids)

    points = vtk.vtkPoints()
    points.SetNumberOfPoints(len(coords))
    for i, c in enumerate(coords):
        points.SetPoint(i, c[0], c[1], c[2])

    unstructured_grid.SetPoints(points)

    append_point_data(unstructured_grid, time_step)

    return unstructured_grid


def make_image_data(dimensions, origin=[0, 0, 0], time_step=0):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

    append_point_data(image_data, time_step)

    return image_data


def make_quad_mesh(dimensions, origin=[0, 0, 0]):
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(dimensions)
    image_data.SetOrigin(origin)

    geometry_filter = vtk.vtkGeometryFilter()
    geometry_filter.SetInputData(image_data)
    geometry_filter.Update()

    return geometry_filter.GetOutput()
