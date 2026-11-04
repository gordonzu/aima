# ![](https://github.com/aimacode/aima-java/blob/gh-pages/aima3e/images/aima3e.jpg)aima-cpp
C++ implementation of algorithms from [Russell](http://www.cs.berkeley.edu/~russell/) and [Norvig's](http://www.norvig.com/) [Artificial Intelligence - A Modern Approach 3rd Edition](http://aima.cs.berkeley.edu/) for  course work in the Columbia University Artificial Intelligence micro masters program.  

### Index of Implemented Algorithms

|Fig|Page|Name (in book)|Code|
| -------- |:--------:| :-----| :----- |
|2|34|Environment|[environment](/aicpp/src/environment/xyenv)|
|2.1|35|Agent|[agent](/aicpp/src/agent)|
|2.7|47|Table-Driven-Agent|[agent_programs](/aicpp/src/agent/agent_programs)|



## Building, testing and benchmarking

```bash
./mm          # Debug build + unit tests in build/ (unchanged behavior)
./mm bench    # Release builds in build/bench-O2 and build/bench-O3, runs xy_bench
./mm all      # Debug tests, then benchmarks
```

Benchmarks use [Google Benchmark](https://github.com/google/benchmark). If it is not installed
(`find_package(benchmark)`), CMake fetches v1.8.3 automatically. The benchmark targets are only
configured with `-DBUILD_BENCHMARKS=ON` (done by `mm bench`), so normal Debug builds don't need it.

JSON results go to the gitignored `bench-results/bench-O2.json` and `bench-O3.json`. Compare with
Google Benchmark's `compare.py` (`tools/compare.py benchmarks bench-O2.json bench-O3.json`).
Always benchmark Release builds; Debug results are not meaningful.
