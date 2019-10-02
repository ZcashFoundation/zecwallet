FROM ubuntu:16.04
LABEL Description="Ubuntu 16.04 for static Qt 5.11.2"

# Dependencies of the Qt offline installer
RUN apt-get -y update && apt-get install -y \
    curl libdbus-1-3 libexpat1 \
    libgl1-mesa-dev libglu1-mesa-dev libfontconfig1-dev libssl-dev  \
    libfreetype6 libgl1-mesa-glx libglib2.0-0 \
    libx11-6 libx11-xcb1 \
    g++ build-essential cmake wget git clang++-6.0 \
    software-properties-common  \
    autoconf automake autopoint bison flex gperf libtool libtool-bin intltool lzip python ruby unzip p7zip-full libgdk-pixbuf2.0-dev libltdl-dev 

# Hack to make clang work with Qt
RUN ln -s /usr/bin/clang++-6.0 /usr/bin/clang++ && \
    ln -s /usr/bin/clang-6.0   /usr/bin/clang

# Get OpenSSL
RUN cd /opt && wget https://www.openssl.org/source/openssl-1.0.2r.tar.gz && \
    tar xvf openssl-1.0.2r.tar.gz && \
    cd openssl-1.0.2r && ./Configure linux-x86_64 && make -j$(nproc) && \
    cd /opt && rm openssl-1.0.2r.tar.gz

# Get Qt5.11.2
RUN cd /opt &&  \
    wget https://download.qt.io/archive/qt/5.11/5.11.2/single/qt-everywhere-src-5.11.2.tar.xz && \
    tar xvf qt-everywhere-src-5.11.2.tar.xz && \
    cd qt-everywhere-src-5.11.2 && \
    OPENSSL_LIBS='-L/opt/openssl-1.0.2r -lssl -lcrypto' ./configure -static -prefix /opt/Qt/5.11.2/static  -skip qtlocation -skip qtmacextras -skip qtpurchasing -skip qtscript -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qtdatavis3d -skip qtdoc -skip qtcharts -skip qtdeclarative -skip qt3d -skip qtwebengine -skip qtandroidextras -skip qtwebview -skip qtgamepad -skip qtquickcontrols -skip qtquickcontrols2 -skip qtremoteobjects -skip qtwebview -skip qtwebchannel -skip qtwebglplugin  -nomake examples -nomake tests -qt-zlib -qt-libpng -qt-xcb -qt-xkbcommon -feature-fontconfig -no-feature-getentropy  -release -openssl-linked -platform linux-clang -opensource -confirm-license 

# Run the make multiple times, because for some reason (multithreading, probably) it fails
# the first time.
RUN cd /opt/qt-everywhere-src-5.11.2 && ( make -j$(nproc) || make -j4 || make )

RUN mkdir -p /opt/Qt/5.11.2 && cd /opt/qt-everywhere-src-5.11.2 && make -j4 install

RUN cd /opt && rm qt-everywhere-src-5.11.2.tar.xz && rm -rf qt-everywhere-src-5.11.2

# Get and build MXE
RUN cd /opt && \
    git clone https://github.com/mxe/mxe.git && \
    cd /opt/mxe && \
    make -j$(nproc) MXE_TARGETS=x86_64-w64-mingw32.static qtbase qtwebsockets 

# Add rust
RUN apt install -y gcc-aarch64-linux-gnu

RUN curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain 1.38.0  -y 
RUN echo 'source $HOME/.cargo/env' >> $HOME/.bashrc
RUN ~/.cargo/bin/rustup target add x86_64-pc-windows-gnu
RUN ~/.cargo/bin/rustup target add aarch64-unknown-linux-gnu

# Append the linker to the cargo config for Windows cross compile
RUN echo "[target.x86_64-pc-windows-gnu]" >> ~/.cargo/config && \
    echo "linker = 'x86_64-w64-mingw32.static-gcc'" >> ~/.cargo/config

RUN echo "[target.aarch64-unknown-linux-gnu]" >> ~/.cargo/config && \
    echo "linker = '/usr/bin/aarch64-linux-gnu-gcc'" >> ~/.cargo/config

ENV CC_x86_64_pc_windows_gnu="x86_64-w64-mingw32.static-gcc"
ENV CC_aarch64_unknown_linux_gnu="aarch64-linux-gnu-gcc"
ENV PATH="/opt/mxe/usr/bin:${PATH}"
