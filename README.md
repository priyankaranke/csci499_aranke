  
<h1> Class project for CSCI499 (Spring 2020) </h1>  

<h2>Overview</h2>  

<p> In this project you are to build the software infrastructure for a new Function-as-a-Service (FaaS) platform we’ll call  
Func, and on top of Func build the infrastructure for a social network platform we’ll call Warble.  Func is intended to
provide a very basic FaaS platform like AWS Lambda or Google Cloud Functions, and Warble is intended to have the same basic  
functionality of Twitter. </p>

<p> A service operator will register a function f and its associated event type t, and when your FaaS platform receives a request that matches event type t it will pass that request to f for processing.  Functions that are executed in this manner have no persistent state, so Func will also provide access to a key-value store for the functions to store their data. </p>

<p> Using this FaaS functionality, you will build an implementation of Warble with a command-line interface, which will enable  
Twitter-like functionality.  Users of Warble will register for user accounts on Warble (though we will not be concerned with  
authentication and security), post “warbles” (akin to tweets), follow other users, and be followed by other users.  They will  
be able to reply to warbles (either their own or those of other users). </p>

<p> Full design doc (https://docs.google.com/document/d/1TCxCbmyiO5mpiuvH8q9FZMXzt7nT2Drwecho8i3bW6g) </p>

<h2>Build the project</h2>    

1. Create a new directory and clone csci499_aranke repo into that using  
`git clone https://github.com/priyankaranke/csci499_aranke.git`  
  
2. ```
   cd csci499_aranke    
   vagrant up  
   vagrant ssh  
   ```
  
3. If the commands from bootstrap.sh didn't run automatically on ssh, copy paste it    
all into terminal and let the packages install  
  
4. Install CMake using instructions on 'Install CMake through the Ubuntu Command Line'  
section on 'https://vitux.com/how-to-install-cmake-on-ubuntu-18-04/'  

5. Get glog:  
```
git clone https://github.com/google/glog.git
cd glog  
cmake -H. -Bbuild -G "Unix Makefiles"  
cmake --build build
```

6. Build using:  
```
mkdir build  
cd build  
cmake ..  
make  
```

<h2>Run the app</h2>  

1. Open up 3 terminal windows  
2. `vagrant ssh` into your machine on all  
3. `./src/func_main` in one, `./src/key_value_store_main` in another  
4. `./src/warble_main` in the third. This will be where you use Warble.  
The command line will instruct you on correct usage of flags

<h2>Run tests</h2>  

Run `./tests/all_tests` from within the same directory you ran make in (inside build)
