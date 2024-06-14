# process_tracker

Simple utility that print runing process list to consol  
Example of view:
```sh
chrome.exe 13% (PID: 3156 User: DOMAIN\UserName)
```
Requirements:
`minimal Windows Vista/Server 2008`

### Building example:  
`conan 1.64.0`
`g++ 12.2.0 (MinGW)`
`boost 1.78.0`  

*In project folder*
```sh
mkdir build && cd build
conan install --build missing ..  -pr ..\conanprof.txt
cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
cmake --build . 
```