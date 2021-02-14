"""Compile and run a .prez file"""

def compile_and_run(file, src, **kwargs):
    native.genrule(
        name = file + "_exe",
        srcs = [
            src,
            "//src/language/built_in:compile_string",
            "//src/language/built_in:syscall.s",
            "//src/language/built_in:string.prez",
        ],
        outs = [
            file,
        ],
        cmd_bash = "$(execpath //src/language:main) $(rootpath %s) $(RULEDIR)/%s.s \
                  && as $(RULEDIR)/%s.s $(execpath //src/language/built_in:syscall.s) $(execpath //src/language/built_in:compile_string) -o $(RULEDIR)/%s.o \
                  && ld $(RULEDIR)/%s.o -o $(RULEDIR)/%s" % (src, file, file, file, file, file),
        executable = True,
        message = "Compiling and running %s" % src,
        tools = ["//src/language:main"],
        **kwargs
    )
