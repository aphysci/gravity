# Rust for Gravity

A Rust API for gravity

[Recent Updates](#current-updates)

## CXX

Since SWIG does not have support for rust, the current implementation uses a crate called [CXX](https://cxx.rs). CXX allows for functions to call and be called from C++ to Rust. 

## Overview

### Current Functionality
Nearly done. I expect to have full functionality by the end of the week (by July 11) at the latest. The only 2 things I do not have are the subscriptionMonitor and HeartbeatListeners. I will also be going back to the logging thing when I finish this.

There is also now zero change to the original GravityAPI so it should integrate with it very nicely. The only thing that has to compile is the shims.h/cpp files and be linked with gravity libraries. I need to figure out how to make them compile separately (so c++ users do not accidentally call some weird rust bridge function).

In addition, this is now a library so you can use it outside of the crate. (im glad because some fields needed to be crate visible, but would be very bad if users could access).

The cargo.toml for it looks like this now. The path is the path to the current cargo project. Of course this is subject to change, and hopefully the rust library is able to use environmental variables so when the gravity build eventually puts the rust library in the right place all a user would have to do is specify that it is the LD_LIBRARY_PATH. tbd

```rust
[package]
name = "gravity_user"
version = "0.1.0"
edition = "2024"

[dependencies]
gravity = { path = "/home/anson/gravity/src/api/rust"}
protobuf = "3.7.2"
```

### Tests

uhhhh... none yet.
I have been doing some light testing of basic functionality as I have been moving through, but thorough testing will be done soon. Once I finish implementing all the GravityNode methods.


## Current Updates
***NEW*** <br>
It is nearly time to integrate. I am (maybe) looking forward to it.

**Last Updated July 2, 2025**