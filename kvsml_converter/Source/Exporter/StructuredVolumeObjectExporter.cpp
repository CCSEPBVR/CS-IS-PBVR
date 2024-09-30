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
#include "StructuredVolumeObjectExporter.h"

#include "kvs/Message"
#include "kvs/StructuredVolumeObject"

kvs::FileFormatBase* cvt::StructuredVolumeObjectExporter::exec( const kvs::ObjectBase* object )
{
    BaseClass::setSuccess( false );

    if ( auto volume = dynamic_cast<const kvs::StructuredVolumeObject*>( object ) )
    {
        switch ( volume->gridType() )
        {
        case kvs::StructuredVolumeObject::Uniform: {
            this->setGridType( "uniform" );
            break;
        }
        case kvs::StructuredVolumeObject::Rectilinear:
            this->setGridType( "rectilinear" );
            this->setCoords( volume->coords() );
            break;
        case kvs::StructuredVolumeObject::Curvilinear:
            this->setGridType( "curvilinear" );
            this->setCoords( volume->coords() );
            break;
        default: {
            BaseClass::setSuccess( false );
            kvsMessageError( "Not supported cell type." );
            return nullptr;
        }
        }
        this->setVeclen( volume->veclen() );
        this->setResolution( volume->resolution() );
        this->setValues( volume->values() );

        if ( volume->hasMinMaxValues() )
        {
            this->setMinValue( volume->minValue() );
            this->setMaxValue( volume->maxValue() );
        }
        if ( volume->hasMinMaxExternalCoords() )
        {
            this->setMinMaxExternalCoords( volume->minExternalCoord(), volume->maxExternalCoord() );
        }
        if ( volume->hasMinMaxObjectCoords() )
        {
            this->setMinMaxObjectCoords( volume->minObjectCoord(), volume->maxObjectCoord() );
        }

        BaseClass::setSuccess( true );

        return this;
    }
    else
    {
        kvsMessageError( "Input object is not structured volume object." );
        return nullptr;
    }
}