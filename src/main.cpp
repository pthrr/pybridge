#include <fstream>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "helpers.h"
#include "types.h"
#include "version.h"

auto main( int argc, char** argv ) -> int
{
    CLI::App app{ "My awesome CLI app" };

    bool debug{ false };
    app.add_flag( "-d,--debug", debug, "Enable debug mode" );

    str config{ "config.txt" };
    app.add_option( "-c,--config", config, "Path to the config file" );

    CLI11_PARSE( app, argc, argv );

    spdlog::level::level_enum log_level{ spdlog::level::info };

    if( debug ) {
        log_level = spdlog::level::debug;
    }

    std::ifstream file{ config };
    nlohmann::json json_config{};
    str app_name{};

    try {
        file >> json_config;
        app_name = json_config["app"]["name"].get< str >();
    }
    catch( const nlohmann::json::exception& err ) {
        SPDLOG_ERROR( "Error parsing config: {}", err.what() );
        return 1;
    }

    spdlog::set_pattern( "[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] [%s:%#] %v" );
    spdlog::set_level( log_level );
    SPDLOG_INFO( "Starting {} v{} ..", app_name, version::getVersionInfo() );
    SPDLOG_INFO( "Done." );
}
