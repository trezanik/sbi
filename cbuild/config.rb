#!/usr/bin/env ruby
require_relative 'cbuild.rb' # only for string .colors - workaround
# Application-specific; these are pre-build configuration options (like ./configure)
# We the process the command line, overwriting these default settings. They get
# written out to $project/src/build_config.h, which is included in every source
# file automatically on compilation.
# Make sure these match the source code definition equivalents!
DEBUG = false
USING_BOOST_IPC = false
USING_BOOST_NET = false
USING_OPENSSL_NET = false
USING_MEMORY_DEBUGGING = false
USING_DEFAULT_QT5_GUI = false
USING_LIBCONFIG = false
USING_JSON_CONFIG = false
USING_API_WARNINGS = false
SET_COMPILER = "clang++"
BUILDCONFIG_FILE = "../src/build_config.h"
# The remainder of these are non-modifiable (or should be, depending).
CLEAN_PROJECT = false
FORCE_REBUILD = false
CLEAR_CACHES = false
if RUBY_PLATFORM =~ /mswin32/ then
	IS_WINDOWS_BUILD = true
else
	IS_WINDOWS_BUILD = false
end

def print_help()
	puts "Specify options on the command line on invocation to alter the build (like ./configure parameters do)."
	puts "All options are case-sensitive, except this help message."
	puts "See http://www.trezanik.org/projects/cbuild/manual.html for a more descriptive and complete summary of options."
	puts ""
	puts "By default, all options are off/false; this includes compulsory choices for certain libraries."
	puts "e.g. USING_OPENSSL_NET or USING_BOOST_NET must be set; the build will fail otherwise."
	puts "This enables additional future libraries to be available"
	puts ""
	puts "(define) options detect if they are present anywhere in the command line"
	puts "(value) options require an equals sign, followed by a relevant setting"
	puts ""
	puts "DEBUG  (define)"
	puts "    - Sets the active configuration to Debug."
	puts "    - Debug symbols are generated and profiling is inbuilt."
	puts "    - Libraries and binaries will reside in their respective 'debug' directory."
	puts "USING_BOOST_NET  (define)"
	puts "    - Uses Boost libraries for networking functionality."
	puts "    - No alternative socket libraries (xxx_NET) can be provided."
	puts "USING_OPENSSL_NET  (define)"
	puts "    - Uses OpenSSL for networking functionality."
	puts "    - No alternative socket libraries (xxx_NET) can be provided."
	puts "USING_BOOST_IPC  (define)"
	puts "    - Uses Boost libraries for IPC functionality."
	puts "    - No alternative IPC libraries (xxx_IPC) can be provided."
	puts "USING_DEFAULT_QT5_GUI  (define)"
	puts "    - Uses the official, default Qt5 GUI."
	puts "    - No alternative GUI libraries (xxx_GUI) can be provided."
	puts "USING_LIBCONFIG  (define)"
	puts "    - Uses libconfig as the configuration file parser."
	puts "    - No alternative config libraries (xxx_CONFIG) can be provided."
	puts "USING_JSON_CONFIG  (define)"
	puts "    - Uses JSON as the configuration file parser."
	puts "    - No alternative config libraries (xxx_CONFIG) can be provided."
	puts "USING_MEMORY_DEBUGGING  (define)"
        puts "    - Activates memory debugging"
	puts "    - In brief, acts as a memory leak checker."
	puts "SET_COMPILER  (value)"
	puts "    - Overrides the default compiler (clang++)"
	puts "    - e.g. SET_COMPILER=g++"
	puts "--force-rebuild  (define)"
	puts "    - Rebuilds all Projects/Tasks from scratch, deleting any caches"
	puts "--clear-cache  (define)"
	puts "    - Clears all project and task caches"
	puts ""
	puts "Example:"
	puts "$ ruby #{$0} DEBUG USING_BOOST_NET USING_LIBCONFIG SET_COMPILER=g++"
	puts ""
end

if ARGV.length > 0
	puts "==> Reading command line options..".fg_white.bold
	ARGV.each do |arg|
	
	# @TODO:: Class these
		
	# Build modifications
		if arg == "DEBUG"
			DEBUG = true
			puts "  -> " + "Enabled Debug build".fg_yellow.bold
		elsif arg == "USING_BOOST_NET"
			USING_BOOST_NET = true
			puts "  -> " + "Enabled Boost networking".fg_yellow.bold
		elsif arg == "USING_OPENSSL_NET"
			USING_OPENSSL_NET = true
			puts "  -> " + "Enabled OpenSSL networking".fg_yellow.bold
		elsif arg == "USING_BOOST_IPC"
			USING_BOOST_IPC = true
			puts "  -> " + "Enabled Boost IPC".fg_yellow.bold
		elsif arg == "USING_DEFAULT_QT5_GUI"
			USING_DEFAULT_QT5_GUI = true
			puts "  -> " + "Enabled default Qt5 GUI".fg_yellow.bold
		elsif arg == "USING_LIBCONFIG"
			USING_LIBCONFIG = true
			puts "  -> " + "Enabled libconfig configuration".fg_yellow.bold
		elsif arg == "USING_JSON_CONFIG"
			USING_JSON_CONFIG = true
			puts "  -> " + "Enabled JSON configuration".fg_yellow.bold
		elsif arg == "USING_MEMORY_DEBUGGING"
			USING_MEMORY_DEBUGGING = true
			puts "  -> " + "Enabled memory debugging".fg_yellow.bold
		elsif arg == "USING_API_WARNINGS"
			USING_API_WARNINGS = true
			puts "  -> " + "Enabled API warnings".fg_yellow.bold
		
	# 'App' arguments
		elsif arg == "--force-rebuild"
			FORCE_REBUILD = true
			puts "  -> " + "Forcing rebuild".fg_yellow.bold
		elsif arg == "--clear-cache"
			CLEAR_CACHES = true
			puts "  -> " + "Clearing caches".fg_yellow.bold
		elsif arg == "--clean"
			CLEAN_PROJECT = true
			puts "  -> " + "Cleaning project".fg_yellow.bold
		
	# Custom build targets (@TODO:: Support)
		elsif arg == "sbi"
			#HAVE_BUILD_TARGETS = true
			#$build_targets.push("tirc")
			puts "  -> " + "Building sbi".fg_yellow.bold
		elsif arg == "api"
			#HAVE_BUILD_TARGETS = true
			puts "  -> " + "Building libapi".fg_yellow.bold
		elsif arg == "irc"
			#HAVE_BUILD_TARGETS = true
			puts "  -> " + "Building libirc".fg_yellow.bold
		elsif arg == "qt5gui"
			#HAVE_BUILD_TARGETS = true
			puts "  -> " + "Building libui-qt5".fg_yellow.bold
		
	# Others
		elsif arg.casecmp("HELP") == 0 or arg.casecmp("-h") == 0 or arg.casecmp("--HELP") == 0
			print_help()
			exit(0)
		else
			puts "  -> " + "Unknown parameter: #{arg}".fg_red.bold
		end
	end
end


# Handle anything that


content = [
	"#pragma once",
	"",
	"/*-----------------------------------------------------------------------------",
	" * auto generated by build tool - all changes will be overwritten on build",
	" *----------------------------------------------------------------------------*/",
	"",
	"// sets up compiler definitions needed before our own headers; no dependencies",
	"#include <api/compiler.h>",
	""
]

#******************************************************************************
# Configuration library
#******************************************************************************
if USING_LIBCONFIG
	content.push("// uses libconfig as the configuration library");
	content.push("#define USING_LIBCONFIG");
	content.push("");
end
if USING_JSON_CONFIG
	content.push("// uses json as the configuration library");
	content.push("#define USING_JSON_CONFIG");
	content.push("");
end
#******************************************************************************
# Networking library
#******************************************************************************
if USING_BOOST_NET
	content.push("// uses Boost as the networking library");
	content.push("#define USING_BOOST_NET");
	content.push("");
end
if USING_OPENSSL_NET
	content.push("// uses OpenSSL as the networking library");
	content.push("#define USING_OPENSSL_NET");
	content.push("");
end
#******************************************************************************
# IPC library
#******************************************************************************
if USING_BOOST_IPC
	content.push("// uses Boost as the IPC library");
	content.push("#define USING_BOOST_IPC");
	content.push("");
end
#******************************************************************************
# GUI
#******************************************************************************
if USING_DEFAULT_QT5_GUI
	content.push("// uses the official, default Qt5 GUI");
	content.push("#define USING_DEFAULT_QT5_GUI");
	content.push("");
end
#******************************************************************************
# Other definitions
#******************************************************************************
if DEBUG
	content.push("// debug build");
	content.push("#define _DEBUG 1");
	content.push("");
end
if USING_API_WARNINGS
	content.push("// enables compile-time assertions for certain types and assumptions");
	content.push("#define USING_API_WARNINGS");
	content.push("");
end
if USING_MEMORY_DEBUGGING
	content.push("// enables memory tracking and leak detection");
	content.push("#define USING_MEMORY_DEBUGGING");
	content.push("");
end
if IS_WINDOWS_BUILD
	content.push("// prevent windows warnings with certain headers");
	content.push("#define _WIN32_WINNT 0x0600");
	content.push("");
end

#******************************************************************************
# Definition conflicts (put at end of file for less visibility)
#******************************************************************************
conflict_checker = [
	"/*-----------------------------------------------------------------------------",
	" * definition conflict checker",
	" *----------------------------------------------------------------------------*/",
	"",
	"#if defined(USING_BOOST_NET) && defined(USING_OPENSSL_NET)",
	"#	error \"Boost and OpenSSL Net libraries enabled; only 1 can be used at a time\"",
	"#endif",
	"#if defined(USING_LIBCONFIG) && defined(USING_JSON_CONFIG)",
	"#	error \"libconfig and JSON config libraries enabled; only 1 can be used at a time\"",
	"#endif",
	""
]
content.push(*conflict_checker)

#******************************************************************************
# Write updated configuration to file
#******************************************************************************
puts "==> Writing build configuration to '#{BUILDCONFIG_FILE}'".fg_white.bold
File.open("#{BUILDCONFIG_FILE}", 'w') { |f|
	content.each do |str|		
		f.write("#{str}\n");
	end
}
puts "==> Finished.".fg_white.bold

