set_project("software-rendering")
set_xmakever("2.6.1")

add_requires("eigen", "spdlog", "stb", "openmp")
add_rules("mode.release", "mode.debug")
set_languages("cxx17")

target("SoftRender")
    set_kind("binary")
    add_includedirs("src")
    add_files("src/main.cpp", "src/common/*.cpp", "src/graphics/*.cpp", "src/renderer/*.cpp")
    if is_plat("windows", "mingw") then
        add_files("src/platforms/win32.cpp")
        add_syslinks("Gdi32", "User32")
    elseif is_plat("macosx") then
        add_frameworks("Cocoa")
        add_files("src/platforms/macos.mm")
        set_values("objc++.build.arc", false)
    end
    add_packages("eigen", "spdlog", "stb", "openmp", {public=true})
    set_targetdir("bin")
