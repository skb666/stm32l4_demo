# stm32l4_demo

```bash
cmake -G "Unix Makefiles" -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=toolchains.cmake
cmake --build build --target all -- -j${nproc}
```
