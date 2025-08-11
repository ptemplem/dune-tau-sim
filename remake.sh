cd ../dk2nu
export DK2NU=$(pwd)
cp ../dk2nu-source/tree/* tree
make
cd -
rm sim
make sim
