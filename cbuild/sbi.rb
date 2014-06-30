#!/usr/bin/env ruby

require_relative 'cbuild.rb'

# [Globals]
#
# Globals apply to ALL ProjectTasks that have not already set the respective variable.
# They will not overwrite an existing value.
# By default, no globals are set - you must declare and set them manually.
# Globals that are plural MUST be arrays.
# Trailing backslashes are required for global paths, they are not modified elsewhere.
#
# $build_mode = BuildMode::Debug
#	allows toggling all the tasks build modes editing a single location
# $compiler = "compiler_name"		
#	equivalent to project.set_compiler("compiler_name") where it is not already set
# $library_paths = [ "./path/" ]
#	enables common search paths for all libraries requiring linking
# $include_paths = [ "./path/" ]
#	enables common search paths for all tasks needing header files via '<>' or '""'
# $object_destination = "./path/"
#	enables a common destination for all object files. Be wary of name conflicts
# $source_extensions = [ ".cc", ".cpp" ]
#	used when searching for source files in source paths
# $source_globbing = true
#	use source_paths to search for source_files
#

# Don't want any output text (except that of the compiler)? Just call this with '>/dev/null'

# Application-specific; these are pre-build configuration options (like ./configure)
# We the process the command line, overwriting the default settings. If you don't
# want anything configurable, you can skip over this entire section.
IS_DEBUG = false
USING_json = true
USING_OPENSSL = true
USING_MEMORY_DEBUGGING = false
USING_GUI = false
HAVE_BUILD_TARGETS = false
VERIFY_STATIC_ASSERTS = false


def print_help()
	puts "Specify options on the command line on invocation to alter the build (like ./configure parameters do)."
	puts "All options are case-sensitive, except this help message."
	puts "See http://www.trezanik.org/projects/cbuild/manual.html for a more descriptive and complete summary of options."
	puts ""
	puts "(define) options detect if they are present anywhere in the command line"
	puts "(value) options require an equals sign, followed by a relevant setting"
	puts ""
	puts "DEBUG  (define)"
	puts "    - Sets the active configuration to Debug."
	puts "    - Debug symbols are generated and profiling is inbuilt."
	puts "    - Library and binary file names will suffix '_d'."
	puts "NO_OPENSSL  (define)"
	puts "    - Does not use OpenSSL."
	puts "    - An alternative socket library must be provided"
	puts "NO_json  (define)"
	puts "    - Does not use json."
	puts "    - An alternative XML parser or configuration library must be provided"
	puts "MEMORY_DEBUGGING  (define)"
        puts "    - Activates memory debugging"
	puts "--force-rebuild  (define)"
	puts "    - Rebuilds all Projects/Tasks from scratch, deleting any caches"
	puts "--clear-cache  (define)"
	puts "    - Clears all project and task caches"
	puts ""
	puts "Example:"
	puts "$ ruby #{$0} DEBUG MEMORY_DEBUGGING"
	puts ""
end

if ARGV.length > 0
	puts "==> Reading command line options..".fg_white.bold
	ARGV.each do |arg|
	
	# TODO:: Class these
	# TODO:: actually make these modify the build_config.h!
	
	# Build modifications
		if arg == "DEBUG"
			IS_DEBUG = true
			puts "  -> " + "Enabled Debug build".fg_yellow.bold
		elsif arg == "NO_OPENSSL"
			USING_OPENSSL = false
			puts "  -> " + "Not using OpenSSL".fg_yellow.bold
		elsif arg == "NO_JSON"
			USING_JSON = false
			puts "  -> " + "Not using json".fg_yellow.bold
		elsif arg == "GUI"
			USING_GUI = true
			puts "  -> " + "Enabled GUI".fg_yellow.bold
		
	# 'App' arguments
		elsif arg == "--force-rebuild"
			puts "  -> " + "Forcing rebuild".fg_yellow.bold
		elsif arg == "--clear-cache"
			puts "  -> " + "Clearing caches".fg_yellow.bold
		
	# Custom build targets (TODO: Support)
		elsif arg == "tirc"
			#HAVE_BUILD_TARGETS = true
			#$build_targets.push("tirc")
			puts "  -> " + "Building tirc".fg_yellow.bold
		elsif arg == "cli"
			#HAVE_BUILD_TARGETS = true
			puts "  -> " + "Building libcli".fg_yellow.bold
		elsif arg == "irc"
			#HAVE_BUILD_TARGETS = true
			puts "  -> " + "Building libirc".fg_yellow.bold
		
	# Others
		elsif arg.casecmp("HELP") == 0 or arg.casecmp("-h") == 0 or arg.casecmp("--HELP") == 0
			print_help()
			exit(0)
		else
			puts "  -> " + "Unknown parameter: #{arg}".fg_red.bold
		end
	end
	puts ">>> Finished reading command line options".fg_white.bold
end

puts ""
puts ">>> cbuild; made by James Warren for the Trezanik project".fg_white.bold
puts "  -> cbuild version ".fg_white.bold + "#{$CBUILD_VERSION_MAJOR}.#{$CBUILD_VERSION_MINOR}" + " using Ruby ".fg_white.bold + RUBY_VERSION
puts ""
# In case we're executing from untested dirs, this may help troubleshooting paths
puts ">>> Current working directory: ".fg_white + Dir.pwd


# Optional: Set our globals to try and keep DRY
if IS_DEBUG
	$build_mode = BuildMode::DEBUG
else
	$build_mode = BuildMode::RELEASE
end

$compiler = "g++"
$library_paths = [ "../lib/" ]
	# our sub-folder sources include from the parent using raw "name.h"; 
	# setting this enables us to not have to modify any sources
$include_paths = [ "../src/" ]
$object_destination = "./obj/"
$source_extensions = [ "cc" ]
$source_globbing = true


# Write out a build configuration based on our inputs here.
# Resides in the root of the source folder, "build_config.h"




# Local overrides for our configuration - marks debug files with extra suffix, changes flags
if IS_DEBUG
	target_path = "../bin/x86_64/debug"
	target = "sbi_d"
	api_lib = "api_d"
	irc_lib = "irc_d"
	compiler_flags = [ "-D_DEBUG", "-std=c++11", "-Wall", "-p", "-g" ]
	linker_flags = [ "-g" ]
else
	target_path = "../bin/x86_64/release"
	target = "sbi"
	api_lib = "api"
	irc_lib = "irc"
	compiler_flags = [ "-D_NDEBUG", "-std=c++11", "-Wall", "-O2" ]
	linker_flags = []
end




#######################
# The Project Tasks themselves
#######################


# As we have dependencies across the board, and don't want a specific layout,
# create all our tasks now, then we can add them as dependencies.
# Dependencies are task names, so always have them first
project_name = "Social Bot Interface"
sbi_task_name = "sbi"
api_task_name = "API"
irc_task_name = "IRC"


###############
# begin options


# Nothing stopping you from using pkg-config inline to add the additional
# libraries (`pkg-config --libs pkg`) - I just choose not to!

if USING_OPENSSL
	# We don't build OpenSSL here, so just link the libraries directly.
	openssl_libraries = [ "dl", "ssl", "crypto" ]
	openssl_include_path = "../third-party/openssl-1.0.1e/include"
end

if USING_JSON
	if IS_DEBUG
		json_lib = "json_d"
	else
		json_lib = "json"
	end
	json_include_path = "../third-party/json"
	json_task_name = "json"
	
	json = ProjectTask.new(json_task_name)
	json.set_build_type(BuildType::STATIC_LIBRARY)
	json.set_target_file(json_lib)
	json.set_target_path("../lib")
	json.add_source_file("../third-party/json/json/json.cpp")
	json.add_linker_flags(linker_flags)
end

if USING_QT_GUI
	if IS_DEBUG
		gui_lib = "gui_qt_d"
	else
		gui_lib = "gui_qt"
	end
	gui_task_name = "Qt5GUI"
	
	qtgui = ProjectTask.new(gui_task_name)
	qtgui.set_build_type(BuildType::SHARED_LIBRARY)
	qtgui.set_target_file(gui_lib)
	qtgui.set_target_path("../lib")
	qtgui.add_source_path("../src/Qt5GUI")
	# Qt is kept outside of 3rd party due to its installation method
	# By default, we have it in the folder above our project, in version-specific names
	# We go hacky on the source paths too (don't like their dir layout)
	qtgui.add_include_path("../../Qt/5.1.1/include")
	qtgui.add_compiler_flags(compiler_flags)
	qtgui.add_linker_flags(linker_flags)
	qtgui.add_dependency(irc_task_name)
	if USING_OPENSSL
		qtgui.add_link_libraries(openssl_libraries)
	end
	if USING_JSON
		qtgui.add_dependency(json_task_name)
		qtgui.add_include_path(json_include_path)
		qtgui.add_link_library(json_lib)
	end
end

# End options
#############


sbi = ProjectTask.new(sbi_task_name)
sbi.add_dependencies( [ cli_task_name, irc_task_name ] )
sbi.set_build_type(BuildType::EXECUTABLE)
sbi.set_target_file(target)
sbi.set_target_path(target_path)
sbi.add_compiler_flags(compiler_flags)
sbi.add_linker_flags(linker_flags)
sbi.add_source_path("../src")
sbi.add_link_library("pthread")
sbi.add_link_library(irc_lib)
sbi.add_link_library(cli_lib)
if USING_JSON
	sbi.add_include_path(json_include_path)
	sbi.add_link_library(json_lib)
	sbi.add_dependency(json_task_name)
end
if USING_OPENSSL
	sbi.add_include_path(openssl_include_path)
	sbi.add_link_libraries(openssl_libraries)
end


apicli = ProjectTask.new(api_task_name)
apicli.set_build_type(BuildType::STATIC_LIBRARY)
apicli.set_target_file(api_lib)
apicli.set_target_path("../lib")
apicli.add_source_path("../src/api")
apicli.add_compiler_flags(compiler_flags)
apicli.add_linker_flags(linker_flags)
if USING_JSON
	libcli.add_include_path(json_include_path)
	libcli.add_link_library(json_lib)
	libcli.add_dependency(json_task_name)
end


libirc = ProjectTask.new(irc_task_name)
libirc.set_build_type(BuildType::STATIC_LIBRARY)
libirc.set_target_file(irc_lib)
libirc.set_target_path("../lib")
libirc.add_source_path("../src/irc")
libirc.add_compiler_flags(compiler_flags)
libirc.add_linker_flags(linker_flags)
libirc.add_link_library("pthread")
if USING_JSON
	libirc.add_include_path(json_include_path)
	libirc.add_link_library(json_lib)
	libirc.add_dependency(json_task_name)
end
if USING_OPENSSL
	libirc.add_include_path(openssl_include_path)
	libirc.add_link_libraries(openssl_libraries)
end



# The actual project - equivalent to a Visual Studio Solution
# not mandatory, but makes working with more than 1 task much easier
trezanik = Project.new(project_name)
trezanik.add_task(sbi);
trezanik.add_task(libcli);
trezanik.add_task(libirc);
if USING_JSON
	trezanik.add_task(json)
end
if USING_GUI
	trezanik.add_task(qtgui)
end

if HAVE_BUILD_TARGETS
	$build_targets.each do |t|
		t.compile()
	end
else
	trezanik.build()	# compiles all added tasks, dependency order
end
