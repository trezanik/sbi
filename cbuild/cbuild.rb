#!/usr/bin/env ruby

###############################################################################
#                      THIS FILE SHOULD NOT BE MODIFIED!                      #
#         Project build scripts should be supplied alongside this one         #
###############################################################################

require 'fileutils'	# Path creation
require 'digest'	# file checksums

# suppress warnings. we don't ever really cause any, except redefining constants
# in client build scripts when overriding default options from the command line
$VERBOSE = nil

# For project build scripts to check against for features/compatibility
$CBUILD_VERSION_MAJOR = 1
$CBUILD_VERSION_MINOR = 0

# Verbosity level (includes everything below it):
# 5 - Debug - very spammy, outputs almost everything; includes old cache/new cache
# 4 - Detailed - Prints the cache contents
# 3 - (Default) Average - Notifies objects [not] up to date, cache saving
# 2 - Little - Outputs task settings, generated command lines
# 1 - Minimal - Outputs initial options and updates
# 0 - Silence - Only compiler output (its own stdout, stderr, etc.)
$verbosity = 5
#$stdout.sync = true	# needed to prevent newline issues with our 'puts.xxx' methods

# Global defaults - nil, unless an array, which are empty
$build_mode = nil
$compiler = nil
$library_paths = []
$include_paths = []
$object_destination = nil
$source_extensions = []
$source_globbing = nil

# Defines the build mode for the task
module BuildMode [ RELEASE = 'release', DEBUG = 'debug' ] end

# Defines the build type for the task
module BuildType [ EXECUTABLE = 'executable', SHARED_LIBRARY = 'shared-library', STATIC_LIBRARY = 'static-library' ] end


# Helper class and module to support our 'level' of verbosity - adds to the Kernel module
class DevNull; def write(*args) end end
module Kernel
	# for calling methods that generate output we don't want
	def puts.silence
		# can use 'unless $verbosity > x; puts string end; yield; $stdout = STDOUT' for others
		$stdout = DevNull.new
		yield
		$stdout = STDOUT
	end
	# These all come in with newlines already present (so don't use "puts string")
	def puts.minimal string
		unless $verbosity < 1
			print string
			$stdout.flush
		end
	end
	def puts.little string
		unless $verbosity < 2
			print string
			$stdout.flush
		end
	end
	def puts.average string
		unless $verbosity < 3
			print string
			$stdout.flush
		end
	end
	def puts.detailed string
		unless $verbosity < 4
			print string
			$stdout.flush
		end
	end
	def puts.debug string
		unless $verbosity < 5
			print string
			$stdout.flush
		end
	end
end


# If building a binary, this will cater the inclusions required via other
# libraries (i.e. including their object files as part of the current Task)
# doesn't really make sense, need to draw a picture for it!
class Project
	def initialize(name)
		@project_name = name
		@tasks = []
	end
	
	# This class does no compilation, so this is for 'building' the project
	def build
		puts.minimal "==> Building Project".fg_white
		
		@tasks.each do |task|
			
			unless task.built
				puts.average ">>> Found unbuilt task: " + "#{task.task_name}".fg_white.bold
			
				task.dependencies.each do |name|
					#puts "Dependency: " + "#{name}".fg_white.bold
					build_dependency(name)
				end
			
				#puts "Compiling '#{task.task_name}'".fg_red.bold
				task.compile()
			end
		end
	end
	
	def build_dependency(dep_name)
		@tasks.each do |task|
			# Locate the task name
			if task.task_name == dep_name
				# Don't build if it's already up to date
				unless task.built
					puts.minimal "==> Building dependency: " + "#{dep_name}".fg_yellow.bold
					
					# Build the dependency dependencies!
					task.dependencies.each do |d|
						build_dependency(d)
					end
					
					task.compile()
				end
			end
		end
	end
	
	def add_task(t)
		@tasks.push(t)
	end
	
	def remove_task(t)
		@tasks.delete_if {|task| task == t}
	end
end



# These are what we store:
# object.cc : (Hash of file: abcdef0123456789)
# object.o : (Hash of file: fedcba9876543210)
#
# We check the source file vs its checksum; mismatch triggers rebuild
# We check the object file vs its checksum; mismatch triggers relink
# If a source file does not exist, it is compiled and added to the list
# If an object file does not exist, the source is compiled and added to the list
#
# Multiple checksums means we can identify if a source was updated only with
# comments (or something retaining the object files checksum), and prevent 
# rebuilding the task - a waste of time and resources.
#
# Missing files or source/objects are not flagged
#
#===== cache_files
#
#=================
#
#===== recompile_list
# create_cache() adds to the recompile list on initial creation, otherwise:
# when a source is checked with its cached counterpart, if the hash mismatches,
# it is added to the recompile list, in ProjectTask.needs_recompile()
# The ProjectTask checks this list to perform compilation
#====================
#
class TaskCache
	def initialize()
		@cache_files = []
		@recompile_list = []
	end
	
	attr_accessor :cache_files
	attr_accessor :recompile_list
	
	def add_cache_entry(src_obj_hash)
		source = src_obj_hash["Source"]
		object = src_obj_hash["Object"]
		src_cs = nil
		obj_cs = nil
		
		if File.exist?(source)
			src_cs = get_file_checksum(source)
			puts_checksum_info(source, src_cs)
		end
		if File.exist?(object)
			obj_cs = get_file_checksum(object)
			puts_checksum_info(object, obj_cs)
		end
		
		entry = { "Source" => source, "SourceChecksum" => src_cs, "Object" => object, "ObjectChecksum" => obj_cs }
		@cache_files.push(entry)
	end
	
	# The arrays should come from the Task itself, which has already done
	# the work of identifying the file names, destination paths, etc.
	# They MUST complement each other - a source file must have a matching
	# object file at the SAME position in the other array
	def create_cache(src_array, obj_array)
		src_array.each_with_index do |src, i|
			obj = obj_array[i]
			# create a hash of the source->object mappings
			src_obj_hash = { "Source" => src, "Object" => obj }
			add_cache_entry(src_obj_hash)
			# add it to the recompile list (since we have no instance yet)
			@recompile_list.push(src)
		end
	end
	
	def load(filename)
		#puts "Loading cache from #{filename}"
		if File.exists?(filename)
			#data = Marshal::load(File.binread(filename))
			#@cache_files = data[0]
			#@built = data[1]
			@cache_files = Marshal::load(File.binread(filename))
			#puts "loaded cache:\n#{@cache_files}"
			return true
		else
			#puts "cache file does not exist"
			return false
		end
	end
	def save(filename)
		#data = [ @cache_files, @built ]
		#serial = Marshal.dump(data)
		serial = Marshal.dump(@cache_files)
		return true unless File.binwrite(filename, serial) == 0
	end
	
	def update_cache_source(src)
		@cache_files.each do |cache|
			if cache["Source"] == src
				puts.detailed "Updating #{src}.."
				cache["SourceChecksum"] = get_file_checksum(src)
				puts_checksum_info(cache["Source"], cache["SourceChecksum"])
			end
		end
	end
	def update_cache_object(obj)
		@cache_files.each do |cache|
			if cache["Object"] == obj
				puts.detailed "Updating #{obj}.."
				cache["ObjectChecksum"] = get_file_checksum(obj)
				puts_checksum_info(cache["Object"], cache["ObjectChecksum"])
			end
		end
	end
	
	def clear_cache
		@cache_files.clear
		@recompile_list.clear
	end
	
	def is_header_uptodate(h)
		# @TODO:: Implement
	
		return false
	end
	def is_source_uptodate(src)
		@cache_files.each do |cache|
			if cache["Source"] == src
				cs = get_file_checksum(src)
				if cs == cache["SourceChecksum"]
					puts_is_uptodate(cache["Source"])
					return true
				else
					puts_isnot_uptodate(cache["Source"])
					return false
				end
			end
		end
		
		# not found, not up to date
		return false
	end
	def is_object_uptodate(obj)
		@cache_files.each do |cache|
			if cache["Object"] == obj
				cs = get_file_checksum(obj)
				if cs == cache["ObjectChecksum"]
					puts_is_uptodate(cache["Object"])
					return true
				else
					puts_isnot_uptodate(cache["Object"])
					return false
				end
			end
		end
		
		# not found, not up to date
		return false
	end
	
	def get_file_checksum(f)
		return Digest::SHA1.hexdigest(File.read(f))
	end
	def puts_checksum_info(file, checksum)
		# some source paths are rather long, and not pertinent to the info required
		fname = File.basename(file)
		puts.debug "Checksum of ".fg_white + "#{fname}".fg_cyan.bold + " is: ".fg_white + "#{checksum}".fg_yellow.bold
	end
	def puts_is_uptodate(file)
		fname = File.basename(file)
		puts.average "#{fname} is up to date"
	end
	def puts_isnot_uptodate(file)
		fname = File.basename(file)
		puts.average "#{fname} is not up to date"
	end
	def puts_cache
		#puts "==> Outputting header cache"
		#@cache_header_files.each_with_index do |cache, i|
		#	puts "[#{i}] " + "Header File......: ".fg_white + "#{cache["Header"]}".fg_cyan.bold
		#	puts "[#{i}] " + "Header Checksum..: ".fg_white + "#{cache["HeaderChecksum"]}".fg_cyan.bold
		#end
		#puts "==> Outputting source + object cache"
		@cache_files.each_with_index do |cache, i|
			puts.debug "[#{i}] " + "Source File......: ".fg_white + "#{cache["Source"]}".fg_cyan.bold
			puts.debug "[#{i}] " + "Source Checksum..: ".fg_white + "#{cache["SourceChecksum"]}".fg_cyan.bold
			puts.debug "[#{i}] " + "Object File......: ".fg_white + "#{cache["Object"]}".fg_cyan.bold
			puts.debug "[#{i}] " + "Object Checksum..: ".fg_white + "#{cache["ObjectChecksum"]}".fg_cyan.bold
		end
	end
end



class ProjectTask
	def initialize(name)
		# All we require is a name; beyond that, we set the target path
		# as it makes sense. Everything else should be validated, and
		# modified if not valid, so retain nil to reduce complexity
		@task_name = name
		@build_mode = nil
		@build_type = nil
		@compiler = nil
		@target_file = name
		@target_path = "./"
		@object_destination = nil
		@source_glob = nil
		# All these variables support (and require) lists
		@dependencies = []
		@compiler_flags = []
		@linker_flags = []
		@link_libraries = []
		@link_library_paths = []
		@include_paths = []
		@source_extensions = []
		@source_files = []
		@source_paths = []
		@forced_inclusions = []
		# dynamic variables - non-configurable
		@built = false
		@object_files = []
		@cache = TaskCache.new
	end
	

	# Project + Cache needs to access some of our internals, read-only
	# attr_accessor -> read+write
	# attr_reader -> read-only
	# attr_writer -> write-only
	# Just make these available to all, no harm
	attr_reader :task_name
	attr_reader :dependencies
	attr_reader :build_type
	attr_reader :object_files
	attr_reader :source_files
	attr_reader :cache
	attr_reader :built
	
	
	def add_compiler_flag(f)
		@compiler_flags.push(f)
	end
	def add_compiler_flags(f)
		# appending an array to an array, do with '*var'
		@compiler_flags.push(*f)
	end
	def add_dependency(d)
		@dependencies.push(d)
	end
	def add_dependencies(d)
		@dependencies.push(*d)
	end
	def add_forced_include(i)
		@forced_inclusions.push(i)
	end
	def add_forced_includes(i)
		@forced_inclusions.push(*i)
	end
	def add_linker_flag(f)
		@linker_flags.push(f)
	end
	def add_linker_flags(f)
		@linker_flags.push(*f)
	end
	def add_link_library(l)
		@link_libraries.push(l)
	end
	def add_link_libraries(l)
		@link_libraries.push(*l)
	end
	def add_link_library_path(p)
		unless p.end_with?('/')
			p << "/" 
		end
		@link_library_paths.push(p)
	end
	def add_include_path(p)
		unless p.end_with?('/')
			p << "/" 
		end
		@include_paths.push(p)
	end
	def add_source_extension(e)
		unless e.start_with?('.')
			e.prepend(".")
		end
		@source_extensions.push(e)
	end
	def add_source_path(p)
		unless p.end_with?('/')
			p << "/" 
		end
		@source_paths.push(p)
	end
	def add_source_file(f)
		@source_files.push(f)
	end
	
	def enable_source_glob(b)
		@source_glob = b
	end
	
	def remove_compiler_flag(f)
		@compiler_flags.delete_if {|flag| flag == f}
	end
	def remove_dependency(d)
		@dependencies.delete_if {|dep| dep == d}
	end
	def remove_forced_include(i)
		@forced_inclusions.delete_if {|dep| dep == i}
	end
	def remove_linker_flag(f)
		@linker_flags.delete_if {|flag| flag == f}
	end
	def remove_link_library(l)
		@link_libraries.delete_if {|lib| lib == l}
	end
	def remove_link_library_path(p)
		@link_library_paths.delete_if {|path| path == p}
	end
	def remove_include_path(p)
		@include_paths.delete_if {|path| path == p}
	end
	def remove_source_extension(e)
		@source_extensions.delete_if {|ext| ext == e}
	end
	def remove_source_path(p)
		@source_paths.delete_if {|path| path == p}
	end
	def remove_source_file(f)
		@source_files.delete_if {|file| file == f}
	end
	
	def set_build_mode(m)
		@build_mode = m
	end
	def set_build_type(t)
		@build_type = t
	end
	def set_compiler(c)
		@compiler = c
	end
	def set_object_destination(p)
		unless p.end_with?('/')
			p << "/" 
		end
		# this path must exist; gcc will NOT make it for you if it doesn't!
		@object_destination = p
	end
	def set_target_file(f)
		@target_file = f
	end
	def set_target_path(p)
		unless p.end_with?('/')
			p << "/" 
		end
		@target_path = p
	end
	
	
	def needs_recompile()
		@cache.puts_cache()
		
		retval = false
		
		# can independently check all the headers before doing any of 
		# the sources - the trick being compiling only affected sources
		#@header_files.each do |h|
		#	if !@cache.is_header_uptodate(h)
		#		retval = true
		#	end
		#end
		
		@source_files.each do |src|
			if !@cache.is_source_uptodate(src)
				# add it to the recompile list
				@cache.recompile_list.push(src)
				retval = true
			end
		end
		
		return retval
	end
	def needs_relink()
		@object_files.each do |obj|
			if !@cache.is_object_uptodate(obj)
				return true
			end
		end
		
		return false
	end
	def needs_build()
	
	end
	
	
	def prepare()	
		# Set globals if local configuration not applied
		if !$compiler.nil? and @compiler.nil?
			@compiler = $compiler
		end
		if !$object_destination.nil? and @object_destination.nil?
			@object_destination = $object_destination
		end
		if !$library_paths.empty?	# Append the additional library paths
			@link_library_paths.push(*$library_paths)
		end
		if !$include_paths.empty?	# Append the additional include paths
			@include_paths.push(*$include_paths)
		end
		if !$source_extensions.empty? and @source_extensions.empty?
			@source_extensions = $source_extensions
		end
		if !$source_globbing.nil? and @source_glob == nil
			@source_glob = $source_globbing
		end
		if !$build_mode.nil?
			@build_mode = $build_mode
		end
		
	# 3:30 am, bitch at me later if you hate this
		raise "Invalid build mode" unless @build_mode == BuildMode::DEBUG or @build_mode == BuildMode::RELEASE
		raise "Invalid build type" unless @build_type == BuildType::EXECUTABLE or @build_type == BuildType::SHARED_LIBRARY or @build_type == BuildType::STATIC_LIBRARY
		raise "No compiler specified" if @compiler.empty?
		raise "No object file destination specified" if @object_destination.empty?
		raise "No target filename specified" if @target_file.empty?
		
		if @source_glob
			# add source glob files to the source files array
			puts.detailed ">>> Source globbing enabled".fg_white
			
			# for each source path, find any files matching the extension
			@source_paths.each do |path|
				@source_extensions.each do |ext|
					puts.average "==> Scanning ".fg_white + "#{path}".fg_yellow.bold + " for ".fg_white + "#{ext}".fg_yellow.bold
					Dir.glob("#{path}/*#{ext}") do |file|
						puts.average "  -> Found ".fg_white + "#{file}".fg_cyan.bold
						@source_files.push(file)
					end
				end
			end
			
			puts.detailed ""
		end
		# generate object file names, now we know each source
		@source_files.each do |src|
			# source file without extension
			object = "#{@object_destination}" + File.basename(src, ".*") + ".o"
			@object_files.push(object)
		end
		
		if @source_files.empty?
			raise "No source files specified"
		end
	end
	
	def print_class
		puts.detailed "Task Name..........: ".fg_white + "#{@task_name}"
		puts.detailed "Build Type.........: ".fg_white + "#{@build_type}"
		puts.detailed "Build Mode.........: ".fg_white + "#{@build_mode}"
		puts.detailed "Target File........: ".fg_white + "#{@target_file}"
		puts.detailed "Target Path........: ".fg_white + "#{@target_path}"
		puts.detailed "Compiler...........: ".fg_white + "#{@compiler}"
		puts.detailed "Compiler Flags.....: ".fg_white + "#{@compiler_flags}"
		puts.detailed "Object Destination.: ".fg_white + "#{@object_destination}"
		puts.detailed "Linker Flags.......: ".fg_white + "#{@linker_flags}"
		puts.detailed "Link Libraries.....: ".fg_white + "#{@link_libraries}"
		puts.detailed "Link Library Paths.: ".fg_white + "#{@link_library_paths}"
		puts.detailed "Forced Includes....: ".fg_white + "#{@forced_inclusions}"
		puts.detailed "Include Paths......: ".fg_white + "#{@include_paths}"
		puts.detailed "Source Glob........: ".fg_white + "#{@source_glob}"
		puts.detailed "Source Extensions..: ".fg_white + "#{@source_extensions}"
		puts.detailed "Source Paths.......: ".fg_white + "#{@source_paths}"
		puts.detailed "Source Files.......: ".fg_white + "#{@source_files}"
		puts.detailed ""
	end
	
	def basic_test
		# Test the array methods
		add_compiler_flag("-D_NDEBUG")
		add_compiler_flag("-g")
		add_compiler_flag("-std=c++11")
		raise "3 compiler flags should exist" unless @compiler_flags.size() == 3
		remove_compiler_flag("-g")
		raise "2 compiler flags should remain" unless @compiler_flags.size() == 2
		
		# Test the set methods
		set_target_file("tirc_d")
		set_target_path("../bin")
		raise "Target File should be 'tirc_d'" unless @target_file == "tirc_d"
		raise "Target Path should be '../bin/'" unless @target_path == "../bin/"
		
		# Test the enable methods
		enable_source_glob(true)
		raise "Source Glob should be true" unless @source_glob == true
		enable_source_glob(false)
		raise "Source Glob should be false" unless @source_glob == false
	end
	
	def compile
		
		# @TODO:: check for dependencies; if any, they need a cache entry
		# if we're linking statically to them, and we can rebuild the 
		# dependency or task even if none of the tasks files have changed
		
		puts.average "\n>>> Processing Task: #{@task_name}\n".fg_white.bold
		
		# acquires/generates list of source & object files to be built.
		# Do note that the source files and object files order will 
		# always remain the same from here on, so after copying a hash,
		# we can be assured the same data is in the same positions
		prepare()
		
		print_class()
		
		# load our cache from file, if there is one
		if @cache.load(".#{@task_name}.cache")
			# if source files haven't changed, the object files won't have
			# either, so no need to recompile
			unless needs_recompile()
				# @TODO:: needs check for target exists
				@built = true
				return true
			end
		else
			# no cache on disk; create it
			puts.average ">>> Creating new cache"
			@cache.create_cache(@source_files, @object_files)
		end
		
		# Source files on disk have changed from what we have in cache;
		# start a recompile (we may not need to relink, but we can't
		# determine that yet).
		# We need to store a copy of all the object checksums to compare
		# them against the newly generated ones - if these then miss
		# against the cache, we need to relink.
		# clone does a shallow copy; we need marshal to do a deep copy
		temp_cache = Marshal.load(Marshal.dump(@cache))
		
		puts.average "\n>>> Compiling\n".fg_white
		
		# This is output within Project instead
		#unless @dependencies.empty?
		#	puts ">>> Task Dependencies: ".fg_white + "#{@dependencies.join(" ")}"
		#end
		
		# Our own build system - defined in case there's anything source
		# code specific that needs adjustment based on the builder
		add_compiler_flag("-D_CBUILD")
		
		# we don't use cache.recompile_list directly, as the object
		# files would not be in the same index order in the array
		@source_files.each_with_index do |src, i|
			
			if !@cache.recompile_list.include?(src) then
				next
			end
			
			@cmd_line = "#{@compiler} -c -o #{@object_files[i]}"
			
			# append variable lists that may be empty or nil (note preceeding spaces required!)
			unless @compiler_flags.empty?
				@cmd_line << " #{@compiler_flags.join(" ")}"
			end
			unless @forced_inclusions.empty?
				@cmd_line << " -include #{@forced_inclusions.join(" -include ")}"
			end
			unless @include_paths.empty?
				@cmd_line << " -I#{@include_paths.join(" -I")}"
			end
			
			if @build_type == BuildType::SHARED_LIBRARY
				@cmd_line << " -fPIC"
			end
			
			# finally, the input source file
			@cmd_line << " #{src}"
			
			puts.detailed ">>> Generated Command Line:".fg_white + "\n\t#{@cmd_line}".fg_yellow.bold
			puts.detailed "==> Executing...".fg_white
			output = `#{@cmd_line}`
			if $? != 0
				puts.little "#{$?}".fg_red.bold
				abort
			else
				# Looks bad in larger projects, yet better in smaller
				#puts "\tok"
			end
			
			# We can now update the checksums
			@cache.update_cache_source(src)
			@cache.update_cache_object(@object_files[i])
		end


		# Save the cache now, as we can return after this - doesn't 
		# need to be done after this point again
		puts.detailed "==> Saving cache"
		@cache.puts_cache()
		@cache.save(".#{@task_name}.cache")
		
		# we can reuse the name generated now in the linking phase,
		# save some time
		case @build_type
		when BuildType::SHARED_LIBRARY
			target = "lib#{@target_file}.so"
		when BuildType::STATIC_LIBRARY
			target = "lib#{@target_file}.a"
		else
			target = @target_file
		end
		target.prepend(@target_path)
		
		
		# The cache now contains the latest checksums of the files; 
		# compare these with the old, copied cache contents.
		# Nasty and hacky, but it works and is quick enough
		relink = false
		temp_cache.cache_files.each_with_index do |tmp, i|
			puts.debug "Old Cache: [#{tmp["Object"]} : #{tmp["ObjectChecksum"]}]"
			puts.debug "New Cache: [#{@cache.cache_files[i]["Object"]} : #{@cache.cache_files[i]["ObjectChecksum"]}]"
			# pretty much the same as @cache.is_object_uptodate, only we need to use the temp
			if tmp["ObjectChecksum"] != @cache.cache_files[i]["ObjectChecksum"]
				relink = true
			end
		end

		# If the compiled object files match what we have in cache (i.e.
		# only comments were updated, etc.) we don't need to relink, so
		# can skip it
		unless relink == true
			# if the target is deleted, we still need a relink
			if File.exists?(target)
				# so the project doesn't rescan this task again
puts.debug "target exists; not building.."
				@built = true
				return true
			end
		end
		
	
		if @build_type == BuildType::EXECUTABLE
			@cmd_line = "#{@compiler} -o #{target}"
		elsif @build_type == BuildType::SHARED_LIBRARY
			@cmd_line = "#{@compiler} -shared -o #{target}"	
		elsif @build_type == BuildType::STATIC_LIBRARY
			# no need for ranlib if 's' specified to ar
			@cmd_line = "ar rcs #{target}"
		else
			raise "Unknown BuildType!"
		end
		
		# object files MUST appear before libraries; spent an hour troubleshooting
		# my methods, command line compare, etc., before coming across that...
		unless @object_files.empty?
			@cmd_line << " #{@object_files.join(" ")}"
		end
		
		unless @build_type == BuildType::STATIC_LIBRARY
			# linker flags, options, etc.
			unless @linker_flags.empty?
				@cmd_line << " #{@linker_flags.join(" ")}"
			end
			unless @link_library_paths.empty?
				@cmd_line << " -L#{@link_library_paths.join(" -L")}"
			end
			unless @link_libraries.empty?
				@cmd_line << " -l#{@link_libraries.join(" -l")}"
			end
			
			puts.average ""
			puts.average ">>> Linking".fg_white
		else
			puts.average ""
			puts.average ">>> Creating archive".fg_white
		end
			
		
		puts.detailed ">>> Generated Command Line:".fg_white + "\n\t#{@cmd_line}".fg_yellow.bold
		puts.detailed "==> Executing...".fg_white
		`#{@cmd_line}`
		if $? != 0
			puts "#{$?}".fg_red.bold
			abort
		else
			@built = true
			puts.minimal "\t#{@task_name} compiled successfully\n".fg_green.bold
		end
	end
end




##############################################
# Helpers, not needed for real functionality #
##############################################

# modify the String class for colourization
class String
	def fg_black;		"\033[30m#{self}\033[0m" end
	def fg_red;		"\033[31m#{self}\033[0m" end
	def fg_green;		"\033[32m#{self}\033[0m" end
	def fg_yellow;		"\033[33m#{self}\033[0m" end
	def fg_blue;		"\033[34m#{self}\033[0m" end
	def fg_pink;		"\033[35m#{self}\033[0m" end
	def fg_cyan;		"\033[36m#{self}\033[0m" end
	def fg_white;		"\033[37m#{self}\033[0m" end
	def bg_black;		"\033[40m#{self}\033[0m" end
	def bg_red;		"\033[41m#{self}\033[0m" end
	def bg_green;		"\033[42m#{self}\033[0m" end
	def bg_yellow;		"\033[43m#{self}\033[0m" end
	def bg_blue;		"\033[44m#{self}\033[0m" end
	def bg_pink;		"\033[45m#{self}\033[0m" end
	def bg_cyan;		"\033[46m#{self}\033[0m" end
	def bg_white;		"\033[47m#{self}\033[0m" end
	def bold;		"\033[1m#{self}\033[22m" end
	def reverse_color;	"\033[7m#{self}\033[27m" end
	def reset;		"#{self}\033[m" end
end
# This is mostly just so we can see what objects are being created/destroyed at runtime
# Outside of that, this serves no purpose
class Class
	alias old_new new
	def new(*args)
		puts.debug "Creating a new '#{self.name}'".fg_cyan
		old_new(*args)
	end
end
