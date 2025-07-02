# Rust for Gravity

A Rust API for gravity

[Recent Updates](#current-updates)

## CXX

Since SWIG does not have support for rust, the current implementation uses a crate called [CXX](https://cxx.rs). CXX allows for functions to call and be called from C++ to Rust. 

## Overview

### Current Functionality
Up to an including a Gravity PubSub kind of deal. GravityNodes can do everything except unsubscriber, request. ans provide services. I imagine these features will not be super tough to implement now I have the subscriber going!

### Basics
In this section, you will learn how I set up the rust Gravity API and the motivation behind it.

It is also important to note that regular CXX does not provide the functionality to call non-const member functions. In order to work around this I created functions that take in an object as a parameter and then call the member functions from there on the C++ side. But as you can imagine that is kind of yucky to deal with, so in order to give the *illusion* of normalcy, I created a struct on the Rust side that holds the object in a unique pointer (so it can communicate with C++ since Rust cannot directly hold C++ values), and gave it functions that call the C++ wrappers. 

### Dependencies
In your cargo.toml folder, include: 
```Rust
[dependencies]
cxx = "1.0"
protobuf = "3.7.2"

[build-dependencies]
cxx-build = "1.0"

```
### Distribution
In the src folder you will see a folder gravity with 2 files : gravity.rs and ffi.rs

#### ffi.rs
This contains all the bindings for the C++ code. The functions here all are in this format:
```Rust
#[rust_name = "fn_name"]
fn cpp_name(parameter: type, ..) -> return_type;
```
This lets you call the C++ function cpp_name in rust with the name ***"fn_name"***. The renaming is purely for simplicity. 

#### gravity.rs
This contains the struct and impl of the gravity types. The goal is that your main.rs calls the functions here, which call the functions in ffi.rs, which actually call the C++.

### shims.h
There is also a file called shims.h which does the C++ handling of the rust calls. Think of gravity.rs is the platform on the  rust side, shims.h/cpp as the platform on the C++ side, and ffi.rs as the bridge between them!


## Current Updates
***NEW*** <br>
The Subscriber works!
One issue is that I needed an internal function to set the data since I cannot send protobuf messages directly across, but I can send them as strings (kinda) and pointers. I created a method that directly takes in a pointer of data to set, but now as I am writing this out i realize that could potentially be bad. Not sure how to do this part yet without touching the source code.

**Last Updated July 2, 2025**