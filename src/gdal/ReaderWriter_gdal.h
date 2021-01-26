#pragma once

#include <vsg/io/ReaderWriter.h>
#include <vsgXchange/Export.h>

#include <map>

namespace osg2vsg
{
    struct PipelineCache;
}

namespace vsgXchange
{

    class VSGXCHANGE_DECLSPEC ReaderWriter_gdal : public vsg::Inherit<vsg::ReaderWriter, ReaderWriter_gdal>
    {
    public:
        ReaderWriter_gdal();

        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        void initGDAL();

    protected:
        ~ReaderWriter_gdal();

    };

} // namespace vsgXchange
