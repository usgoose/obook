## A shared memory orderbook

This software, written in Python and C++ aims to provide a real-time orderbook for cryptocurrency trading.

| Feature | Motivation |
| ---- | --- |
| **Written in C++**| Allow fast insertion/deletion from websocket/FIX feeds |
| **Python Wrapped**| Allow integration with project using Python library such as exchannge APIs and Zipline/CCXT |
| **Residing in Shared memory (SHM)**| Allow multiprocess, independant, service-based running logic |
| **Rationals-based**| Allow bbetter precision than floats and doubles, and easy Python interfacing |
