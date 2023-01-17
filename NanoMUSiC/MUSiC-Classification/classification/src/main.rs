// use libc;
use std::ffi::CString;
use std::os::raw::c_char;

use root_binds::rvec_float;
use std::slice;

mod root_binds {

    #[repr(C)]
    pub struct TTreeReader {
        _private: [u8; 0],
    }

    #[repr(C)]
    pub struct TTreeReaderValueFloat {
        _private: [u8; 0],
    }

    #[repr(C)]
    pub struct ValueReaders {
        _private: [u8; 0],
        pub px: *mut TTreeReaderValueFloat,
        pub py: *mut TTreeReaderValueFloat,
    }

    #[repr(C)]
    pub struct rvec_float {
        pub data: *const libc::c_float,
        pub len: libc::size_t,
    }

    #[repr(C)]
    pub struct vecrvec_float {
        pub data: *const libc::c_void,
        pub len: libc::size_t,
    }

    // #[repr(C)]
    // pub struct vecvec_float {
    //     _private: [u8; 0],
    //     pub data: *mut rvec_float,
    //     pub len: libc::size_t,
    // }
}

extern "C" {

    fn get_float_arrays(
        tree_name: *const c_char,
        file_path: *const c_char,
    ) -> *mut root_binds::vecrvec_float;
    // ) -> root_binds::rvec_float;

    fn get_tree_reader(
        tree_name: *const c_char,
        file_path: *const c_char,
    ) -> *mut root_binds::TTreeReader;

    fn get_value_readers(
        tree_reader: *mut root_binds::TTreeReader,
    ) -> *mut root_binds::ValueReaders;

    fn next(tree_reader: *mut root_binds::TTreeReader) -> bool;

    fn get_value(value_reader: *mut root_binds::TTreeReaderValueFloat) -> f32;

    fn process_tree(tree_reader: *mut root_binds::TTreeReader);
}

fn main() {
    let tree_name = CString::new("ntuple").unwrap();
    let file_path = CString::new("$ROOTSYS/tutorials/hsimple.root").unwrap();

    unsafe {
        let tree_reader = get_tree_reader(tree_name.as_ptr(), file_path.as_ptr());
        let value_readers = get_value_readers(tree_reader);

        let mut event_count: i32 = 0;
        let mut sum: f32 = 0.;
        while next(tree_reader) {
            event_count += 1;
            sum += get_value((*value_readers).px) + get_value((*value_readers).py);
        }
        println!("Sum: {}", sum);
        println!("Event count: {}", event_count);

        process_tree(get_tree_reader(tree_name.as_ptr(), file_path.as_ptr()));
    }

    // RDataFrame ...
    let tree_name = CString::new("Events").unwrap();
    let file_path = CString::new("/home/home1/institut_3a/silva/projects/NanoMUSiC_Test_Data/DY_2017/04426F83-BF53-0440-B2D1-F2DD9AB96EB8.root").unwrap();

    let float_arrays_of_array =
        unsafe { &*get_float_arrays(tree_name.as_ptr(), file_path.as_ptr()) };

    unsafe {
        println!("Float Arrays: {:?}", float_arrays_of_array.len);
    }

    for idx in 0..float_arrays_of_array.len {
        //     // println!("Event: {}", evt);
        //     let float_arrays_slice =
        let evt = root_binds::rvec_float {
            data: (float_arrays_of_array + idx).data,
            len: (float_arrays_of_array + idx).len,
        };
        let float_arrays = unsafe { slice::from_raw_parts(evt.data as f32, evt.len) };
        println!("Float Arrays: {:?}", float_arrays);
    }
}
