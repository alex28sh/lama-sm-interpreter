### Usage

#### Build
```
cd runtime 
make
cd ..
cmake .
make # Or any other generator
```

#### Run

```
./lama_sm_interpreter -i ../examples/Sort.bc # interpreting
or 
./lama_sm_interpreter -p ../examples/Sort.bc # printing readable bytecode instructions
or 
./lama_sm_interpreter -a ../examples/Sort.bc # analyze instruction sequences frequency
```

You can check on programs in `examples` directory. 

### Performance

Running my version:
```
lamac -b Sort.lama
time ./lama_sm_interpreter -i Sort.bc
```

Running lama interpreter:
```
lamac -i Sort.lama
```

##### Results:
* 25s - my version
* 15s - lama interpreter

### Instruction sequences analysis

Analyzer is placed in [src/analyzer](src/analyzer).

Running:
```
./lama_sm_interpreter -p Sort.bc
```

#### Results (truncated):

```
Parametrized count for length 1
DROP: 41
DUP: 35
ELEM: 25
CONST 1: 18
CONST 0: 14
LD Argument 0: 8
END: 6
JMP 908: 5
ST Local 0: 4
SEXP 0 2: 4
LD Local 0: 4
TAG 0 2: 3
ARRAY 2: 3
LD Local 3: 3
LD Local 1: 3
JMP 496: 3
CALL_ARRAY 2: 3
CALL 497 1: 3
...

Parametrized count for length 2
CONST 1; ELEM: 15
DROP; DUP: 14
DROP; DROP: 13
DUP; CONST 1: 13
CONST 0; ELEM: 10
DUP; CONST 0: 9
ELEM; DROP: 9
DUP; DUP: 5
ST Local 0; DROP: 4
ELEM; ST Local 0: 4
DUP; TAG 0 2: 3
DUP; ARRAY 2: 3
CALL_ARRAY 2; JMP 908: 3
DROP; LD Local 0: 3
...

Process finished with exit code 0

```