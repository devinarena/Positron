# Positron

## !! Unfinished and experimental software, subject to changes and is not suitable for production at this time. !!

## About
A bytecode VM interpreter written in C. Positron is based on another unfinished language I wrote called Palladium, which was simply too clunky and not elegant to use. I decided to take what I learned and create a new programming language with the goal of making solving programming-style problems easier and more fun. The goal is to create a language that elegantly handles data structures and algorithms with python-like wordiness.

## Requirements
- gcc > (determine version)
- GNU Make (optional)

## Building
To build the Positron executable, download the source and either use make:
```sh
make
```
or simply compile the executable manually with gcc
```sh
gcc src/*.c -o positron
```

## Running
To run the REPL, simply run
```sh
./positron
.\positron.exe
```
To run a .pt file, simply run
```sh
./positron <file>
.\positron.exe <file>
```

## Examples
### Print keyword will be replaced with a call to wln() in the future
"Hello, World" written in Positron:
```
print "Hello, world"
```
Print a string 10 times:
```
vd print_n(str text, i32 count) {
  for (i32 i = 0; i < count; i = i + 1) {
    print text
  }
}

print_n("Hello, world!", 10);
```

## License and Contribute
Published under the MIT License. Contributions and ideas are welcome.
