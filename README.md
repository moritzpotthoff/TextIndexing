# TextIndexing

## Building and Execution

To build, run the following commands from the main folder `Framework`:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the program using 
```
./build/Framework [topk|repeat] path_to_input_file
```