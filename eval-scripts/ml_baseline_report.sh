#!/bin/bash

echo "O0 vs O3"

echo "Results of GMN"
find ml-baseline-results/how-solve-gmn -name "*.sret" -and -name "*O0*O3*"|xargs egrep "All"

echo "Results of GNN"
find ml-baseline-results/how-solve-gnn -name "*.sret" -and -name "*O0*O3*"|xargs egrep "All"

echo "O2 vs O3"

echo "Results of GMN"
find ml-baseline-results/how-solve-gmn -name "*.sret" -and -name "*O2*O3*"|xargs egrep "All"

echo "Results of GNN"
find ml-baseline-results/how-solve-gnn -name "*.sret" -and -name "*O2*O3*"|xargs egrep "All"