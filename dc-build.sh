docker run -v `pwd`:/simulant:Z kazade/dreamcast-sdk /bin/sh -c "source /etc/bash.bashrc; mkdir -p /simulant/dbuild; cd /simulant/dbuild; cmake -DCMAKE_TOOLCHAIN_FILE=/simulant/toolchains/Dreamcast.cmake -DCMAKE_BUILD_TYPE=Release .. && make"
