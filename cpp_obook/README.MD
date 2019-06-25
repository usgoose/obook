## A shared memory orderbook

This software, written in Python and C++ aims to provide a real-time orderbook for cryptocurrency trading.

| Feature | Motivation |
| ---- | --- |
| **Written in C++**| Allow fast insertion/deletion and easy interation with Unix SHM |
| **Python Wrapped**| Allow integration with project using Python library such as exchannge APIs and Zipline/CCXT  |
| **Shared memory (SHM)**| Allow multiprocess, independant, service-based running logic |
