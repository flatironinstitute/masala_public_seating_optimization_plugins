mkdir bin && echo "Created bin/ directory." || echo "Directory bin/ already exists."
mkdir bin/tests && echo "Created bin/tests directory." || echo "Directory bin/tests already exists."
mkdir bin/api && echo "Created bin/api directory." || echo "Directory bin/api already exists."
mkdir build && echo "Created build/ directory." || echo "Directory build/ already exists."
cd build
mkdir cmake_generated && echo "Created build/cmake_generated/ sub-directory." || echo "Sub-directory build/cmake_generated/ already exists."
cmake ../cmake/
make -j `nproc`

test "${PIPESTATUS[0]}" != '0' && { echo "Build errors!  Check output log for details."; exit 1; } || { echo "Build completed."; }
cd ..

cd bin/tests
ln -s ../../build/*_tests ./
cd ../api
ln -s ../../build/generate_*_api ./
cd ..
# TODO link any applications here.
ln -s ../build/optimize_seating ./
cd ..

mkdir headers/seating_optimization_masala_plugins && echo "Created headers/seating_optimization_masala_plugins/ directory." || echo "Directory headers/seating_optimization_masala_plugins/ already exists."
rm -r headers/seating_optimization_masala_plugins/*
python3 ./code_templates/copy_headers.py seating_optimization_masala_plugins seating_optimization_api src headers/seating_optimization_masala_plugins
python3 ./code_templates/copy_headers.py seating_optimization_masala_plugins registration_api src headers/seating_optimization_masala_plugins

echo "Finished building project and copying headers."
