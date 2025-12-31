target("fast-networking")
    set_kind("binary")
    add_files("src/**.c")
    add_includedirs("include")
    add_defines("_BUILDING_FN")
    if is_plat("windows") then
        add_links("ws2_32")
    end
target_end()
