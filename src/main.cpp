#include <fstream>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

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

    Py_Initialize();

    std::string add_path =
        std::string( "import sys; sys.path.append(\"" ) + json_config["app"]["path"].get< str >() + "\")";
    PyRun_SimpleString( add_path.c_str() );

    PyObject* pName = PyUnicode_DecodeFSDefault( json_config["app"]["module"].get< str >().c_str() );
    PyObject* pModule = PyImport_Import( pName );
    Py_DECREF( pName );

    if( pModule != nullptr ) {
        PyObject* pClass =
            PyObject_GetAttrString( pModule, json_config["app"]["class"].get< str >().c_str() );

        if( pClass and PyCallable_Check( pClass ) ) {
            PyObject* pInstance = PyObject_CallObject( pClass, Py_BuildValue( "(i)", 10 ) );

            if( pInstance != nullptr ) {
                PyObject* pValue = PyObject_CallMethod( pInstance, "get_value", nullptr );

                if( pValue != nullptr ) {
                    SPDLOG_INFO( "Value: {}", PyLong_AsLong( pValue ) );
                    Py_DECREF( pValue );
                }
                else {
                    PyErr_Print();
                }

                PyObject_CallMethod( pInstance, "set_value", "(i)", 20 );
                pValue = PyObject_CallMethod( pInstance, "get_value", nullptr );

                if( pValue != nullptr ) {
                    SPDLOG_INFO( "Updated Value: {}", PyLong_AsLong( pValue ) );
                    Py_DECREF( pValue );
                }
                else {
                    PyErr_Print();
                }

                Py_DECREF( pInstance );
            }
            else {
                PyErr_Print();
            }

            Py_DECREF( pClass );
        }
        else {
            PyErr_Print();
        }

        Py_DECREF( pModule );
    }
    else {
        PyErr_Print();
    }

    Py_Finalize();
    SPDLOG_INFO( "Done." );
}
