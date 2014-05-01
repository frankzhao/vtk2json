# Script for installation of libc++
# Frank Zhao April 2014
# frank.zhao@anu.edu.au

cd $HOME
sudo apt-get install g++ subversion cmake make
svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
mkdir build_libcxx && cd build_libcxx
CC=clang CXX=clang++ cmake -G "Unix Makefiles" -DLIBCXX_CXX_ABI=libsupc++ -DLIBCXX_LIBSUPCXX_INCLUDE_PATHS="/usr/include/c++/4.6/;/usr/include/c++/4.6/x86_64-linux-gnu/" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr $HOME/libcxx
make -j 8
sudo make install
