#include "ReaderWriter_osg.h"

#include <osg/TransferFunction>
#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/MeshOptimizers>
#include <osgUtil/Optimizer>

#include "ConvertToVsg.h"
#include "ImageUtils.h"
#include "Optimize.h"
#include "SceneBuilder.h"

#include <iostream>

using namespace vsgXchange;

ReaderWriter_osg::ReaderWriter_osg()
{
    pipelineCache = osg2vsg::PipelineCache::create();
}

bool ReaderWriter_osg::readOptions(vsg::Options& options, vsg::CommandLine& arguments) const
{
    if (arguments.read("--original")) options.setValue("original", true);

    std::string filename;
    if (arguments.read("--read-osg", filename))
    {
        auto defaults = vsg::read(filename);
        std::cout << "vsg::read(" << filename << ") defaults " << defaults << std::endl;
        if (defaults)
        {
            options.setObject("osg", defaults);
        }
    }

    if (arguments.read("--write-osg", filename))
    {
        auto defaults = osg2vsg::BuildOptions::create();
        if (vsg::write(defaults, filename))
        {
            std::cout << "Written osg2vsg defaults to: " << filename << std::endl;
        }
    }
    return false;
}

vsg::ref_ptr<vsg::Object> ReaderWriter_osg::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    osg::ref_ptr<osgDB::Options> osg_options;

    if (options)
    {
        if (osgDB::Registry::instance()->getOptions())
            osg_options = osgDB::Registry::instance()->getOptions()->cloneOptions();
        else
            osg_options = new osgDB::Options();
        osg_options->getDatabasePathList().insert(osg_options->getDatabasePathList().end(), options->paths.begin(), options->paths.end());
    }

    osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(filename, osg_options.get());
    if (!object)
    {
        return {};
    }

    if (osg::Node* osg_scene = object->asNode(); osg_scene != nullptr)
    {
        vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH"); // TODO, use the vsg::Options ?

        // check to see if osg_options have been assigned to vsg::Options
        auto default_options = options->getObject<osg2vsg::BuildOptions>("osg");

        // clone the osg specific buildOptions if they have been assign to vsg::Options
        vsg::ref_ptr<osg2vsg::BuildOptions> buildOptions;
        if (default_options)
            buildOptions = osg2vsg::BuildOptions::create(*default_options);
        else
            buildOptions = osg2vsg::BuildOptions::create();

        buildOptions->options = options;
        buildOptions->pipelineCache = pipelineCache;

        if (bool original_conversion = false; options->getValue("original", original_conversion) && original_conversion)
        {
            osg2vsg::SceneBuilder sceneBuilder(buildOptions);
            auto vsg_scene = sceneBuilder.optimizeAndConvertToVsg(osg_scene, searchPaths);
            return vsg_scene;
        }
        else
        {
            vsg::ref_ptr<vsg::StateGroup> inheritedStateGroup;

            osg2vsg::ConvertToVsg sceneBuilder(buildOptions, inheritedStateGroup);

            sceneBuilder.optimize(osg_scene);
            auto vsg_scene = sceneBuilder.convert(osg_scene);

            if (sceneBuilder.numOfPagedLOD > 0)
            {
                uint32_t maxLevel = 20;
                uint32_t estimatedNumOfTilesBelow = 0;
                uint32_t maxNumTilesBelow = 40000;

                uint32_t level = 0;
                for (uint32_t i = level; i < maxLevel; ++i)
                {
                    estimatedNumOfTilesBelow += std::pow(4, i - level);
                }

                uint32_t tileMultiplier = std::min(estimatedNumOfTilesBelow, maxNumTilesBelow) + 1;

                vsg::CollectDescriptorStats collectStats;
                vsg_scene->accept(collectStats);

                auto resourceHints = vsg::ResourceHints::create();

                resourceHints->maxSlot = collectStats.maxSlot;
                resourceHints->numDescriptorSets = static_cast<uint32_t>(collectStats.computeNumDescriptorSets() * tileMultiplier);
                resourceHints->descriptorPoolSizes = collectStats.computeDescriptorPoolSizes();

                for (auto& poolSize : resourceHints->descriptorPoolSizes)
                {
                    poolSize.descriptorCount = poolSize.descriptorCount * tileMultiplier;
                }

                vsg_scene->setObject("ResourceHints", resourceHints);
            }
            return vsg_scene;
        }
    }
    else if (osg::Image* osg_image = object->asImage(); osg_image != nullptr)
    {
        return osg2vsg::convertToVsg(osg_image);
    }
    else if (osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(object.get()); tf != nullptr)
    {
        auto tf_image = tf->getImage();
        auto vsg_image = osg2vsg::convertToVsg(tf_image);
        vsg_image->setValue("Minimum", tf->getMinimum());
        vsg_image->setValue("Maximum", tf->getMaximum());

        return vsg_image;
    }
    else
    {
        std::cout << "ReaderWriter_osg::readFile(" << filename << ") cannot convert object type " << object->className() << "." << std::endl;
    }

    return {};
}

bool ReaderWriter_osg::write(const vsg::Object* object, const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> /*options*/) const
{
    std::cout << "ReaderWriter_osg::writeFile(" << object->className() << ", " << filename << ") using OSG not supported yet." << std::endl;

    return false;
}
