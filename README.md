# CVM++ — Custom Virtual Machine in C++

A simple scripting language built from scratch in C++.
code goes through 4 stages: **Lexer → Parser → Compiler → VM**

---

## Build

```bash
g++ -std=c++17 -I include src/*.cpp -o cvm
```

## Run

```bash
./cvm script.cvm             # run a file
./cvm --ast --bc script.cvm  # debug: show AST + bytecode
./cvm                        # interactive REPL
```

---

## Language Syntax

```js
// Variables
let x = 10;
let flag = true;
x = x + 1;

// Print & Input
print(x);
input(x);   // reads a number into x

// If / Else
if (x > 5) {
    print(x);
} else {
    print(0);
}

// While loop
while (x > 0) {
    print(x);
    x = x - 1;
}
```

**Operators:** `+  -  *  /  ==  !=  <  >`

---

## Test Scripts

| File | What it tests |
|------|--------------|
| `tests/test1.cvm` | Arithmetic |
| `tests/test2.cvm` | if/else |
| `tests/test3.cvm` | while loop |
| `tests/test4.cvm` | Fibonacci |

---

## REPL Commands

```
cvm> reset    # clear all variables
cvm> --ast    # toggle AST view
cvm> --bc     # toggle bytecode view
cvm> exit     # quit
```

## REPL Commands

```
cvm> reset    # clear all variables
cvm> --ast    # toggle AST view
cvm> --bc     # toggle bytecode view
cvm> exit     # quit
```
