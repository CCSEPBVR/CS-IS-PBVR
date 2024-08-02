/*
 * Copyright (c) 2010 Visualization Laboratory (Koyamada Lab.), ACCMS of Kyoto University
 * Copyright (c) 2014 Naohisa Sakamoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of KVS nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright (c) 2022 Japan Atomic Energy Agency
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
#include "UnstructuredVolumeObjectExporter.h"

#include "kvs/Message"

kvs::FileFormatBase* cvt::UnstructuredVolumeObjectExporter::exec( const kvs::ObjectBase* object )
{
    BaseClass::setSuccess( true );

    if ( !object )
    {
        BaseClass::setSuccess( false );
        kvsMessageError( "Input object is NULL." );
        return nullptr;
    }

    // Cast to the structured volume object.
    const kvs::UnstructuredVolumeObject* volume = kvs::UnstructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        BaseClass::setSuccess( false );
        kvsMessageError( "Input object is not unstructured volume object." );
        return nullptr;
    }

    if ( volume->label() != "" )
    {
        this->setLabel( volume->label() );
    }
    if ( volume->unit() != "" )
    {
        this->setUnit( volume->unit() );
    }

    // Check the cell type of the given unstructured volume object.
    switch ( volume->cellType() )
    {
    case kvs::UnstructuredVolumeObject::UnknownCellType: {
        kvsMessageError( "Unknown cell type." );
        break;
    }
    case kvs::UnstructuredVolumeObject::Tetrahedra: {
        this->setCellType( "tetrahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticTetrahedra: {
        this->setCellType( "quadratic tetrahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Hexahedra: {
        this->setCellType( "hexahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticHexahedra: {
        this->setCellType( "quadratic hexahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Pyramid: {
        this->setCellType( "pyramid" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Prism: {
        this->setCellType( "prism" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Point: {
        this->setCellType( "point" );
        break;
    }
    default: {
        BaseClass::setSuccess( false );
        kvsMessageError( "Not supported cell type." );
        break;
    }
    }

    this->setVeclen( volume->veclen() );
    this->setNNodes( volume->numberOfNodes() );
    this->setNCells( volume->numberOfCells() );
    this->setValues( volume->values() );
    this->setCoords( volume->coords() );
    this->setConnections( volume->connections() );
    this->setMinMaxObjectCoords( object->minObjectCoord(), object->maxObjectCoord() );
    this->setMinMaxExternalCoords( object->minExternalCoord(), object->maxExternalCoord() );

    if ( volume->hasMinMaxValues() )
    {
        this->setMinValue( volume->minValue() );
        this->setMaxValue( volume->maxValue() );
    }

    return this;
}