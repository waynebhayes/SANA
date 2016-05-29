#!/bin/bash
set -e

branch=$1
version=$2

echo "Building ghost [branch = ${branch}]. Tagging version as ${version}"

# Activate Holy Build Box environment.
source /hbb_exe/activate

set -x

# Install things we need
yum install -y --quiet wget
wget http://download.fedoraproject.org/pub/epel/5/x86_64/epel-release-5-4.noarch.rpm
rpm -i --quiet epel-release-5-4.noarch.rpm
#yum install -y --quiet git
yum install -y --quiet unzip
yum install -y --quiet bzip2-devel.x86_64

curl -k -L http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download -o boost_1_55_0.tar.gz
tar xzf boost_1_55_0.tar.gz
cd boost_1_55_0
./bootstrap.sh --with-libraries=iostreams,system,thread
./b2 -j 8 link=static install

cd ..
curl -k -L https://github.com/Kingsford-Group/ghost2/archive/${branch}.zip -o ${branch}.zip
unzip ${branch}.zip
mv ghost2-${branch} ghost2
cd ghost2
make
mkdir GHOST_x86_64-${version}
cp GHOST GHOST_x86_64-${version}
tar czf GHOST_x86_64-${version}.tar.gz GHOST_x86_64-${version}
cp GHOST_x86_64-${version}.tar.gz /io/
