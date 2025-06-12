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

It is also possible to use subclasses with autocxx, but generating these bindings is experimental and often interferes with other methods we are trying to bind. It may be possible for the bindings to be generated separatly for the non-inheritance based classes, and then a binder for each inheritance based one. Stay Tuned

Another thought is use autocxx for strictly the subclass stuff, and then manually  bind the rest, in case the above does not work. 

## Current Updates

I have included a wrapper on each end (C++ and Rust). This makes the code feel like true Rust code, while not having to impact any of the source code.

The provided stuff here will in fact make a gravity node publish a basic string data product successfully. Rust stuff cannot yet subscribe. Stay tuned. The next steps that I will take are either do the protobufs or work with the subscriber. Neither are easy since there is not support for a vector of shared pointers yet, nor protobuf, so some janky work-arounds are in order for sure! Exciting :/

June 12, 2025