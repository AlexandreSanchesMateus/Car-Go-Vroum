add_rules("mode.asan", "mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

add_requires("fmt", "sfml")
add_requires("enet6", { configs = { debug = is_mode("debug") }})
add_requires("vcpkg::physx", { alias = "physx", configs = { shared = true }})
add_requires("nlohmann_json") 

set_project("CarGoServer")

set_allowedmodes("debug", "release", "distrib")
set_allowedplats("windows", "mingw", "linux", "macosx")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "macosx|x86_64")
set_defaultmode("debug")
set_runtimes("MD")

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
    add_packages("enet6", "fmt", "physx", "nlohmann_json", "sfml")
    add_defines("SERVER_BUILD")
    
    on_config(function (target)
        local physx = target:pkg("physx")
        local baseincludedir = table.wrap(physx:get("sysincludedirs"))[1]
        for _, dir in ipairs(os.dirs(path.join(baseincludedir, "*"))) do
            target:add("sysincludedirs", dir)
        end
    end)

    -- if is_mode("debug") then
    --     add_defines("DEBUG")
    --     set_optimize("none")
    -- else
    --     set_optimize("fastest")
    -- end
