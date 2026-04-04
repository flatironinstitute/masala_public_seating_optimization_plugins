# Run this script to remove output files.
rm -v *.log
rm -v problem_summary.txt
rm -v solution_*.txt
rm -v *.png
rm -v *.svg
cd test_scratch
./clean.sh
cd ..
