#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "test/io_helpers.h"

#include "parsing/IfcLoader.h"
#include "schema/IfcSchemaManager.h"
#include "geometry/IfcGeometryProcessor.h"
#include "utility/LoaderError.h"
#include "utility/LoaderSettings.h"
// #include "schema/ifc-schema.h"

using namespace webifc::io;

std::string ReadFile(std::string filename)
{
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

long long get_time()
{
    using namespace std::chrono;
    milliseconds millis = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch());

    return millis.count();
}

void print(std::string message)
{
    std::cout << message << std::endl;
}

int main()
{
    std::string ifc_file_path = "/Users/sebastian/Desktop/InfraByte/__data/ifc/AC-20-Smiley-West-10-Bldg.ifc";
    std::string content = ReadFile(ifc_file_path);

    webifc::utility::LoaderSettings set;
    set.COORDINATE_TO_ORIGIN = true;
    set.OPTIMIZE_PROFILES = true;

    webifc::utility::LoaderErrorHandler errorHandler;
    webifc::schema::IfcSchemaManager schemaManager;
    webifc::parsing::IfcLoader loader(set.TAPE_SIZE, set.MEMORY_LIMIT, errorHandler, schemaManager);

    auto start_time = get_time();

    loader.LoadFile([&](char *dest, size_t sourceOffset, size_t destSize)
                    {
                        uint32_t length = std::min(content.size() - sourceOffset, destSize);
                        memcpy(dest, &content[sourceOffset], length);
                        
                        return length; });

    auto elapsed_time = get_time() - start_time;

    print("Reading took: " + std::to_string(elapsed_time) + "ms");

    webifc::geometry::IfcGeometryProcessor geometryLoader(loader, errorHandler, schemaManager, set.CIRCLE_SEGMENTS, set.COORDINATE_TO_ORIGIN, set.OPTIMIZE_PROFILES);
    std::vector<webifc::geometry::IfcFlatMesh> meshes;
    webifc::schema::IfcSchemaManager schema;

    auto sum_of_all_elements = 0;
    for (auto typeCode : schema.GetIfcElementList())
    {
        auto typeName = schema.IfcTypeCodeToType(typeCode);
        print("Getting " + typeName + " objects");

        auto elements = loader.GetExpressIDsWithType(typeCode);

        for (unsigned int i = 0; i < elements.size(); i++)
        {
            auto elementId = elements[i];

            auto mesh = geometryLoader.GetFlatMesh(elements[i]);
        }
        sum_of_all_elements += elements.size();
    }
    print("Total number of elements: " + std::to_string(sum_of_all_elements));
    // loader.
    // auto all = loader.GetExpressIDsWithType(webifc::schema::IFCROOT);
    // print(std::to_string(all.size()) + " objects loaded");

    // Geometry test
    // webifc::geometry::IfcGeometryProcessor geometryLoader(loader, errorHandler, schemaManager, set.CIRCLE_SEGMENTS, set.COORDINATE_TO_ORIGIN, set.OPTIMIZE_PROFILES);

    // start_time = get_time();

    // auto walls = loader.GetExpressIDsWithType(webifc::schema::IFCSLAB);

    // bool writeFiles = true;

    // auto mesh = geometryLoader.GetMesh(203941);

    // if (writeFiles)
    // {
    //     DumpMesh(mesh, geometryLoader, "TEST.obj");
    // }

    // auto errors = errorHandler.GetErrors();
    // errorHandler.ClearErrors();

    // for (auto error : errors)
    // {
    //     std::cout << error.expressID << " " << error.ifcType << " " << std::to_string((int)error.type) << " " << error.message << std::endl;
    // }

    // elapsed_time = get_time() - start_time;

    // print("Generating geometry took " + std::to_string(elapsed_time) + "ms");

    print("Done");
}