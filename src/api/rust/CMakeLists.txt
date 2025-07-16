cmake_minimum_required(VERSION 3.17)

include(FetchContent)
project(Gravity)

set(RUST_LIB rust_gravity)
# FetchContent_Declare(
#     Corrosion
#     GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
#     GIT_TAG v0.5
# )

# FetchContent_MakeAvailable(Corrosion)

# corrosion_import_crate(MANIFEST_PATH Cargo.toml)

add_custom_target(
    ${RUST_LIB}
    COMMAND cargo build
)