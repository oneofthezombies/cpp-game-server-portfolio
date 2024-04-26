FROM ubuntu:24.04

SHELL ["/bin/bash", "-c"]

RUN apt-get update
RUN apt-get install -y git-all 
RUN apt-get install -y curl
RUN apt-get install -y build-essential
RUN apt-get install -y zlib1g-dev
RUN apt-get install -y libbz2-dev
RUN apt-get install -y libsqlite3-dev
RUN apt-get install -y libgdbm-dev
RUN apt-get install -y libssl-dev
RUN apt-get install -y libncurses5-dev
RUN apt-get install -y libreadline-dev
RUN apt-get install -y liblzma-dev
RUN apt-get install -y unzip
RUN apt-get install -y llvm
RUN apt-get install -y clang
RUN apt-get install -y lld
RUN apt-get install -y clangd
RUN rm -rf /var/lib/apt/lists/*

# python
RUN curl https://pyenv.run | bash
RUN echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
RUN echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
RUN echo 'eval "$(pyenv init -)"' >> ~/.bashrc

ENV HOME="/root"
ENV PYENV_ROOT="$HOME/.pyenv"
ENV PATH="$PYENV_ROOT/bin:$PATH"

RUN pyenv install 3.8 
RUN pyenv global 3.8

# cmake
RUN curl -L --output cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.29.2/cmake-3.29.2-linux-aarch64.tar.gz
RUN tar -xvf cmake.tar.gz
RUN mkdir -p $HOME/app
RUN mv cmake-3.29.2-linux-aarch64 $HOME/app/cmake
RUN rm cmake.tar.gz
RUN echo 'export PATH="$HOME/app/cmake/bin:$PATH"' >> ~/.bashrc

# ninja
RUN curl -L --output ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.12.0/ninja-linux-aarch64.zip
RUN unzip ninja.zip
RUN mkdir -p $HOME/app/ninja/bin
RUN mv ninja $HOME/app/ninja/bin/ninja
RUN rm ninja.zip
RUN echo 'export PATH="$HOME/app/ninja/bin:$PATH"' >> ~/.bashrc
