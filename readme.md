# tmake
tmake is a build system for C++ projects. 
it has its own simple scripting language to configure the build process.
it supports conditional compilation, custom build steps, and more.

# Building
to build tmake, run the following command in the root directory of the project:
```bash
./build.sh
```


# Usage

## tmake config <...>
to parse the build configuration file and generate the build files.
<...> represents the arguments passed to the build script, such as the mode, wich is then interpeted by the build script to determine how to configure the build process.

## tmake build
to build the project using the generated build files.

## tmake init
to initialize a new tmake project in the current directory, this will create a default `tmake.txt` file and a `build` and `src` directory for the source files.

## tmake clean
to clean the build files and the generated binaries.



# Editor Extension
tmake has a VS Code extension that provides syntax highlighting and autocomplete for `.tmake` files.

## Installing the VS Code Extension
requirements: VS Code, Node.js, npm

run the install script from the project root:
```bash
./editors/vscode/install.sh
```
this will install dependencies, compile the extension, and symlink it into your VS Code extensions directory. restart VS Code or reload the window to activate the extension.

# tmake lang
tmake is the scripting language used to configure the build process in tmake.
it has a simple syntax and supports variables, function calls and conditional statements.
the build configuration file is written in tmake and is usually named `tmake.txt`.

```js
// comment
/* multi-line comment */
var variable = "debug" // variable declaration
var array = ["flag1", "flag2"] // array declaration 
if (variable == "debug") { // conditional statement
    // do something
} else { 
    // do something else 
}
// PRINT is a built-in function that prints the given string to the console.
// arguments:
// to_print: a string or array representing the message to be printed
// PRINT(to_print)
PRINT("Hello, World!") 

// PROGRAM is a built-in function that builds a program, can be used to build multiple programs in the same project
// arguments:
// program_name: a string representing the name of the program to build
// files:        an array or string of source files to compile
// flags:        an array of string compiler flags to use when building the program
// output_dir:   a string representing the directory where the built program should be placed
// libraries:    an array of string representing the libraries to link against when building the program, this argument is optional
// PROGRAM(program_name, files, flags, output_dir, libraries)
PROGRAM("my_program", ["main.cpp", "utils.cpp"], ["-Wall", "-O2"], "bin", ["library1.dll", "library2.a"])

// VERSION is a built-in function that sets the version of c++ used to build the program, it can be excluded, in which case the default version will be used (currently c++17)
// arguments:
// version: a string representing the version of c++ to use when building the program, such as "17" for c++17, "20" for c++20, etc.
VERSION("17") 

// COMPILER is a built-in function that sets the compiler to be used when building the program, it can be excluded, in which case the default compiler will be used (currently g++)
// arguments:
// compiler: a string representing the compiler to use when building the program
COMPILER("g++")


// LANGUAGE is a built-in function that sets the programming language to be used when building the program, it can be excluded, in which case the default language will be used (default c++)
// arguments: 
// language: a string representing the programming language to use when building the program, such as "c" for C, "cpp" for C++, etc.
LANGUAGE("c") 



// string interpolation 
// replaces $1 with the value of the first argument passed to the script when running tbuild configure
"$1" 
// example: tbuild configure debug test
// "$1" will be replaced with "debug"
// "$2" will be replaced with "test"

```