# tmake Workflow

## `tmake init`
- Generates a file named `tmake.txt` with the following content:

```js
// tmake configuration file
// files: a list of source files to be compiled
// debug_flags: a list of flags to be used when compiling in debug mode
// release_flags: a list of flags to be used when compiling in release mode
// output_directory: the directory where the compiled files will be placed
// output_name: the name of the output file
// PROGRAM(var program_name, var files, var flags, var output_dir) is a function that compiles the source files with the specified flags and outputs to the specified directory
// COMPILER(var compiler) is a function that sets the compiler to be used, default is g++
// VERSION(var version) is a function that sets the C++ language standard version to be used, default is 17

// List of source files to be compiled
// "src/**.cpp"   can be used to include all cpp files in the src directory and its subdirectories
// "src/*.cpp"    can be used to include all cpp files in the src directory
// "src/test.cpp" can be used to include a specific file
var files = {
    "src/file1.cpp",
    "src/file2.cpp",
    // ...
}

// List of flags for debug mode
var debug_flags = {
    "flag1",
    "flag2",
    // ...
}

// List of flags for release mode
var release_flags = {
    "flag1",
    "flag2",
    // ...
}

// The name of the output file and the output directory
var program_name = "program"
var output_directory = "/bin"

// Set the compiler to be used
var compiler = "g++"
COMPILER(compiler)

// Set the C++ language standard version to be used
var version = "17"
VERSION(version)

// Compile the source files with the specified flags and output to the specified directory based on the build mode
if "$1" == "debug" {
    PROGRAM(program_name, files, debug_flags, output_directory)
}
else if "$1" == "release" {
    PROGRAM(program_name, files, release_flags, output_directory)
}
else {
    PRINT("Invalid build mode. Use 'debug' or 'release'.")
}
```

- Generates a `src` directory if not exists
- Generates a `build` directory if not exists

---

## `tmake clean`
- Clears the build directory

---

## tmake

## `tmake config ...`
- Reads the `tmake.txt` file
- Parses the configuration and checks for errors 
- use the ... as replacement for the "$..." strings in the file
- Gets the list of source files, flags, output directory and output name
- Generates a file `tmake.config` with the configuration

## `tmake build`
- Reads the `tmake.config` file
- check if the configuration is valid and if the source files exist
- if the cache exists and the source files and dependencies have not changed since the last build, skip the compilation and display a message that the build is up to date
- generates a cache file with the last modified time of the source files
- Compiles the source files with the specified flags and outputs to the specified directory
- Displays a success message when the compilation is complete
- 


## tmake help
- Displays the help message with the list of available commands and their descriptions.

## tmake version
- Displays the current version of the `tmake` tool.

## tmake 
- Displays the help message with the list of available commands and their descriptions.