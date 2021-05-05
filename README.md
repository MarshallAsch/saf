![Code linter on main](https://img.shields.io/github/workflow/status/marshallasch/saf/Clang-format/main?style=plastic)
![ns3 build checker on main](https://img.shields.io/github/workflow/status/marshallasch/saf/build-ns3/main?style=plastic)
![GitHub](https://img.shields.io/github/license/marshallasch/saf?style=plastic)
![Lines of code](https://img.shields.io/tokei/lines/github/marshallasch/saf?style=plastic)
![NS3 version](https://img.shields.io/badge/NS--3-3.32-blueviolet?style=plastic)

# SAF Simulation

This modified version of the SAF simulator is designed to allow the SAF storage scheme
to run as a module independently of the simulation runner logic. 

The previous version of this implementation involved putting this repository into the 
`scratch` folder in NS-3, this is no longer the case. 
This module is designed to be placed in the `contrib` folder so it can be used as its own module
and can be run independently of my simulation runner code. 

This NS-3 scratch simulator code that attempts to implement the SAF data storage
scheme and reproduce the performance evaluation as described in:

> T. Hara, :Effective replica allocation in ad hoc networks for improving data
> accessibility" Proceedings IEEE INFOCOM 2001. Apr. 2001


## Motivation

As part of my masters work as part of the Gillis Lab group<sup>[1]</sup> I am
evaluating existing data storage schemes that does not have any available
implementations to evaluate.
A challenge that our lab group discovered is that there although there have been
a number of works published that focus on developing new, better algorithms
very little work has been done to evaluate the ones that exist, largely due to
challenges around replicating the studies or reproducing the algorithms.

The goal of this implementation is to be able to have a baseline to compare other
data storage schemes too (since there are none that are readily available
and Hara's methods have been the defacto standard that others are compared to).

*Note*: Hara implemented their algorithm using a custom built simulator, we have
chosen to implement it using the popular NS-3 network simulator instead.

## Prereqs:
Not sure if there are others but on ubuntu:
- sqlite3, libsqlite3-dev

If building netanim:
- qt4-qmake
- libqt4-dev

(additionally, on ubuntu 20.10, you need to add an additional repo because qt4 no longer ships with it)
- https://launchpad.net/~gezakovacs/+archive/ubuntu/ppa

## Building this project

Reproducibility is one of the key goals of simulator studies.

 1. Download and build copy of the ns-3.32 all-in-one distribution.

    ```sh
    wget https://www.nsnam.org/release/ns-allinone-3.32.tar.bz2
    tar xjvf ns-allinone-3.32.tar.bz2
    cd ns-allinone-3.32
    python3 ./build.py --enable-examples
    ```

 2. Change directories to the `contrib/` folder of the ns-3.32 source
    distribution.

    ```sh
    cd ns-3.32/contrib/
    ```

 3. Clone this repository.

    ```sh
    git clone git@github.com:marshallasch/saf.git saf
    ```

4. Change directory back to the `ns-3.32` folder of the source distribution
   and re-configure `./waf` so it can cache the new changes

   ```sh
   cd ..
   ./waf configure --enable-examples --enable-tests
   ./waf build
   ```

5. Run the example simulation that is included within the module.

   ```sh
   ./waf --run 'saf-example'
   ```

6. Create your own simulations using the SAF module. 
   This is done the same way you would run any other simulation using NS-3. 
   Setup your nodes, add a mobility model, install the SAF application using the
   helper, then run the simulation. 

   One this to note is that the current implementation does not separate the SAF
   validation simulation from the base SAF implementation that can then be used 
   in other applications, that is a work in progress.

## Running the simulation

If you're familiar with ns-3, then you should know that the simulation is run
via the `waf` build tool. Arguments to this program *must* be part of the same
string that is passed to `./waf --run` (that's just how it works :shrug:).

Every parameter of the simulation is configurable. Run the following to see
all the configurable parameters. The default values are as described in the
SAF paper cited at the top of this document.

```sh
./waf --run 'saf-example --printHelp'  # <-- mind the quotes!
```

You can view an animation of the simulation using `NetAnim`, which is included
with the ns-3 all-in-one distribution. To do so, run the following:

```sh
./waf --run 'saf-example --animation-xml=path/to/saf.xml'
```

This will generate an XML file at the specified path. You can then open this
file with `NetAnim` to view what happens during the simulation run.

**NOTE: currently the generation of the animation has been disabled to improve the run time**

## Code style

This project is formatted according to the `.clang-format` file included in this repository. It intentionally deviates from the code style used by the ns-3 library and simulator developers.

The code can be formatted using the included python script created by Guillaume Papin ([@Sarcasm]) and can be found [Sarcasm/run-clang-format].


The following command can be run to automatically reformat the code in place according the included style guideline.

```sh
./run-clang-format.py -r -i --style file .
```

## Special Thanks

I would like to acknowledge the amazing work that was done by Keefer Rourke ([@keeferrourke])
on the initial [RHPMAN] project that I used as a base for this implementation.


## License
While ns-3 is itself licensed under the GNU General Public License v2, the code in this repository is made available under the Internet Systems Consortium (ISC) License.

A copy of this license is included in this repository, and embedded in the top of each source file.

<!-- links -->

[1]: https://danielgillis.wordpress.com/students/
[RHPMAN]: https://github.com/keeferrourke/rhpman-sim
[@keeferrourke]: https://github.com/keeferrourke
[Sarcasm]: https://github.com/Sarcasm
[Sarcasm/run-clang-format]: https://github.com/Sarcasm/run-clang-format
