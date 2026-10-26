#!/usr/bin/env python3
"""Generate the non-Generic VTK NetCDF sample files."""

from functools import partial
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

import MakeVTKNetCDFCFSampleData
import MakeVTKNetCDFCAMSampleData
import MakeVTKNetCDFMPASSampleData
import MakeVTKNetCDFSLACSampleData
import MakeVTKNetCDFUGRIDSampleData


FORMAT_OPTIONS = {
    "cdf-2": "-6",
    "classic": "-1",
}


def find_ncgen():
    """Return the ncgen executable or raise a useful error."""
    executable = shutil.which("ncgen")
    if executable is None:
        raise RuntimeError(
            "ncgen was not found. Install NetCDF command-line tools and ensure "
            "ncgen is on PATH."
        )
    return executable


def write_netcdf(ncgen_path, output_dir, filename, format_kind, cdl_text):
    """Create one NetCDF file through a same-filesystem temporary output."""
    try:
        format_option = FORMAT_OPTIONS[format_kind]
    except KeyError as error:
        raise ValueError(f"unsupported NetCDF format: {format_kind}") from error

    final_path = Path(output_dir) / filename
    with tempfile.TemporaryDirectory(
        prefix=f".{filename}.", dir=str(output_dir)
    ) as temporary_directory:
        temporary_dir = Path(temporary_directory)
        cdl_path = temporary_dir / f"{filename}.cdl"
        temporary_output = temporary_dir / filename
        cdl_path.write_text(cdl_text, encoding="utf-8")

        subprocess.run(
            [
                ncgen_path,
                format_option,
                "-o",
                str(temporary_output),
                str(cdl_path),
            ],
            check=True,
        )
        os.replace(temporary_output, final_path)

    return final_path


def main():
    ncgen_path = find_ncgen()
    output_dir = Path(__file__).resolve().parent / "data"
    output_dir.mkdir(parents=True, exist_ok=True)

    writer = partial(write_netcdf, ncgen_path)
    generated_files = []
    for generator in (
        MakeVTKNetCDFCFSampleData,
        MakeVTKNetCDFCAMSampleData,
        MakeVTKNetCDFMPASSampleData,
        MakeVTKNetCDFSLACSampleData,
        MakeVTKNetCDFUGRIDSampleData,
    ):
        generated_files.extend(generator.generate(output_dir, writer))

    print(f"Generated {len(generated_files)} VTK NetCDF sample files:")
    for path in generated_files:
        print(f"  {path.name}")


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        raise SystemExit(str(error)) from error
