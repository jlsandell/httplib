
-- Project ----------------------------------------------------------------------------------

project.name = "httplib"
project.bindir = "bin"

-- Package ----------------------------------------------------------------------------------

package = newpackage()

package.name = "httplib"
package.kind = "dll"
package.language = "c++"
package.configs = { "Debug", "Release" }

if (windows) then
   table.insert(package.defines, "WIN32") -- Needed to fix something on Windows.
end

-- Include and library search paths, system dependent (I don't assume a directory structure)

package.includepaths = {
"./include",
"/usr/local/include/boost"
}

package.libpaths = {
"/usr/local/lib",
}

-- Libraries to link to ---------------------------------------------------------------------

package.links = {
"pthread",
"boost_system",
"boost_regex"
}

-- pkg-configable stuff ---------------------------------------------------------------------

if (linux) then
package.buildoptions = {
}

package.linkoptions = {
}
end

-- Files ------------------------------------------------------------------------------------

package.files = {
matchrecursive("include/*.h", "./src/*.cpp"),
}

-- Debug configuration ----------------------------------------------------------------------

debug = package.config["Debug"]
debug.defines = { "DEBUG", "_DEBUG" }
debug.objdir = "obj/debug"
debug.target = "debug/" .. package.name .. "_d"

debug.buildoptions = { "-g" }

-- Release configuration --------------------------------------------------------------------

release = package.config["Release"]
release.objdir = "obj/release"
release.target = "release/" .. package.name
