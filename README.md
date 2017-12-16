Build example with: 

```sh
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake ..
cmake --build . --target example
```

And then drop `index.html` and `index.js` into any http server folder
(or run python `http.server` in example folder)
