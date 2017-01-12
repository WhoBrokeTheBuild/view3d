#!/bin/sh

function get_distro()
{
    which dnf &>/dev/null && { echo fedora; return; }
    which yum &>/dev/null && { echo centos; return; }
    which zypper &>/dev/null && { echo opensuse; return; }
    which apt-get &>/dev/null && { echo debian; return; }
}

distro=`get_distro`

case $distro in
    fedora)
        sudo dnf install -y \
            make \
            clang \
            freeglut-devel \
            glew-devel \
            check-devel \
            zlib-devel
        ;;
    centos)
        sudo yum install -y \
            make \
            clang \
            freeglut-devel \
            glew-devel \
            check-devel \
            zlib-devel
        ;;
    * )
        echo "Unsupported Distro"
        ;;
esac
