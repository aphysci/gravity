# Rust for Gravity

A Rust API for gravity

## Usage
If you want to include it in your Rust project, ensure that you have all the Gravity dependencies installed, specified in the [Gravity Build Guide](https://github.com/aphysci/gravity/wiki/GravitySetup).

Not all are necessary, just g++, cmake, bison, flex.

In linux:
```sh
sudo apt install cmake g++ bison flex
```

Then add the crate to your Cargo.toml:
```toml
[dependencies]
gravity = { git = "https://github.com/astrauc/gravity.git" }
```

You are done! Now you can use Gravity from your Rust application!

> [!NOTE]
> Using Gravity this way does not create/run the ServiceDirectory. Follow the [Gravity Build Guide](https://github.com/aphysci/gravity/wiki/GravitySetup) to set that up.
>

## Tests
To run the unit tests, you need a Service Directory running. Then simply run ``` cargo test ```. As long you are not in the examples folder, the folder you run it in does not matter. (If you are in the examples it will run the specific example)

## Runnable examples

Once there is a ServiceDirectory setup, go into the example directory you want to run and ``` cargo run ```