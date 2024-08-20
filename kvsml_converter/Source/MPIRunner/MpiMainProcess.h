/*
 * Copyright (c) 2023 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CVT__MPI_MAIN_PROCESS_H_INCLUDE
#define CVT__MPI_MAIN_PROCESS_H_INCLUDE

#include <functional>
#include <optional>

#include <vtkMPIController.h>
#include <vtkSmartPointer.h>

#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"
#include "Filesystem.h"

namespace cvt
{
/**
 * A task runner manager and a task runner itself in the MPI main process.
 *
 * This class will send a distributed file configurations to sub processes.
 * Sub processes should convert a file or run some tasks using a passed value.
 */
class MpiMainProcess
{
public:
    /**
     * Construct a task runner manager.
     *
     * \param[in] argc argc
     * \param[in] argv argv
     */
    MpiMainProcess( int argc, char** argv );

public:
    /**
     * Execute a task.
     *
     * \param[in] f A task to execute in the main process.
     */
    void execute(
        std::function<std::optional<cvt::ConverterTaskOutput>( cvt::ConverterTaskInput )>&& f =
            nullptr );

private:
    vtkSmartPointer<vtkMPIController> controller;
    std::filesystem::path input_xml_path;
};
} // namespace cvt

#endif