use libc::c_void;

fn bar(oi: i32) -> i32 {
    oi
}

extern "C" {
    fn get_file() -> c_void;
    fn ttree_reader(file: c_void);
}

fn main() {
    let foo = 32;
    println!("Hello, world - {} !", bar(foo));
    // ffi::ttree_reader();

    unsafe {
        ttree_reader(get_file());
    }
}
