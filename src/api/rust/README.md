# Rust for Gravity

A Rust API for gravity

## CXX

Since SWIG does not have support for rust, the current implementation uses a crate called [CXX](https://cxx.rs). CXX allows for functions to call and be called from C++ to Rust. 

## AUTOCXX
[Autocxx](https://google.github.io/autocxx/index.html) is a more powerful tool to bind Rust to C++. However, it does have some limitations to it. From what I have experimented with so far, it sometimes has trouble with user defined data types. In particular, it has not been able to see GravityReturnCode and GravityTransportType, which means it has not been able to generate the bindings for any GravityNode method that uses this **A LOT OF THEM DO...**

It is also possible to use subclasses with autocxx, but generating these bindings is experimental and often interferes with other methods we are trying to bind. It may be possible for the bindings to be generated separatly for the non-inheritance based classes, and then a binder for each inheritance based one. Stay Tuned

Another thought is use autocxx for strictly the subclass stuff, and then manually  bind the rest, in case the above does not work. 

## Overview
### Basics
In this section, you will learn how I set up the rust Gravity API and the motivation behind it.

It is also important to note that regular CXX does not provide the functionality to call non-const member functions. In order to work around this I created functions that take in an object as a parameter and then call the member functions from there on the C++ side. But as you can imagine that is kind of yucky to deal with, so in order to give the *illusion* of normalcy, I created a struct on the Rust side that holds the object in a unique pointer (so it can communicate with C++ since Rust cannot directly hold C++ values), and gave it functions that call the C++ wrappers. 

### Dependencies
In your cargo.toml folder, include: 
```Rust
[dependencies]
cxx = "1.0"
autocxx = "0.28.0"


[build-dependencies]
cxx-build = "1.0"
autocxx-build = "0.28.0"

```

> [!NOTE]
> The latest version of autocxx is 0.30.0, but there a number of issues that prevented  this from working properly with this version.

### Distribution
In the src folder you will see 3 files: ffi.rs, gravity.rs, main.rs

#### ffi.rs
This contains all the bindings for the C++ code. The functions here all are in this format:
```Rust
#[rust_name = "fn_name"]
fn cpp_name(parameter: type) -> return_type;
```
This lets you call the C++ function cpp_name in rust with the name fn_name. The renaming is purely for simplicity. 

#### gravity.rs
This contains the struct and impl of the gravity types. The goal is that your main.rs calls the functions here, which call the functions in ffi.rs, which actually call the C++.

I know there is a lot of bouncing around and wrapper functions, but to me it seems like this is the best way to make the main.rs feel like actual rust code. Any suggestions are definitely welcome.


## Current Updates

I have included a wrapper on each end (C++ and Rust). This makes the code feel like true Rust code, while not having to impact any of the source code.

The provided stuff here will in fact make a gravity node publish a basic string data product successfully. Rust stuff cannot yet subscribe. Stay tuned. The next steps that I will take are either do the protobufs or work with the subscriber. Neither are easy since there is not support for a vector of shared pointers yet, nor protobuf, so some janky work-arounds are in order for sure! Exciting :/

June 12, 2025