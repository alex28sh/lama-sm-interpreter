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
./lama_sm_interpreter -b ../examples/Sort.bc # printing readable bytecode instructions
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
* 6.4s - my version
* 15s - lama interpreter