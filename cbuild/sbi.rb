#!/usr/bin/env ruby

require_relative 'config.rb'	# executes first, handles cmdline + config
require_relative 'cbuild.rb'	# build tool core, shouldn't need modification unless customizing

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

puts ""
puts ">>> cbuild; made by James Warren for the Trezanik project".fg_white.bold
puts "  -> cbuild version ".fg_white.bold + "#{$CBUILD_VERSION_MAJOR}.#{$CBUILD_VERSION_MINOR}" + " using Ruby ".fg_white.bold + RUBY_VERSION
puts ""
# In case we're executing from untested dirs, this may help troubleshooting paths
puts ">>> Current working directory: ".fg_white + Dir.pwd


# Local overrides for our configuration.
# Remember, variables with '$' prefix are cbuild internals
if DEBUG
	$build_mode = BuildMode::DEBUG
	$object_destination = "./debug/obj/"
else
	$build_mode = BuildMode::RELEASE
	$object_destination = "./release/obj/"
end

# make the output directory if it doesn't exist (@TODO can move this into cbuild.rb)
if !Dir.exists?($object_destination)
	FileUtils.mkpath($object_destination)
end

$compiler = SET_COMPILER
#$library_paths = [ "../lib/" ] # unused here; remove?
$include_paths = [ "../src/" ]
$source_extensions = [ "cc" ]
$source_globbing = true

# cbuild modifications done, now do project customizations
target = "sbi"
api_lib = "api"
irc_lib = "irc"
ui_lib = "ui-qt5"

# These are common/baseline settings; specifics should be set per-project task
if DEBUG
	target_path = "../bin/linux_x86-64/debug"
	lib_path = "../lib/linux_x86-64/debug"
	compiler_flags = [ "-D_DEBUG", "-std=c++11", "-Wall", "-g" ]
	linker_flags = [ "-L#{lib_path}", "-g" ]
else
	target_path = "../bin/linux_x86-64/release"
	lib_path = "../lib/linux_x86-64/release"
	compiler_flags = [ "-D_NDEBUG", "-std=c++11", "-Wall", "-O2" ]
	linker_flags = [ "-L#{lib_path}" ]
end



###############
# begin options


# Nothing stopping you from using pkg-config inline to add the additional
# libraries (`pkg-config --libs pkg`) - I just choose not to!

if USING_BOOST_NET
	# We don't build Boost here, so just link the libraries directly.
	boost_net_libraries = [ "boost_date_time", "boost_regex", "boost_system" ]
	boost_net_library_path = "../third-party/boost/lib"
	boost_net_include_path = "../third-party/boost"
end
if USING_OPENSSL_NET
	# We don't build OpenSSL here, so just link the libraries directly.
	openssl_net_libraries = [ "dl", "ssl", "crypto" ]
	openssl_net_library_path = "../third-party/openssl/lib"
	openssl_net_include_path = "../third-party/openssl"
end

if USING_LIBCONFIG
	# We don't build libconfig here, so just link the libraries directly.
	libconfig_libraries = [ "config++" ]
	libconfig_library_path = "../third-party/libconfig/lib"
	libconfig_include_path = "../third-party/libconfig"
end
if USING_JSON_CONFIG
end
if USING_JSON_SPIRIT_RPC
	# There's no need to build this library, is header-only
	json_spirit_include_path = "../third-party/json_spirit"
	# json_spirit requires boost
	boost_rpc_libraries = [ "boost_system" ]
	boost_rpc_library_path = "../third-party/boost/lib"
	boost_rpc_include_path = "../third-party/boost"
	# boost requires openssl
	openssl_rpc_libraries = [ "dl", "ssl", "crypto" ]
	openssl_rpc_library_path = "../third-party/openssl/lib"
	openssl_rpc_include_path = "../third-party/openssl"
end
if USING_DEFAULT_QT5_GUI
	# note: this is different from the qt5gui task!
	qt_libraries = [ "Qt5Core", "Qt5Gui", "Qt5Widgets" ]
	qt_library_path = "../../Qt/5.3/gcc_64/lib"
	qt_include_path = "../../Qt/5.3/gcc_64/include"
end


# End options
#############



#######################
# The Project Tasks themselves
#######################


# As we have dependencies across the board, and don't want a specific layout,
# create all our tasks now, then we can add them as dependencies.
# Dependencies are task names, so always have them first
project_name = "Social Bot Interface"
sbi_task_name = "sbi"
api_task_name = "api"
irc_task_name = "IRC"
qt5gui_task_name = "Qt5GUI"


sbi = ProjectTask.new(sbi_task_name)
sbi.add_dependencies( [ api_task_name ] )
sbi.set_build_type(BuildType::EXECUTABLE)
sbi.set_target_file(target)
sbi.set_target_path(target_path)
sbi.add_compiler_flags(compiler_flags)
sbi.add_linker_flags(linker_flags)
sbi.add_link_library(api_lib)
sbi.add_source_path("../src/sbi")
sbi.add_forced_include(BUILDCONFIG_FILE)
# api deps - thought api.so would cover this?? (not cbuild related)
sbi.add_link_libraries(["dl", "pthread"])
if USING_LIBCONFIG
	sbi.add_link_library_path(libconfig_library_path)
	sbi.add_link_libraries(libconfig_libraries)
end
if USING_JSON_SPIRIT_RPC
	sbi.add_include_path(json_spirit_include_path)
	sbi.add_link_library_path(boost_rpc_library_path)
	sbi.add_link_libraries(boost_rpc_libraries)
end
# /api deps




api = ProjectTask.new(api_task_name)
api.set_build_type(BuildType::SHARED_LIBRARY)
api.set_target_path(lib_path)
api.set_target_file(api_lib)
api.add_compiler_flags(compiler_flags)
api.add_linker_flags(linker_flags)
api.add_link_library("pthread")
api.add_source_path("../src/api")
api.add_forced_include(BUILDCONFIG_FILE)
if SET_COMPILER.include? "clang++"
	api.add_compiler_flags("-Wno-c++11-compat-deprecated-writable-strings")
	api.add_compiler_flags("-Wno-unused-variable")
end
if USING_LIBCONFIG
	api.add_include_path(libconfig_include_path)
	api.add_link_library_path(libconfig_library_path)
	api.add_link_library(libconfig_libraries)
end
if USING_JSON_SPIRIT_RPC
	api.add_include_path(json_spirit_include_path)
	api.add_include_path(boost_rpc_include_path)
	api.add_include_path(openssl_rpc_include_path)
	api.add_link_library_path(openssl_rpc_library_path)
	api.add_link_library(openssl_rpc_libraries)
end



irc = ProjectTask.new(irc_task_name)
irc.add_dependencies( [ api_task_name ] )
irc.set_build_type(BuildType::SHARED_LIBRARY)
irc.set_target_path(lib_path)
irc.set_target_file(irc_lib)
irc.add_source_path("../src/irc")
irc.add_compiler_flags(compiler_flags)
irc.add_linker_flags(linker_flags)
irc.add_link_library(api_lib)
irc.add_link_library("pthread")
irc.add_forced_include(BUILDCONFIG_FILE)
if SET_COMPILER.include? "clang++"
	irc.add_compiler_flags("-Wno-unused-label") # remove
	irc.add_compiler_flags("-Wno-unused-value")
	irc.add_compiler_flags("-Wno-unused-variable")
	irc.add_compiler_flags("-Wno-c++11-compat-deprecated-writable-strings")
end
if USING_BOOST_NET
	irc.add_include_path(boost_net_include_path)
	irc.add_link_library_path(boost_net_library_path)
	irc.add_link_libraries(boost_net_libraries)
end
if USING_OPENSSL_NET
	irc.add_include_path(openssl_net_include_path)
	irc.add_link_library_path(openssl_net_library_path)
	irc.add_link_libraries(openssl_net_libraries)
end
if USING_JSON_SPIRIT_RPC
	irc.add_include_path(json_spirit_include_path)
end
if USING_DEFAULT_QT5_GUI
	irc.add_include_path(qt_include_path)
	irc.add_link_library_path(qt_library_path)
	irc.add_link_libraries(qt_libraries)
end


qt5gui = ProjectTask.new(qt5gui_task_name)
qt5gui.add_dependencies( [ api_task_name ] )
qt5gui.set_build_type(BuildType::SHARED_LIBRARY)
qt5gui.set_target_path(lib_path)
qt5gui.set_target_file(ui_lib)
qt5gui.add_source_path("../src/Qt5GUI")
qt5gui.add_source_path("../src/Qt5GUI/generated") # moc files
qt5gui.add_compiler_flags(compiler_flags)
qt5gui.add_linker_flags(linker_flags)
qt5gui.add_link_library(api_lib)
qt5gui.add_forced_include(BUILDCONFIG_FILE)
qt5gui.add_include_path("../../Qt/5.3/gcc_64/include")
qt5gui.add_link_library_path("../../Qt/5.3/gcc_64/lib")
# debugs non-existent on linux by default
#if DEBUG
#	qt5gui.add_link_libraries([ "Qt5Cored", "Qt5Guid", "Qt5Widgetsd" ])
#else
	qt5gui.add_link_libraries([ "Qt5Core", "Qt5Gui", "Qt5Widgets" ])
#end
if USING_LIBCONFIG
	qt5gui.add_include_path(libconfig_include_path)
	qt5gui.add_link_library_path(libconfig_library_path)
	qt5gui.add_link_library(libconfig_libraries)
end
if USING_JSON_SPIRIT_RPC
	qt5gui.add_include_path(json_spirit_include_path)
end



# The actual project - equivalent to a Visual Studio Solution
# not mandatory, but makes working with more than 1 task much easier
social_bot_interface = Project.new(project_name)
social_bot_interface.add_task(sbi)
social_bot_interface.add_task(api)
social_bot_interface.add_task(irc)
social_bot_interface.add_task(qt5gui)

# Additional requirements to see the libraries avoiding setting rpath
# /etc/ld.so.conf.d/sbi.conf
# -> /usr/local/lib/sbi
# ldconfig


# specific order in case more than one is provided
if CLEAN_PROJECT
#	social_bot_interface.clean_project()
end
if CLEAR_CACHES
#	social_bot_interface.clear_caches()
end
if FORCE_REBUILD
#	social_bot_interface.rebuild_all()
end


#if HAVE_BUILD_TARGETS
#	$build_targets.each do |t|
#		t.compile()
#	end
#else
	# compiles all added tasks, dependency order
	social_bot_interface.build()
#end
