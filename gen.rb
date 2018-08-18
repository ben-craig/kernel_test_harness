TEST_FILES = [
    "\\src\\llvm\\runtimes\\libcxx\\test\\std\\nothing_to_do.pass.cpp",
    "sys\\dummy.cpp",
]

DEST_ROOT = "x64\\Release"
INFRA_OBJ = "#{DEST_ROOT}\\doAssert.obj #{DEST_ROOT}\\sioctl.obj"
APP = "#{DEST_ROOT}\\ioctlapp.exe"

def strip_fname(fname)
    if fname[0] == "\\"
        return fname[idx+1, fname.length]
    end
    return fname
end

def srcToDestRoot(fname)
    return DEST_ROOT + "\\" + strip_fname(fname)
end

def srcToObj(fname)
    return srcToDestRoot(fname) + ".obj"
end

def srcToAsm(fname)
    return srcToDestRoot(fname) + ".asm"
end

def srcToSys(fname)
    return srcToDestRoot(fname) + ".sys"
end

def srcToDevSrc(fname)
    return srcToDestRoot(fname) + ".name.cpp"
end

def srcToDevObj(fname)
    return srcToDestRoot(fname) + ".name.obj"
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
    File.open(devSrc, "w") do |h|
        h.print NT_DEVICE_NAME + devName + "\";\n"
        h.print DOS_DEVICE_NAME + devName + "\";\n"
    end
end

def main()
    File.open("generated_targets_from_gen.rb.ninja", "w") do |h|
        h.print "ninja_required_version = 1.7\n\n"

        h.print "build"
        TEST_FILES.each do |fname|
            h.print " #{srcToDevSrc(fname)}"
        end
        h.print ": phony generated_targets_from_gen.rb.ninja\n\n"

        TEST_FILES.each do |fname|
            obj = srcToObj(fname)
            asm = srcToAsm(fname)
            sys = srcToSys(fname)
            devSrc = srcToDevSrc(fname)
            devObj = srcToDevObj(fname)
            devName = srcToDevName(fname)

            genDevSrc(devSrc, devName)

            h.print "# #{fname}\n"
            h.print "build #{obj}: kcompile #{fname}\n"
            h.print "build #{obj}.pdb: dummy_dep #{obj}\n"

            h.print "build #{devObj}: kcompile #{devSrc}\n"
            h.print "build #{devObj}.pdb: dummy_dep #{devObj}\n"

            h.print "build #{sys} | #{sys}.pdb: klink $\n"
            h.print "    #{obj} #{devObj} #{INFRA_OBJ}\n"

            h.print "build #{sys}.signed: signtool #{sys}\n"

            h.print "build #{asm}: asm_dump #{sys} || #{sys}.signed\n"
            check_asm_rule = "check_" + asm.gsub("\\", ".")
            h.print "build #{check_asm_rule}: check_asm_for_float #{asm}\n"

            h.print "build check_#{srcToRuleName(fname)}: check #{sys} || #{sys}.signed #{APP}\n"
            h.print "    devPath = #{devName}\n"
            h.print "\n"
        end

        h.print "build check: phony ||"
        TEST_FILES.each do |fname|
            h.print " check_#{srcToRuleName(fname)}"
        end
        h.print  "\n"

        h.print "build check_asm: phony ||"
        TEST_FILES.each do |fname|
            h.print " check_#{srcToAsm(fname).gsub("\\", ".")}"
        end
        h.print "\n"

        h.print "default"
        TEST_FILES.each do |fname|
            sys = srcToSys(fname)
            h.print " #{sys}.signed"
        end
        h.print "\n"

    end
end

if __FILE__ == $0
   main()
end
