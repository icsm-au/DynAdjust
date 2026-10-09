#define TESTING_MAIN
#include "testing.hpp"

#include <filesystem>
#include <fstream>
#include <system_error>

#include <dynadjust/dnaimport/dnainterop.hpp>
#include <include/measurement_types/dnaheight.hpp>

using dynadjust::dynamlinterop::dna_import;
using namespace dynadjust::measurements;

namespace {
struct TempFile {
    explicit TempFile(const char* name) : path(name) {}
    ~TempFile() {
        std::error_code error;
        std::filesystem::remove(path, error);
    }
    std::string path;
};

void Parse(dna_import& importer, const std::string& filename, project_settings& settings,
           vdnaStnPtr& stations, vdnaMsrPtr& measurements, input_file_meta_t& metadata) {
    UINT32 station_count = 0, measurement_count = 0, cluster_id = 0;
    std::string status;
    importer.ParseInputFile(filename, &stations, &station_count, &measurements,
        &measurement_count, &cluster_id, &metadata, true, &status, &settings);
    REQUIRE(stations.empty());
    REQUIRE(measurements.size() == 1);
}
}

TEST_CASE("Measurement-only DNA export selects the 3.02 layout independently") {
    TempFile input("epoch-io-input301.msr");
    TempFile output("epoch-io-output302.msr");
    TempFile unused_station_file("epoch-io-unused.stn");
    dna_msr_fields locations{}, widths{};
    determineDNAMSRFieldParameters<UINT16>("3.01", locations, widths);
    CDnaHeight height;
    height.SetFirst("STN1");
    height.SetEpoch("01.01.2020");
    height.SetValue("100");
    height.SetStdDev("0.01");
    {
        std::ofstream file(input.path);
        REQUIRE(file.is_open());
        file << "!#=DNA 3.01 MSR    29.09.2026    GDA2020     01.01.2020         1\n";
        height.WriteDNAMsr(&file, widths, locations, false);
    }

    project_settings settings;
    settings.i.observation_epoch = "15.06.2015";
    settings.i.user_supplied_observation_epoch = true;
    dna_import importer;
    importer.InitialiseDatum("GDA2020", "01.01.2020", settings.i.observation_epoch);
    vdnaStnPtr stations;
    vdnaMsrPtr measurements;
    input_file_meta_t metadata{};
    Parse(importer, input.path, settings, stations, measurements, metadata);
    REQUIRE(measurements[0]->GetObservationEpoch() == "15.06.2015");
    vifm_t sources{metadata};
    importer.SerialiseDNA(&stations, &measurements, unused_station_file.path,
        output.path, settings, &sources, false);
    REQUIRE(!std::filesystem::exists(unused_station_file.path));
    {
        std::ifstream file(output.path);
        std::string header;
        std::getline(file, header);
        REQUIRE(header.find("!#=DNA 3.02 MSR") == 0);
    }

    settings.i.observation_epoch = "01.01.2025";
    dna_import reader;
    reader.InitialiseDatum("GDA2020", "01.01.2020", settings.i.observation_epoch);
    Parse(reader, output.path, settings, stations, measurements, metadata);
    REQUIRE(measurements[0]->GetObservationEpoch() == "15.06.2015");
    REQUIRE(measurements[0]->ObservationEpochSupplied());
}
