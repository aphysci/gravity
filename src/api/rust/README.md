# Rust for Gravity

A Rust API for gravity

## CXX

Since SWIG does not have support for rust, the current implementation uses a crate called [CXX](https://cxx.rs). CXX allows for functions to call and be called from C++ to Rust. In order to make this work, the cargo.toml must include these lines:

```Rust
[dependencies]
cxx = "1.0"

[build-dependencies]
cxx-build = "1.0"
```