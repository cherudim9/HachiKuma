#!/usr/local/bin/python3

import argparse
import os
import os.path
import sys
import time
import subprocess
import python_config

def my_execute(*args):
    print(' '.join(args))
    subprocess.check_call(args)

def main(args):    
    arg_parser=argparse.ArgumentParser()

    arg_parser.add_argument("source", nargs=1, help="one source to be compiled")
    arg_parser.add_argument("-a", "--additional_sources", action="append", default=[], help="additonal sources to be compiled")

    arg_parser.add_argument("-b", "--build", help="OUT build path(default to be .)")
    arg_parser.add_argument("-i", "--interface", help="IN interface path(default to be <source_base>.i)")
    #read build_path+interface_path to get interface files
    arg_parser.add_argument("-e", "--extension", help="OUT extension path(default to be <source_base>.so)")
    #write extension to build_path+extension_path
    arg_parser.add_argument("-w", "--wrapper", help="OUT wrapper path(default to be _<source_base>_wrapper.cc)")
    #write wrapper to build_path+wrapper_path
    arg_parser.add_argument("-m", "--module", help="OUT module path(default to be <source_base>.py)")
    #write modules to build_path+module_path

    arg_parser.add_argument("-I", "--python_include_path", help="path of python head files")
    arg_parser.add_argument("-L", "--python_lib_path", help="path of python library files")
    arg_parser.add_argument("-l", "--python_lib", help="python library files")

    arg_parser.add_argument("-t", "--have-tag", action="store_true", help="have tag in output files")
    arg_parser.add_argument("-T", "--tag", help="descriptions of the tags")
    #with '-t' but no tag name specification, time stamps will be added instead

    args=arg_parser.parse_args(args[1:])

    source_full=args.source[0]
    dir_path, source_name=os.path.split(source_full)
    if not dir_path:
        dir_path='.'
    source_base, source_ext=os.path.splitext(source_name)
    # /usr/local/bin(dir_path), build(source_base), py(source_ext)

    build_path=args.build if args.build else '.'
    if build_path != '.':
        os.makedirs(build_path, exist_ok=True)

    interface_path=args.interface if args.interface else os.path.join(dir_path, source_base+".i")
    extension_path=args.extension if args.extension else os.path.join(build_path, '_'+source_base+".so")
    wrapper_path=args.wrapper if args.wrapper else os.path.join(build_path,  source_base+"_wrapper.cc")
    module_name=args.module if args.module else source_base
    if args.have_tag or args.tag:
        tag=args.tag if args.tag else time.strftime("%Y%m%d%H%M%S")
        module_name+="_"+tag
    
    my_execute("swig", "-python", "-c++", "-py3", "-builtin", "-module", module_name, "-outdir", build_path, "-o", wrapper_path, interface_path)
    
    compile_args=["c++", "-O3", "-Wall", "-std=c++11", "-shared", "-fPIC", "-I"+dir_path]
    compile_args.append(args.source[0])
    compile_args.append(wrapper_path)
    compile_args.extend(args.additional_sources)
    compile_args.extend(['-o', extension_path])

    python_include_path=args.python_include_path if args.python_include_path else python_config.PYTHON_INCLUDE_PATH
    if python_include_path:
        compile_args.append('-I'+python_include_path)
    python_lib_path=args.python_lib_path if args.python_lib_path else python_config.PYTHON_LIB_PATH
    if python_lib_path:
        compile_args.append('-L'+python_lib_path)
    python_lib=args.python_lib if args.python_lib else python_config.PYTHON_LIB
    if python_lib:

        compile_args.append('-l'+python_lib)

    my_execute(*compile_args)

if __name__=="__main__":
    main(sys.argv)
