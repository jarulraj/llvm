echo "Recompiling sample.c"
rm sample
gcc -o sample sample.c -msse4 -static
