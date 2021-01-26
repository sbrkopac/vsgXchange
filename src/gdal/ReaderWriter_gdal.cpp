#include "ReaderWriter_gdal.h"

#include <gdal_priv.h>

#include <iostream>

using namespace vsgXchange;

ReaderWriter_gdal::ReaderWriter_gdal()
{
}

ReaderWriter_gdal::~ReaderWriter_gdal()
{
}

void ReaderWriter_gdal::initGDAL()
{
    GDALAllRegister();
}

vsg::ref_ptr<vsg::Object> ReaderWriter_gdal::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    initGDAL();

    std::cout<<"ReaderWriter_gdal::read("<<filename<<")"<<std::endl;
    return {};
}
