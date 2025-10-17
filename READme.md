# Hy Compiler

Hy is a simple, educational compiler that translates a custom C-like programming language (called "Hy") into **x86-64 Linux assembly**. It then uses `nasm` and `ld` to assemble and link the code into a native executable.

The project is a practical demonstration of modern compiler front-end and back-end design, including tokenization, parsing into an Abstract Syntax Tree (AST), and direct code generation.

---

## Features

The Hy language and compiler currently support:
-   **Variable Declaration**: `int` variables.
-   **Variable Assignment**: Assigning values to declared variables.
-   **Integer Literals**: Using whole numbers in expressions.
-   **Arithmetic Expressions**: Addition (`+`), subtraction (`-`), multiplication (`*`), and division (`/`).
-   **Comparison**: Equality checks (`==`).
-   **Conditional Logic**: `if` statements with scopes (`{ ... }`).
-   **Program Exit**: Returning a final value from the program using `return(...)`, which becomes the executable's exit code.

---

## Architecture

The compiler is built in three main stages, a classic pipeline design:

1.  **Tokenizer (Lexer)**: The `Tokenizer` class reads the source code (`.hy` file) and converts it into a flat stream of tokens (e.g., `Token_Identifier`, `Token_Int`, `Token_Plus`).

2.  **Parser**: The `Parser` class consumes the stream of tokens and constructs an **Abstract Syntax Tree (AST)**. The AST is a hierarchical representation of the code's structure. This project uses an efficient **Arena Allocator** (`arena.hpp`) to manage memory for the AST nodes.

3.  **Generator**: The `Generator` class traverses the AST and emits the corresponding x86-64 assembly instructions for the **NASM assembler**. It manages variables and scopes by using the stack pointer (`rsp`).

Finally, the `main` function orchestrates this pipeline and calls the system's `nasm` and `ld` tools to produce the final executable.

---

## Project Structure
````
.
├── CMakeLists.txt      # Build configuration for CMake
├── my.hy               # Example source file
└── src/
    ├── main.cpp        # Main driver, handles file I/O and orchestrates the pipeline
    ├── lexer.hpp       # Contains the Tokenizer and token definitions
    ├── parser.hpp      # AST node definitions and the Parser class
    ├── arena.hpp       # Efficient memory arena allocator for the AST
    └── generation.hpp  # The code Generator class to produce assembly
````

---

## How to Build and Run

### Prerequisites
You must have the following tools installed on a Linux-based system (like Ubuntu, Debian, or WSL):
* `g++` (C++20 compatible)
* `cmake`
* `nasm`
* `ld` (linker)

### Steps

1.  **Clone the Repository**
    Clone or download the project files into a directory.

2.  **Create a Build Directory**
    It's best practice to build the project out-of-source.
    ```bash
    cmake -S . -B build
    ```

3.  **Build the Compiler**
    Run the build command from the root directory of the project.
    ```bash
    cmake --build build
    ```
    This will create an executable named `comp` inside the `build` directory.

4.  **Compile a `.hy` File**
    Use the newly built `comp` executable to compile your `.hy` source file. This will generate `out.asm`, `out.o`, and the final executable `out`.
    ```bash
    ./build/comp my.hy
    ```

5.  **Run the Executable**
    Execute the compiled program.
    ```bash
    ./build/out
    ```

6.  **Check the Exit Code**
    The value from the `return(...)` statement in your Hy code is passed as the program's exit code. You can check it with the `echo $?` command.
    ```bash
    echo $?
    ```

---

## Example

Here is a complete example using the provided `my.hy` file.

**1. Source File (`my.hy`)**
```cpp
// Variable declaration
int a;
int b;
int c;

// Assignment
a = 10;
b = 20;
c = a + b;

// Conditional
if (c == 30) {
 c = c + 1;
}

return (c);
```
2. Compile and Run
```aiignore
# Build the compiler
cmake -S . -B build
cmake --build build

# Use the compiler to build the Hy program
./build/comp my.hy

# Run the final executable
./out

# Check the result. It should be 31.
echo $?
# output: 31
```