﻿# We can use aliases, but the names actually correspond to the definitions already...
Param (
    [switch]$DEBUG = $false,
    [switch]$USING_BOOST_IPC = $false,
    [switch]$USING_BOOST_NET = $false,
    [switch]$USING_OPENSSL_NET = $false,
    [switch]$USING_MEMORY_DEBUGGING = $false,
    [switch]$USING_DEFAULT_QT5_GUI = $false,
    [switch]$USING_LIBCONFIG = $false,
    [switch]$USING_JSON_CONFIG = $false,
    [switch]$USING_API_WARNINGS = $false
)

Push-Location

$BUILDCONFIG_FILE = "../src/build_config.h"
# Powershell, so assume Windows build
$IS_WINDOWS_BUILD = $true


# Process command line, override defaults




$content = @(
    "#pragma once",
    "",
    "/*-----------------------------------------------------------------------------",
    " * auto generated by build tool - all changes will be overwritten on build",
    " *----------------------------------------------------------------------------*/",
    "",
    "// sets up compiler definitions needed before our own headers; no dependencies",
    "#include <api/compiler.h>",
    ""
)


#******************************************************************************
# Configuration library
#******************************************************************************
if ( $USING_LIBCONFIG )
{
    $content += , @(
        "// uses libconfig as the configuration library",
        "#define USING_LIBCONFIG",
        ""
    )
}
if ( $USING_JSON_CONFIG )
{
    $content += , @(
        "// uses json as the configuration library",
        "#define USING_JSON_CONFIG",
        ""
    )
}
#******************************************************************************
# Networking library
#******************************************************************************
if ( $USING_BOOST_NET )
{
    $content += , @(
    	"// uses Boost as the networking library",
    	"#define USING_BOOST_NET",
    	""
    )
}
if ( $USING_OPENSSL_NET )
{
    $content += , @(
    	"// uses OpenSSL as the networking library",
    	"#define USING_OPENSSL_NET",
    	""
    )
}
#******************************************************************************
# IPC library
#******************************************************************************
if ( $USING_BOOST_IPC )
{
    $content += , @(
    	"// uses Boost as the IPC library",
    	"#define USING_BOOST_IPC",
    	""
    )
}
#******************************************************************************
# GUI
#******************************************************************************
if ( $USING_DEFAULT_QT5_GUI )
{
    $content += , @(
    	"// uses the official, default Qt5 GUI",
    	"#define USING_DEFAULT_QT5_GUI",
    	""
    )
}
#******************************************************************************
# Other definitions
#******************************************************************************
if ( $DEBUG )
{
    $content += , @(
        "// debug build",
    	"#define _DEBUG 1",
    	""
    )
}
if ( $USING_API_WARNINGS )
{
    $content += , @(
    	"// enables compile-time assertions for certain types and assumptions",
    	"#define USING_API_WARNINGS",
    	""
    )
}
if ( $USING_MEMORY_DEBUGGING )
{
    $content += , @(
    	"// enables memory tracking and leak detection",
    	"#define USING_MEMORY_DEBUGGING",
    	""
    )
}
if ( $IS_WINDOWS_BUILD )
{
    $content += , @(
    	"// prevent windows warnings with certain headers",
    	"#define _WIN32_WINNT 0x0600",
    	""
    )
}

#******************************************************************************
# Definition conflicts (put at end of file for less visibility)
#******************************************************************************
$content += , @(
    "/*-----------------------------------------------------------------------------",
    " * definition conflict checker",
    " *----------------------------------------------------------------------------*/",
    "",
    "#if defined(USING_BOOST_NET) && defined(USING_OPENSSL_NET)",
    "#	error `"Boost and OpenSSL Net libraries enabled; only 1 can be used at a time`"",
    "#endif",
    "#if defined(USING_LIBCONFIG) && defined(USING_JSON_CONFIG)",
    "#	error `"libconfig and JSON config libraries enabled; only 1 can be used at a time`"",
    "#endif",
    ""
)

#******************************************************************************
# Write updated configuration to file
#******************************************************************************
Write-Host "==> Writing build configuration to '$BUILDCONFIG_FILE'"
$content | Out-File $BUILDCONFIG_FILE

Write-Host "==> Finished."

Pop-Location