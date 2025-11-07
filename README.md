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
./lama_sm_interpreter -a Sort.bc
```

#### Results (truncated):

```
DROP : 31
DUP : 28
ELEM : 21
CONST 1 : 16
CONST 1; ELEM : 13
CONST 0 : 11
DUP; CONST 1 : 11
DROP; DUP : 11
DROP; DROP : 10
CONST 0; ELEM : 8
LD 34 0 : 7
DUP; CONST 0 : 7
ELEM; DROP : 7
END : 5
JMP 908 : 4
SEXP 0 2 : 4
DUP; DUP : 4
ELEM; ST 65 0 : 3
DUP; ARRAY 2 : 3
ST 65 0; DROP : 3
CALL_ARRAY 2 : 3
ARRAY 2 : 3
CALL 497 1 : 3
ST 65 0 : 3
CALL_ARRAY 2; JMP 908 : 3
LD 33 3 : 3
LD 33 0 : 3
JMP 496 : 3
...

Process finished with exit code 0

```
