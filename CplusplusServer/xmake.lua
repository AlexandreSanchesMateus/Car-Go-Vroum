add_rules("mode.asan", "mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

add_requires("fmt")
add_requires("enet6", { configs = { debug = is_mode("debug") }})
add_requires("vcpkg::physx", { alias = "physx", configs = { shared = true, debug = is_mode("debug") }})
add_requires("nlohmann_json") 

set_project("GearUpServer")

set_allowedmodes("debug", "release", "distrib")
set_allowedplats("windows", "mingw", "linux", "macosx")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "macosx|x86_64")
set_defaultmode("debug")
set_runtimes(is_mode("debug") and "MDd" or "MD")

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
elseif is_mode("debug") then
    set_suffixname("-debug")
end

if is_mode("debug") then
    add_defines("_DEBUG")
end

target("GearUpServer")
--    if is_mode("distrib") then
--        set_kind("static")
--        add_defines("SERVER_STATIC", { public = true })
--    else
--        set_kind("shared")
--    end

    set_kind("binary")

    add_headerfiles("include/CarGoServer/**.hpp")
    add_headerfiles("include/**.natvis")
    add_files("src/**.cpp")
    add_packages("enet6", "fmt", "physx", "nlohmann_json")
    add_defines("SERVER_BUILD")

    on_load(function (target)
        local physx = target:pkg("physx")
        for _, dir in ipairs(physx:get("sysincludedirs")) do
            local sub = path.join(dir, "physx")
            if os.isdir(sub) then
                target:add("includedirs", sub)
            end
        end
    end)

    -- if is_mode("debug") then
    --     add_defines("DEBUG")
    --     set_optimize("none")
    -- else
    --     set_optimize("fastest")
    -- end