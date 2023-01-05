use std::env;

fn main() {
    println!("cargo:rerun-if-changed=src/ttree_reader.cpp");
    println!("cargo:rerun-if-changed=build.rs");

    let cc_env = env::var("CC").unwrap();
    cc::Build::new()
        .cpp(true)
        .file("src/tree_reader.cpp")
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
