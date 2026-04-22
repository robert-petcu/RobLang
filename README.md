
# RobLang

RobLang is a custom interpreted pseudolanguage. The C++ interpreter parses the entire source code into a vector of strings and executes it line by line, managing nested blocks through recursion.

## 1. Data Types

The language supports four core data types and their vector (array) counterparts.

|**Data Type**|**Description**|**Syntax Example**|
|---|---|---|
|`int`|Stores integers|`int x`|
|`double`|Stores decimal numbers|`double y`|
|`bool`|Stores logical values|`bool status`|
|`string`|Stores character strings|`string name`|
|`vector_...`|Vector of a specific type|`int v[100]`|
|`matrix_...`|Matrix of a specific type|`double a[100][100]`|

Under the hood, all variables are dynamically stored in `std::unordered_map` structures corresponding to their data type (e.g., `memory_int`, `memory_string`, `memory_vector_double`).

## 2. Command Syntax

-   `read`
    
    Reads the value of a variable from `input.txt`.
    
    **Syntax:** `read <variable_name>`
    
    **Example:** `read x` (After this, `x` will hold the value read from the file).
    
-   `assign`
    
    Assigns a value or the result of an expression to a variable.
    
    **Syntax:** `assign <variable_name>: <value_or_expression>`
    
    **Examples:**
    
    -   `assign x: 10`
        
    -   `assign y: 2 * (z + 5)`
        
    -   `assign pi: 3.14`
        
    -   `assign v[i]: i % 10`
        
    -   `assign s: "Hello World!"`
        
-   `print`
    
    Prints output to `output.txt`.
    
    -   **Strings:** `print "Hello World!"`
        
    -   **Variables/Expressions:** `print x + 5`
        
    -   **Special case:** `print "endl"` outputs a newline character.
-   `//` (comment)
    
    Adds a comment line to the code.
    
    **Syntax:** ```// <comment>```
        

## 3. Expression Evaluator

The `evaluate_logic` function and its sub-functions (`evaluate_math`, `evaluate_term`, etc.) implement standard operator precedence without relying on external parsing libraries:

**1. Factors:** Numbers, variables, and expressions inside parentheses `()`.

**2. Terms:** Multiplication `*`, Division `/`, Modulo `%`.

**3. Math:** Addition `+`, Subtraction `-`.

**4. Comparisons:** `==`, `!=`, `<`, `>`, `<=`, `>=`.

**5. Logic:** AND `&&`, OR `||`.

## 4. Control Structures

All control structures use curly braces `{ }` to define code blocks.

-   `if` / `else if` / `else`
    
    Executes a specific code block based on a logical condition.
    
    **Syntax Example:**
    
    ```
    if x % 2 == 0 {
        print "even"
        print "endl"
    }
    else {
        print "odd"
        print "endl"
    }
    
    ```
    
-   `while`
    
    Executes a code block repeatedly as long as a logical condition remains true.
    
    **Syntax Example:**
    
    ```
    while i <= 10 {
        print i
        print "endl"
        assign i: i + 1
    }
    
    ```
    
-   `for`
    
    A loop that iterates based on numeric bounds.
    
    **Syntax Example:**
    
    ```
    for i 0 10 1 {
        ...
    }
    
    ```
    
    _(Meaning: "For an iterator `i` starting at 0, up to 10, increasing by 1 after each iteration.")_
    

## 5. Execution Logic

To handle loops, conditionals, and nested blocks, the interpreter relies on three core functions:

-   `read_block`
    
    When the interpreter encounters an opening brace `{`, this function reads all subsequent lines until it finds the matching closing brace `}`, keeping track of the nesting level. It returns a vector containing the block's lines.
    
-   `execute_block`
    
    Takes a vector of code lines and passes them to `execute_current_line` sequentially.
    
-   `execute_current_line`
    
    Parses the first word of the current line to determine which command or control structure to trigger.
    

## 6. Processing Example

Here is a complete RobLang program that generates the Fibonacci sequence:

```
int num_elements
int f[100]

assign f[0]: 0
assign f[1]: 1

read num_elements

for i 0 num_elements 1 {
    if i >= 2 {
        assign f[i]: f[i - 1] + f[i - 2]
    }
    print "f["
    print i
    print "] is "
    print f[i]
    print "endl"
}

```

### Step-by-step Breakdown:

**1. Declaring the data structures:**

The interpreter registers `num_elements` in `memory_int` and allocates an array of size 100 for `f` in `memory_vector_int`.

**2. Base cases initialization:**

The first two elements of the Fibonacci sequence (`f[0]` and `f[1]`) are assigned their standard starting values.

**3. Reading input:**

The `read` command fetches the target number from `input.txt` and stores it in `num_elements`.

**4. Iteration and logic:**

The `for` loop starts at `0` and runs up to `num_elements`. For any index `i >= 2`, the program computes the next number using the previously stored array values. It then uses sequential `print` statements to format the text seamlessly.

### Expected Output

If `input.txt` contains the number `10`, the program will write the following to `output.txt`:
```
f[0] is 0
f[1] is 1
f[2] is 1
f[3] is 2
f[4] is 3
f[5] is 5
f[6] is 8
f[7] is 13
f[8] is 21
f[9] is 34
f[10] is 55
```
