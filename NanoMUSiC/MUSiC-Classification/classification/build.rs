use std::env;
// use std::path::Path;
// use std::process::Command;
fn main() {
    // cxx_build::bridge("src/main.rs")
    //     .file("src/ttree_reader.cpp")
    //     .flag_if_supported("-std=c++17")
    //     .compile("ttree_reader");

    println!("cargo:rerun-if-changed=src/ttree_reader.cpp");
    println!("cargo:rerun-if-changed=build.rs");

    // let out_dir = env::var("OUT_DIR").unwrap();
    let cc_env = env::var("CC").unwrap();

    // // compile
    // Command::new("g++")
    //     .args(&["-pthread", "-std=c++17", "-m64", "-I/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/include", "-L/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib", "-lCore", "-lImt", "-lRIO", "-lNet", "-lHist", "-lGraf", "-lGraf3d", "-lGpad", "-lROOTVecOps", "-lTree", "-lTreePlayer", "-lRint", "-lPostscript", "-lMatrix", "-lPhysics", "-lMathCore", "-lThread", "-lMultiProc", "-lROOTDataFrame", "-Wl,-rpath,/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib", "-lm", "-ldl", "-rdynamic", "src/ttree_reader.cpp", "-c", "-fPIC", "-o"])
    //     .arg(&format!("{}/ttree_reader.o", out_dir))
    //     .status()
    //     .unwrap();

    // compile
    // Command::new("g++")
    //     .args(&[
    //         "-std=c++17",
    //         "-m64",
    //         "src/ttree_reader.cpp",
    //         "-c",
    //         "-fPIC",
    //         "-Wall",
    //         "-Wextra",
    //         "-Werror",
    //         "-o",
    //         "-lstdc++",
    //     ])
    //     .arg(&format!("{}/ttree_reader.o", out_dir))
    //     .status()
    //     .unwrap();

    // // make static
    // Command::new("ar")
    //     .args(&["crus", "libttreereader.a", "ttree_reader.o"])
    //     .current_dir(&Path::new(&out_dir))
    //     .status()
    //     .unwrap();

    // println!("cargo:rustc-link-search=native={}", out_dir);
    // println!("cargo:rustc-link-search=native=/usr/lib64");
    // println!("cargo:rustc-link-lib=stdc++");
    // println!("cargo:rustc-link-search=native=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib");
    // println!("cargo:rustc-link-lib=dylib=Core");
    // println!("cargo:rustc-link-lib=dylib=Imt");
    // println!("cargo:rustc-link-lib=dylib=RIO");
    // println!("cargo:rustc-link-lib=dylib=Net");
    // println!("cargo:rustc-link-lib=dylib=Hist");
    // println!("cargo:rustc-link-lib=dylib=Graf");
    // println!("cargo:rustc-link-lib=dylib=Graf3d");
    // println!("cargo:rustc-link-lib=dylib=Gpad");
    // println!("cargo:rustc-link-lib=dylib=ROOTVecOps");
    // println!("cargo:rustc-link-lib=dylib=Tree");
    // println!("cargo:rustc-link-lib=dylib=TreePlayer");
    // println!("cargo:rustc-link-lib=dylib=Rint");
    // println!("cargo:rustc-link-lib=dylib=Postscript");
    // println!("cargo:rustc-link-lib=dylib=Matrix");
    // println!("cargo:rustc-link-lib=dylib=Physics");
    // println!("cargo:rustc-link-lib=dylib=MathCore");
    // println!("cargo:rustc-link-lib=dylib=Thread");
    // println!("cargo:rustc-link-lib=dylib=MultiProc");
    // println!("cargo:rustc-link-lib=dylib=ROOTDataFrame");
    // println!("cargo:rustc-link-lib=static=ttreereader");
    // println!("cargo:rerun-if-changed=src/ttree_reader.cpp");
    // println!("cargo:rerun-if-changed=build.rs");

    cc::Build::new()
        .cpp(true)
        .file("src/ttree_reader.cpp")
        .extra_warnings(true)
        .cpp_link_stdlib("stdc++")
        .warnings(true)
        .flag("-std=c++17")
        .flag("-pthread")
        .flag("-m64")
        .include(
            "/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/include",
        )
        .compile("ttreereader");

    println!("cargo:rustc-link-search=native=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib");
    println!("cargo:rustc-link-lib=Core");
    println!("cargo:rustc-link-lib=Imt");
    println!("cargo:rustc-link-lib=RIO");
    println!("cargo:rustc-link-lib=Net");
    println!("cargo:rustc-link-lib=Hist");
    println!("cargo:rustc-link-lib=Graf");
    println!("cargo:rustc-link-lib=Graf3d");
    println!("cargo:rustc-link-lib=Gpad");
    println!("cargo:rustc-link-lib=ROOTVecOps");
    println!("cargo:rustc-link-lib=Tree");
    println!("cargo:rustc-link-lib=TreePlayer");
    println!("cargo:rustc-link-lib=Rint");
    println!("cargo:rustc-link-lib=Postscript");
    println!("cargo:rustc-link-lib=Matrix");
    println!("cargo:rustc-link-lib=Physics");
    println!("cargo:rustc-link-lib=MathCore");
    println!("cargo:rustc-link-lib=Thread");
    println!("cargo:rustc-link-lib=MultiProc");
    println!("cargo:rustc-link-lib=ROOTDataFrame");
    println!("cargo:rustc-link-arg-bins=-lm");
    println!("cargo:rustc-link-arg-bins=-ldl");
    println!("cargo:rustc-link-arg-bins=-rdynamic");
    println!("cargo:rustc-link-arg-bins=-pthread");
    println!("cargo:rustc-link-arg-bins=-Wl,-rpath,/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/lib");
    println!("cargo:rustc-linker={}", cc_env);
}
