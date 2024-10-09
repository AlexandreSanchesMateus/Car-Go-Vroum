add_rules("mode.asan", "mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

add_requires("fmt")
add_requires("enet6", { configs = { debug = is_mode("debug") }})

set_project("CarGoServer")

set_allowedmodes("debug", "release", "distrib")
set_allowedplats("windows", "mingw", "linux", "macosx")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "macosx|x86_64")
set_defaultmode("debug")

set_languages("cxx20")
set_exceptions("cxx")
set_encodings("utf-8")
set_rundir(".")
set_warnings("allextra")
add_includedirs("include")

add_cxflags("/wd4251")

if is_mode("distrib") then
    set_optimize("fastest")
    set_symbols("none")
else
    if is_mode("debug") then
        set_suffixname("-debug")
    end
end

target("CarGoServer")
--    if is_mode("distrib") then
--        set_kind("static")
--        add_defines("SERVER_STATIC", { public = true })
--    else
--        set_kind("shared")
--    end

    set_kind("binary")

    add_headerfiles("include/CarGoServer/**.hpp")
    add_files("src/**.cpp")
    add_packages("enet6", "fmt")
    add_defines("SERVER_BUILD")

