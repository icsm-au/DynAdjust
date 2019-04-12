# Script to automate processing of skye geodetic network
dnaimport -n gnss gnss-network.stn gnss-network.msr
dnageoid gnss -g gnss-network-geoid.gsb --convert-stn-hts
dnaadjust gnss --output-adj-msr

# Check adjustment output identical to expected output
# (skipping first 53 lines, because these contain changing values)
diff <(tail -n +53 gnss.simult.adj) <(tail -n +53 gnss.simult.adj.expected)
