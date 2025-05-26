add_rules("mode.debug", "mode.release")

target("clibs")
    set_kind("binary")
    add_files("./*.c")
 