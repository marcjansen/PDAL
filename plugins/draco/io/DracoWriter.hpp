/******************************************************************************
* Copyright (c) 2020, Hobu, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include <pdal/Writer.hpp>
#include <nlohmann/json.hpp>

#include <draco/point_cloud/point_cloud.h>
#include <draco/compression/encode.h>
#include <draco/attributes/geometry_attribute.h>
#include <draco/attributes/point_attribute.h>
#include <draco/core/vector_d.h>
#include "draco/compression/decode.h"
#include "draco/compression/encode.h"
#include "draco/attributes/attribute_quantization_transform.h"

namespace pdal
{
    typedef std::shared_ptr<std::ostream> FileStreamPtr;

class PDAL_DLL DracoWriter : public Writer/*, public Streamable*/
{
public:

    DracoWriter();
    ~DracoWriter();
    std::string getName() const;
private:
    virtual void addArgs(ProgramArgs& args);
    virtual void initialize(PointTableRef table);
    virtual void ready(PointTableRef table);
    virtual void write(const PointViewPtr view);
    virtual void done(PointTableRef table);

    bool flushCache(size_t size);
    void addAttribute(draco::GeometryAttribute::Type t, int n);
    void addGeneric(Dimension::Id pt);
    void initPointCloud(point_count_t size);
    void addPoint(int attId, Dimension::IdList idList, PointRef &point, PointId idx);
    draco::GeometryAttribute::Type getGeometryAttribute(std::string s);
    void parseDimensions();
    void parseQuants();

    template <typename T>
    void addToPointCloud(int attId, Dimension::IdList idList, PointRef &point, PointId idx)
    {
        point.setPointId(idx);
        const auto pointId = draco::PointIndex(idx);
        //get point information, N dimensional
        std::vector<T> pointData;
        for (size_t i = 0; i < idList.size(); ++i) {
            T data = point.getFieldAs<T>(idList[i]);
            pointData.push_back(data);
        }

        draco::PointAttribute *const att = m_pc->attribute(attId);
        att->SetAttributeValue(att->mapped_index(pointId), pointData.data());
    }

    std::string m_filename;
    NL::json m_userDimJson;
    std::map<Dimension::Id, std::string> m_userDimMap;
    //these are the default quanitization levels. They will be overridden by any
    //quantization levels specified in the json argument "quantization"
    NL::json m_userQuant;
    std::map<std::string, int> m_quant =
    {
        { "POSITION",  11 },
        { "NORMAL",     7 },
        { "TEX_COORD", 10 },
        { "COLOR",      8 },
        { "GENERIC",    8 }
    };

    FileStreamPtr m_stream;
    //map of known draco attributes to the number of dimensions associated
    //eg m_dims[POSITION]=3
    std::map<draco::GeometryAttribute::Type, int> m_dims;
    //map of draco attributes to their attribute id
    std::map<draco::GeometryAttribute::Type, int32_t> m_attMap;
    //map of generic attributes to their attribute id
    std::map<Dimension::Id, int32_t> m_genericMap;
    Dimension::IdList m_genericDims;

    std::unique_ptr<draco::PointCloud> m_pc =
        std::unique_ptr<draco::PointCloud>(new draco::PointCloud());
    int m_precision;

    DracoWriter(const DracoWriter&) = delete;
    DracoWriter& operator=(const DracoWriter&) = delete;
};

} // namespace pdal
