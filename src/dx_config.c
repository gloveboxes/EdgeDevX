/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include "dx_config.h"

// Usage text for command line arguments in application manifest.
static const char *cmdLineArgsUsageText =
    "DPS connection type: \"CmdArgs:\" -s \"<your_scope_id>\" -d \"<your_device_id>\" -k \"<your_device_key>\"\n"
    "Connection string type: \"CmdArgs:\" -c \"<iot_hub_central_connection_string>\"\n";

/// <summary>
///     Parse the command line arguments given in the application manifest.
/// </summary>
bool dx_configParseCmdLineArguments(int argc, char *argv[], DX_USER_CONFIG *userConfig)
{
    bool result = true;
    int option = 0;
    static const struct option cmdLineOptions[] = {
        {.name = "ScopeId", .has_arg = required_argument, .flag = NULL, .val = 's'},
        {.name = "DeviceId", .has_arg = required_argument, .flag = NULL, .val = 'd'},
        {.name = "DeviceKey", .has_arg = required_argument, .flag = NULL, .val = 'k'},
        {.name = "ConnectionString", .has_arg = required_argument, .flag = NULL, .val = 'c'},
        {.name = "Hostname", .has_arg = required_argument, .flag = NULL, .val = 'h'},
    };

    userConfig->connectionType = DX_CONNECTION_TYPE_NOT_DEFINED;

    // Loop over all of the options.
    while ((option = getopt_long(argc, argv, "s:c:h:k:d:", cmdLineOptions, NULL)) != -1) {
        // Check if arguments are missing. Every option requires an argument.
        if (optarg != NULL && optarg[0] == '-') {
            printf("WARNING: Option %c requires an argument\n", option);
            continue;
        }
        switch (option) {
        case 'c':
            userConfig->connection_string = optarg;
            userConfig->connectionType = DX_CONNECTION_TYPE_STRING;
            break;
        case 's':
            userConfig->idScope = optarg;
            userConfig->connectionType = DX_CONNECTION_TYPE_DPS;
            break;
        case 'k':
            userConfig->device_key = optarg;
            break;
        case 'd':
            userConfig->device_id = optarg;
            break;
        case 'h':
            // printf("Hostname String: %s\n", optarg);
            userConfig->hostname = optarg;
            userConfig->connectionType = DX_CONNECTION_TYPE_HOSTNAME;
            break;
        default:
            // Unknown options are ignored.
            break;
        }
    }

    switch (userConfig->connectionType) {
    case DX_CONNECTION_TYPE_NOT_DEFINED:
        dx_terminate(DX_ExitCode_Validate_Connection_Type_Not_Defined);
        result = false;
        break;
    case DX_CONNECTION_TYPE_DPS:
        if (dx_isStringNullOrEmpty(userConfig->idScope)) {
            dx_terminate(DX_ExitCode_Validate_ScopeId_Not_Defined);
            result = false;
        }
        break;
    case DX_CONNECTION_TYPE_STRING:
        if (dx_isStringNullOrEmpty(userConfig->connection_string)) {
            dx_terminate(DX_ExitCode_Validate_Hostname_Not_Defined);
            result = false;
        }
        break;
    case DX_CONNECTION_TYPE_HOSTNAME:
        if (dx_isStringNullOrEmpty(userConfig->hostname)) {
            dx_terminate(DX_ExitCode_Validate_Hostname_Not_Defined);
            result = false;
        }
        break;
    }

    if (!result) {
        printf("%s\n", cmdLineArgsUsageText);
    }

    return result;
}