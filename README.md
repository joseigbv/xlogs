# xlogs

Ultra-fast, minimalist logs parser and analyzer.

'xlogs' is a low level and spartan log parser. It arises from a very specific need: to analyze several TB of logs in a short time using general-purpose hardware and with few resources. A problem for which none of the options available at the time satisfied the need satisfactorily.

## Features

xLogs is made up of two templates:

* 'parser.c': an ultra-fast parser developed in the C language that transforms text logs into much more compact and user-friendly binary data.
* 'query.c': run a query by launching multiple threads, filter and process the results.

Currently, xlog is so spartan that it doesn't even have a user interface. All configurations and queries must be done by editing and compiling templates in C. Priority has been given to speed and the development and improvement of its main functionality. 

Its main features are:

* Ultra fast parser & query tools, developed in C.
* The parser converts and stores logs to binary format to optimize space and improve performance.
* Multi thread query tool.
* Can work on raw partition to reduce overhead and improve performance.
* Uses its own data structure as filesystem.
* light compression to improve transfer rate between disk and memory.
* A variable-size section of data where each text string is stored only once for your reference.
* A data section with fixed-size log records in chunks and references to the string pool to optimize performance.


## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

xlogs can be compiled on any Win32 or UNIX/Linux box. You only need a relatively modern C compiler.
It uses TommyDS libraries for dynamic hashs. 


### Installing

Download a copy of the project from github:

```
$ git clone https://github.com/joseigbv/xlogs.git

```

PENDING ...


### Usage

PENDING ...


## Authors

* **José Ignacio Bravo** - *Initial work* - nacho.bravo@gmail.com

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* http://www.tommyds.it 

