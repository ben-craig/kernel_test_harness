TEST_ROOT = "\\src\\llvm\\runtimes\\libcxx\\test"

LONG_TERM_TODO_DIRS = [
    # beware of allocations
    "std\\language.support\\support.dynamic",
    "std\\language.support\\support.rtti",
    "std\\language.support\\support.exception",
    "std\\language.support\\cmp",   #doesn't exist yet
    "std\\language.support\\support.start.term", # testing exit in the kernel, fun
]

TEST_DIRS = [
    "std\\language.support\\support.types",
    # beware of floats in limits
    "std\\language.support\\support.limits",
    "std\\language.support\\cstdint",
    "std\\language.support\\support.initlist",
    "std\\language.support\\support.runtime\\cstdarg.pass.cpp",
    "std\\language.support\\support.runtime\\cstdbool.pass.cpp",
    "std\\diagnostics\\errno",
    "std\\diagnostics\\syserr\\errc.pass.cpp",
    "std\\utilities\\allocator.tag",
    "std\\utilities\\allocator.traits",
    "std\\utilities\\allocator.uses",
    "std\\utilities\\pointer.conversion",
    "std\\utilities\\pointer.traits",
    "std\\utilities\\ptr.align",
    "std\\utilities\\specialized.algorithms",
    "std\\utilities\\function.objects\\arithmetic.operations",
    "std\\utilities\\function.objects\\bitwise.operations",
    "std\\utilities\\function.objects\\comparisons",
    "std\\utilities\\function.objects\\func.def",
    "std\\utilities\\function.objects\\func.invoke",
    "std\\utilities\\function.objects\\func.memfn",
    "std\\utilities\\function.objects\\func.require",
    "std\\utilities\\function.objects\\func.search\\func.search.default",
    "std\\utilities\\function.objects\\logical.operations",
    "std\\utilities\\function.objects\\negators",
    "std\\utilities\\function.objects\\unord.hash",
    "std\\utilities\\ratio",
    "std\\utilities\\time\\time.clock.req",
    "std\\utilities\\time\\time.point",
    "std\\utilities\\time\\time.traits",
    "std\\utilities\\time\\hours.pass.cpp",
    "std\\utilities\\time\\microseconds.pass.cpp",
    "std\\utilities\\time\\milliseconds.pass.cpp",
    "std\\utilities\\time\\minutes.pass.cpp",
    "std\\utilities\\time\\nanoseconds.pass.cpp",
    "std\\utilities\\time\\seconds.pass.cpp",
    "std\\strings\\char.traits",
    "std\\strings\\c.strings\\cstring.pass.cpp", #partial
    "std\\strings\\c.strings\\cwchar.pass.cpp", #partial
    "std\\iterators\\iterator.container",
    "std\\iterators\\iterator.primitives",
    "std\\iterators\\iterator.range",
    "std\\iterators\\iterator.requirements",
    "std\\iterators\\iterator.synopsis",
    "std\\iterators\\iterators.general",
    "std\\iterators\\predef.iterators\\move.iterators",
    "std\\iterators\\predef.iterators\\reverse.iterators",
    "std\\algorithms\\alg.c.library",
    "std\\algorithms\\alg.modifying.operations",
    "std\\algorithms\\algorithms.general",
#    "std\\utilities\\utility",
#    "std\\utilities\\tuple",
#    "std\\utilities\\meta",
    "std\\algorithms\\alg.nonmodifying\\alg.copy",
    "std\\algorithms\\alg.nonmodifying\\alg.fill",
    "std\\algorithms\\alg.nonmodifying\\alg.generate",
    "std\\algorithms\\alg.nonmodifying\\alg.move",
    "std\\algorithms\\alg.nonmodifying\\alg.random.sample",
    "std\\algorithms\\alg.nonmodifying\\alg.random.shuffle",
    "std\\algorithms\\alg.nonmodifying\\alg.remove",
    "std\\algorithms\\alg.nonmodifying\\alg.replace",
    "std\\algorithms\\alg.nonmodifying\\alg.reverse",
    "std\\algorithms\\alg.nonmodifying\\alg.rotate",
    "std\\algorithms\\alg.nonmodifying\\alg.swap",
    "std\\algorithms\\alg.nonmodifying\\alg.transform",
    "std\\algorithms\\alg.nonmodifying\\alg.unique",
    "std\\algorithms\\alg.sorting\\alg.binary.search",
    "std\\algorithms\\alg.sorting\\alg.clamp",
    "std\\algorithms\\alg.sorting\\alg.heap.operations",
    "std\\algorithms\\alg.sorting\\alg.lex.comparison",
    "std\\algorithms\\alg.sorting\\alg.min.max",
    "std\\algorithms\\alg.sorting\\alg.nth.element",
    "std\\algorithms\\alg.sorting\\alg.permutation.generators",
    "std\\algorithms\\alg.sorting\\alg.set.operations",
]
TODO_TEST_DIRS = [
    "std\\utilities\\function.objects\\bind",
    "std\\utilities\\function.objects\\refwrap",
    "std\\utilities\\function.objects\\func.not_fn",
    "std\\utilities\\time\\time.duration",
    #    "std\\utilities\\charconv", #partial, does not exist yet
    #"std\\algorithms\\alg.nonmodifying\\alg.partitions\\is_partitioned.pass.cpp",
    #"std\\algorithms\\alg.nonmodifying\\alg.partitions\\partition.pass.cpp",
    #"std\\algorithms\\alg.nonmodifying\\alg.partitions\\partition_copy.pass.cpp",
    #"std\\algorithms\\alg.nonmodifying\\alg.partitions\\partition_point.pass.cpp",
    "std\\algorithms\\alg.sorting\\alg.sort\\is.sorted",
    "std\\algorithms\\alg.sorting\\alg.sort\\partial.sort.copy",
    "std\\algorithms\\alg.sorting\\alg.sort\\partial.sort",
    "std\\algorithms\\alg.sorting\\alg.sort\\sort",
    "std\\algorithms\\alg.sorting\\alg.merge\\merge.pass.cpp",
    "std\\algorithms\\alg.sorting\\alg.merge\\merge_comp.pass.cpp",
    "std\\numerics\\rand\\rand.adapt",
    "std\\numerics\\rand\\rand.eng",
    "std\\numerics\\rand\\rand.predef",
    "std\\numerics\\rand\\rand.req",
    "std\\numerics\\rand\\rand.dis\\rand.dist.uni\\rand.dist.uni.int",
    "std\\numerics\\numeric.ops",
    "std\\numerics\\c.math\\cmath.pass.cpp", #partial
    "std\\atomics",
]

$test_files = [
    "sys\\dummy.cpp",
]

DEST_ROOT = "x64\\Release"
INFRA_OBJ = "#{DEST_ROOT}\\doAssert.obj #{DEST_ROOT}\\sioctl.obj"
APP = "#{DEST_ROOT}\\ioctlapp.exe"

def walk(path)
    if not File.directory?(path)
        if path.end_with?(".pass.cpp")
            $test_files << path.gsub("/", "\\")
        end
        return
    end
    Dir.foreach(path) do |x|
        if x == "." or x == ".."
            next
        end
        walk(File.join(path, x))
    end
end

def strip_fname(fname)
    if fname[0] == "\\"
        return fname[idx+1, fname.length]
    end
    return fname
end

def scrub_fname(fname)
    tmpName = fname.gsub("+", "__plus__")
    tmpName.gsub!("-", "__minus__")
    tmpName.gsub!("=", "__eq__")
    return tmpName
end

def srcToDestRoot(fname)
    return File.join(DEST_ROOT, strip_fname(fname)).gsub("/", "\\")
end

def srcToObj(fname)
    return srcToDestRoot(fname) + ".obj"
end

def srcToAsm(fname)
    return srcToDestRoot(scrub_fname(fname)) + ".asm"
end

def srcToSys(fname)
    return srcToDestRoot(scrub_fname(fname)) + ".sys"
end

def srcToDevSrc(fname)
    return srcToDestRoot(fname) + ".name.cpp"
end

def srcToDevObj(fname)
    return srcToDestRoot(fname) + ".name.obj"
end

def srcToCheckFile(fname)
    return srcToDestRoot(scrub_fname(fname)) + ".check.log"
end

def srcToAsmCheckFile(fname)
    return srcToDestRoot(scrub_fname(fname)) + ".asm.check.log"
end

def srcToDevName(fname)
    return strip_fname(fname).gsub("\\", ".")
end

def srcToRuleName(fname)
    return strip_fname(fname).gsub("\\", ".")
end

def makedirs(*dirs)
    for dir in dirs
        parent = File.dirname(dir)
        next if parent == dir or File.directory? dir
        makedirs parent unless File.directory? parent
        if File.basename(dir) != ""
            begin
                Dir.mkdir dir
            rescue SystemCallError
                raise unless File.directory? dir
            end
        end
    end
end


NT_DEVICE_NAME = %q{const wchar_t *NT_DEVICE_NAME = L"\\\\Device\\\\}
DOS_DEVICE_NAME = %q{const wchar_t *DOS_DEVICE_NAME = L"\\\\DosDevices\\\\}

def genDevSrc(devSrc, devName)
    makedirs(File.dirname(devSrc.gsub("\\", "/")))
    target_contents =
        NT_DEVICE_NAME + devName + "\";\n" +
        DOS_DEVICE_NAME + devName + "\";\n";
    if File.file?(devSrc)
        if IO.read(devSrc) == target_contents
            return
        end
    end
    File.open(devSrc, "w") do |h|
        h.print target_contents
    end
end

def main()
    TEST_DIRS.each do |f|
        walk(File.join(TEST_ROOT, f))
    end
    File.open("generated_targets_from_gen.rb.ninja", "w") do |h|
        h.print "ninja_required_version = 1.7\n\n"

        $test_files.each do |fname|
            obj = srcToObj(fname)
            asm = srcToAsm(fname)
            sys = srcToSys(fname)
            devSrc = srcToDevSrc(fname)
            devObj = srcToDevObj(fname)
            devName = srcToDevName(fname)

            genDevSrc(devSrc, devName)

            h.print "# #{fname}\n"
            h.print "build #{obj}: kcompile #{fname}\n"
            #h.print "build #{obj}.pdb: dummy_dep #{obj}\n"

            h.print "build #{devObj}: kcompile #{devSrc}\n"
            #h.print "build #{devObj}.pdb: dummy_dep #{devObj}\n"

            h.print "build #{sys} | #{sys}.pdb: klink $\n"
            #h.print "build #{sys}: klink $\n"
            h.print "    #{obj} #{devObj} #{INFRA_OBJ}\n"

            h.print "build #{sys}.signed: signtool #{sys}\n"

            h.print "build #{asm}: asm_dump #{sys} | #{sys}.signed\n"
            h.print "build #{srcToAsmCheckFile(fname)}: check_asm_for_float #{asm} | x64\\Release\\asm_checker.exe\n"

            h.print "build #{srcToCheckFile(fname)}: check #{sys} | #{sys}.signed #{APP}\n"
            h.print "    devPath = #{devName}\n"
            h.print "\n"
        end

        h.print "build check: phony |"
        $test_files.each do |fname|
            h.print " #{srcToCheckFile(fname)}"
        end
        h.print  "\n"

        h.print "build check_asm: phony |"
        $test_files.each do |fname|
            h.print " #{srcToAsmCheckFile(fname)}"
        end
        h.print "\n"

        h.print "default"
        $test_files.each do |fname|
            sys = srcToSys(fname)
            h.print " #{sys}.signed"
        end
        h.print "\n"

    end
end

if __FILE__ == $0
   main()
end
