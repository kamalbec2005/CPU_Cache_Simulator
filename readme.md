-----Cache Simulator----
https://github.com/kamalbec2005/CPU_Cache_Simulator.git

Description:
===========
Writing code which meets functionality is a must. Optimizing the code to fit into less program memory and less number of cycles is experts job. Performance improvement makes the special place for your code. 
Cache hit/miss is one important among perforamnce metrics. We can use "perf" tool to measure the same, but.... it gives cache hit/miss against the same processor L1 cache size only. It is good if there is a tool which can simulate the cache size, maping model, replacing algorithm, memory block size, with prefetch/without prefetch.
Here is the tool which can allow user to set the configuration of cache settings and get cache hit/miss metrics.
Note: User has to collect the number of address locations to pass as input to the tool. "trace2.txt" is the input to the tool which has list of accessed address locations.


Code: 
=====
Directory: /code

Example:
=======
$./first 32 assoc:2 lru 4 trace2.txt
no-prefetch
Memory reads: 3292
Memory writes: 2861
Cache hits: 6708
Cache misses: 3292
with-prefetch
Memory reads: 3315
Memory writes: 2861
Cache hits: 8331
Cache misses: 1669


