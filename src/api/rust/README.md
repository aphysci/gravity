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

## AUTOCXX
[Autocxx](https://google.github.io/autocxx/index.html) is a more powerful tool to bind Rust to C++. However, it does have some limitations to it. From what I have experimented with so far, it sometimes has trouble with user defined data types. In particular, it has not been able to see GravityReturnCode and GravityTransportType, which means it has not been able to generate the bindings for any GravityNode method that uses this **A LOT OF THEM DO...**