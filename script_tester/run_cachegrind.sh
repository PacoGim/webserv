#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 <binary> [args...]"
  exit 1
fi

binary=$1
shift

#commentaire multiligne
: <<'COMMENTAIRE'
CACHEGRIND OPTIONS
       --I1=<size>,<associativity>,<line size>
           Specify the size, associativity and line size of the level 1
           instruction cache.

       --D1=<size>,<associativity>,<line size>
           Specify the size, associativity and line size of the level 1 data
           cache.

       --LL=<size>,<associativity>,<line size>
           Specify the size, associativity and line size of the last-level
           cache.

       --cache-sim=no|yes [yes]
           Enables or disables collection of cache access and miss counts.

       --branch-sim=no|yes [no]
           Enables or disables collection of branch instruction and
           misprediction counts. By default this is disabled as it slows
           Cachegrind down by approximately 25%. Note that you cannot
           specify --cache-sim=no and --branch-sim=no together, as that
           would leave Cachegrind with no information to collect.

       --cachegrind-out-file=<file>
           Write the profile data to file rather than to the default output
           file, cachegrind.out.<pid>. The %p and %q format specifiers can
           be used to embed the process ID and/or the contents of an
           environment variable in the name, as is the case for the core
           option --log-file.

$ lscpu --cache
NAME ONE-SIZE ALL-SIZE WAYS TYPE        LEVEL  SETS PHY-LINE COHERENCY-SIZE >> OneSize in bytes, ways, coherencySize
L1d       48K     192K   12 Data            1    64        1             64 >> 48*1024,           12,    64
L1i       32K     128K    8 Instruction     1    64        1             64 >> 32*1024,            6,    64
L2       1.3M       5M   20 Unified         2  1024        1             64
L3        12M      12M   12 Unified         3 16384        1             64 >> 12*1024*1024,      12,    64

COMMENTAIRE
#fin commentaire multiligne

\valgrind --tool=cachegrind \
  --cachegrind-out-file=cachegrind.out."${binary}" \
  --D1=49152,12,64 \
  --I1=32768,8,64 \
  --LL=12582912,12,64 \
  --branch-sim=yes \
  "./$binary" "$@"
