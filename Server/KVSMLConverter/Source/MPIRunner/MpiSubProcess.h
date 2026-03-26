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
#ifndef CVT__MPI_SUB_PROCESS_H_INCLUDE
#define CVT__MPI_SUB_PROCESS_H_INCLUDE

#include <vtkMPIController.h>
#include <vtkSmartPointer.h>

#include "mpi.h"

#include <functional>
#include <optional>

#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"
#include "MPIRunner/MpiTag.h"

namespace cvt
{
/**
 * A task runner in a MPI sub process.
 */
class MpiSubProcess
{
public:
    /**
     * Construct a task runner.
     *
     * \param[in] argc argc
     * \param[in] argv argv
     * \param[in] main_process_id The main process ID (rank) to be managed this.
     */
    MpiSubProcess( int argc, char** argv, int main_process_id = 0 );

public:
    /**
     * Start to run tasks which be sent from the main process.
     *
     * \param[in] f A task.
     */
    void execute(
        std::function<std::optional<cvt::ConverterTaskOutput>( cvt::ConverterTaskInput )>&& f );

private:
    vtkSmartPointer<vtkMPIController> controller;
    int main_process_id;
};
} // namespace cvt

#endif