# TPCC

Tpcc is the compiler of the tpc language, which is a very subset of the C language.

## Contributors
- Ramaroson "Sacane" Johan
- Menaa "Veresta" Mathis

## How to use

To generate the compiler execute the command-line :
```shell
make
```

The tpcc file will be generated in the bin folder. 
To compile a tpc project, use the following command : 
```shell
./bin/tpcc <your_source.tpc>
```
or 
```shell
./bin/tpcc < votre_source.tpc
```
Here is some additional options that you can use with the tpcc compiler :

- -h/--help : to print how to use the options in the stdout
- -t/--tree : to print the tree in the stdout
- -s/--symtabs : to print all the symbol-tables
- -n/--noexec : to avoid generating the executable 

