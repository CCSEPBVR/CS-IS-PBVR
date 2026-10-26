# VTK NetCDF Sample Data Generator

## Purpose

This script family creates the non-Generic NetCDF samples used by the VTK
readers in this directory. Every value is defined by a small formula or a
fixed connectivity table in Python, so generation does not depend on an
external input dataset.

Existing files below data that are not among the 17 targets are left
untouched. Generic sample data is not generated.

## Script structure

MakeVTKNetCDFSampleData.py is the only execution entry point. It checks
for ncgen, creates the data directory, renders in-memory CDL into a temporary
directory, invokes ncgen, atomically replaces each destination, and prints the
generated file list.

The entry point delegates to these format-specific modules:

- MakeVTKNetCDFCFSampleData.py: CF files.
- MakeVTKNetCDFCAMSampleData.py: CAM point and connectivity files.
- MakeVTKNetCDFMPASSampleData.py: MPAS files.
- MakeVTKNetCDFSLACSampleData.py: SLAC mode and volume files.
- MakeVTKNetCDFUGRIDSampleData.py: UGRID files.

No Python package installation is required. The scripts use only the Python
standard library. The NetCDF command-line utility ncgen is required.

## Check that ncgen is available

~~~sh
command -v ncgen
ncgen -h
~~~

The first command should print the executable path. The second command should
print the ncgen usage information.

## Generate the samples

~~~sh
cd Server/KVSMLConverter/Example/Input/VTKSampleData
python3 MakeVTKNetCDFSampleData.py
~~~

The generator writes or replaces only the following 17 files:

### CF

Three rectilinear structured-grid volume snapshots for time indices 0, 1, and
2.

- vtkNetCDFCFReader_0.nc
- vtkNetCDFCFReader_1.nc
- vtkNetCDFCFReader_2.nc

### CAM

One shared two-cell connectivity file and three point-field snapshots for time
indices 0, 1, and 2.

- vtkNetCDFCAMReader_connectivity.nc
- vtkNetCDFCAMReader_point_0.nc
- vtkNetCDFCAMReader_point_1.nc
- vtkNetCDFCAMReader_point_2.nc

### MPAS

Three fixed-grid multilayer samples for time indices 0, 1, and 2.

- vtkMPASReader_0.nc
- vtkMPASReader_1.nc
- vtkMPASReader_2.nc

### SLAC

Three linear-temperature mode files with frequencies 0, 1, and 2, plus one
tetrahedral volume mesh file.

- vtkSLACReader_mode_0.ncdf
- vtkSLACReader_mode_1.ncdf
- vtkSLACReader_mode_2.ncdf
- vtkSLACReader_volume.ncdf

### UGRID

Three unstructured polygon-mesh snapshots for time values 0, 31, and 62 days.

- vtkNetCDFUGRIDReader_0.nc
- vtkNetCDFUGRIDReader_1.nc
- vtkNetCDFUGRIDReader_2.nc

CF, CAM, MPAS, UGRID, and SLAC mode output use CDF-2 (64-bit offset).
The SLAC volume output uses CDF-1 (classic). The SLAC volume tetrahedron
connectivity is stored as fixed Python values rather than recomputed by a
triangulation library.

## Confirm the 17 files exist

The directory may also contain other pre-existing samples, so check the target
names explicitly:

~~~sh
python3 - <<'PY'
from pathlib import Path

names = [
    "vtkNetCDFCFReader_0.nc",
    "vtkNetCDFCFReader_1.nc",
    "vtkNetCDFCFReader_2.nc",
    "vtkNetCDFCAMReader_connectivity.nc",
    "vtkNetCDFCAMReader_point_0.nc",
    "vtkNetCDFCAMReader_point_1.nc",
    "vtkNetCDFCAMReader_point_2.nc",
    "vtkMPASReader_0.nc",
    "vtkMPASReader_1.nc",
    "vtkMPASReader_2.nc",
    "vtkSLACReader_mode_0.ncdf",
    "vtkSLACReader_mode_1.ncdf",
    "vtkSLACReader_mode_2.ncdf",
    "vtkSLACReader_volume.ncdf",
    "vtkNetCDFUGRIDReader_0.nc",
    "vtkNetCDFUGRIDReader_1.nc",
    "vtkNetCDFUGRIDReader_2.nc",
]
missing = [name for name in names if not (Path("data") / name).is_file()]
if missing:
    raise SystemExit("Missing: " + ", ".join(missing))
print("All 17 target files are present.")
PY
~~~

## Confirm the file format and schema

~~~sh
for f in data/vtkNetCDFCFReader_*.nc \
         data/vtkNetCDFCAMReader_*.nc \
         data/vtkMPASReader_*.nc \
         data/vtkSLACReader_mode_*.ncdf \
         data/vtkNetCDFUGRIDReader_*.nc \
         data/vtkSLACReader_volume.ncdf
do
    file "$f"
done
~~~

The file output should identify the first 16 files as NetCDF 64-bit offset
and vtkSLACReader_volume.ncdf as classic NetCDF.

Print the schema of every generated target:

~~~sh
for f in data/vtkNetCDFCFReader_*.nc \
         data/vtkNetCDFCAMReader_*.nc \
         data/vtkMPASReader_*.nc \
         data/vtkSLACReader_mode_*.ncdf \
         data/vtkNetCDFUGRIDReader_*.nc \
         data/vtkSLACReader_volume.ncdf
do
    echo "===== $f"
    ncdump -h "$f"
done
~~~

If a reference copy is available, compare its ncdump output with the
generated file. Save the reference output before regenerating a file, then
compare, for example:

~~~sh
ncdump -h reference/vtkNetCDFCFReader_0.nc > /tmp/reference_cf.h
ncdump -h data/vtkNetCDFCFReader_0.nc > /tmp/generated_cf.h
diff -u /tmp/reference_cf.h /tmp/generated_cf.h
~~~

The comparison can be extended from ncdump -h to ncdump when data values
also need to be checked. The generator never reads NetCDF files from data or
from another directory, and it does not create Generic files.
