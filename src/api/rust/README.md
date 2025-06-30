# Rust for Gravity

A Rust API for gravity

[Recent Updates](#current-updates)

## CXX

Since SWIG does not have support for rust, the current implementation uses a crate called [CXX](https://cxx.rs). CXX allows for functions to call and be called from C++ to Rust. 

## AUTOCXX
[Autocxx](https://google.github.io/autocxx/index.html) is a more powerful tool to bind Rust to C++. However, it does have some limitations to it. From what I have experimented with so far, it sometimes has trouble with user defined data types. In particular, it has not been able to see GravityReturnCode and GravityTransportType, which means it has not been able to generate the bindings for any GravityNode method that uses this **A LOT OF THEM DO...**

It is also possible to use subclasses with autocxx, but generating these bindings is experimental and often interferes with other methods we are trying to bind. It may be possible for the bindings to be generated separatly for the non-inheritance based classes, and then a binder for each inheritance based one. Stay Tuned

Another thought is use autocxx for strictly the subclass stuff, and then manually  bind the rest, in case the above does not work. 

## Overview
### Current Functionality
You don't care how it works, only that it does.
I have implemented all the spdlog functions, in addition to every function you see before you in main. There really is not much else so far. Most functions are trivial to implement, I just have not gotten around to it so far. I hope to get the subscriber going soon. If I can get that moving, service providers, requestors, etc, will be very similar I imagine. 

To the ~0 people reading this. hi!

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

The autocxx is actually not being used right now since I have made everything work with cxx.

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

I believe this is what they call a shim

I have included a wrapper on each end (C++ and Rust). This makes the code feel like true Rust code, while not having to impact any of the source code.


## Current Updates
***NEW*** <br>
The subscriber works! (ish)

The only way I have been able to get it to work (may not be the only possible solution...) is to have C++ do all the inheritance, and all rust supplies is a function pointer that points to the Rust subscriptionFilled function.

I think I can get this to work, but we shall see. In addition, I need to work through the populateMessage function for GravityDataProduct, since this is pretty key in subscriber functionality, and this is not trivial since you cannot pass protobufs directly to C++ (AFAIK). If my knowledge changes than this becomes way easier, but I think I can wrangle this.

Currently for it to work properly the user has to define a subscriptionFilled function with parameter **&CxxVector<GDataProduct>**, where the goal would be the user impl the GravitySubscriber trait that has a fn subscriptionFilled(&Vec<GravityDataProduct>) on a struct and pass that to subscribe.

In addition, all the non-inheritance based methods are implemented for GravityNode in rust, and I am starting to work through GravityDataProduct :)

**Last Updated June 27, 2025**