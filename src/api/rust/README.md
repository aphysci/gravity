# Rust for Gravity

A Rust API for gravity

## Usage

### Dependencies 

**Requires cargo 1.85 or later**
On linux, this can be easily installed through the following:
```sh
sudo apt install rustup
rustup update stable
```

Rustup will automatically install the latest stable release of rust compiler.


Before you add it to your ```Cargo.toml```, ensure that you have all the Gravity dependencies installed, specified in the [Gravity Build Guide](https://github.com/aphysci/gravity/wiki/GravitySetup).

Not all are necessary, just g++, cmake, bison, flex.

In linux:
```sh
sudo apt install cmake g++ bison flex
```

Then, add the crate to your Cargo.toml:
```toml
[dependencies]
gravity = { git = "REPOSITORY_URL" }
```

You are done! Now you can use Gravity from your Rust application!

> [!NOTE]
> Using Gravity this way does not create/run the ServiceDirectory. Follow the [Gravity Build Guide](https://github.com/aphysci/gravity/wiki/GravitySetup) to set that up.
>

## Tests
To run the unit tests, you need a Service Directory running. Then simply run ``` cargo test ```. As long you are not in the examples folder, the folder you run it in does not matter. (If you are in the examples it will run the specific example)

## Runnable examples

Once there is a ServiceDirectory setup, go into the example directory you want to run and ``` cargo run ```