# Common Subexpression Eliminator.
A local block subexpression eliminator in C. 
Builds signatures of ILOC instructions, hashes them, then parses through the code, removing
and replacing instructions as able to minimze the total number of instructions.
## Usage
Generate an executable with 
```make```

The executable expects input to stdin, so it should look like

```./codegen < demo```

or


```./codegen -O < demo```

## Flags
The [-O] flag represents the optimizer running. 
* The first code block represents an unoptimized pass of the compiler.
* The second code block represents the optimizer working.
