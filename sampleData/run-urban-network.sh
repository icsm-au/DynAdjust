#! /bin/bash

# Script to automate processing of urban survey control network using phased adjustment
dnaimport -n urban urban-network.stn urban-network.msr
dnageoid urban -g urban-network-geoid.gsb --convert-stn-hts
dnasegment urban --min-inner-stns 50 --max-block-stns 250
dnaadjust urban --output-adj-msr --phased

# Check adjustment output identical to expected output
# (skipping first 62 lines, because these contain changing values)
diff <(tail -n +62 urban.phased.adj) <(tail -n +62 urban.phased.adj.expected) > /dev/null 2>&1
rv=$?

# Clean up
rm ./*.bms ./*.bst ./*.seg ./*.map ./*.imp ./*.adj ./*.xyz ./*.dbid ./*.dnaproj ./*.mtx ./*.aml ./*.asl

exit $rv
