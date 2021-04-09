![GitHub Workflow Status (branch)](https://img.shields.io/github/workflow/status/marshallasch/saf/Clang-format/main)
![GitHub](https://img.shields.io/github/license/marshallasch/saf?style=plastic)
![Lines of code](https://img.shields.io/tokei/lines/github/marshallasch/saf?style=plastic)
![NS3 version](https://img.shields.io/badge/NS--3-3.32-blueviolet?style=plastic)

# SAF Simulation

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


## Building this project

Reproducibility is one of the key goals of simulator studies.

 1. Download and build copy of the ns-3.32 all-in-one distribution.

    ```sh
    wget https://www.nsnam.org/release/ns-allinone-3.32.tar.bz2
    tar xjvf ns-allinone-3.32.tar.bz2
    cd ns-allinone-3.32
    python3 ./build.py --enable-examples
    ```

 2. Change directories to the `scratch/` folder of the ns-3.32 source
    distribution.

    ```sh
    cd ns-3.32/scratch/
    ```

 3. Clone this repository.

    ```sh
    git clone git@github.com:marshallasch/saf.git saf
    ```

4. Change directory back to the `ns-3.32` folder of the source distribution
   and run this simulation through the `waf` tool. This will compile the
   simulation code and start executing the code.

   ```sh
   cd ..
   ./waf --run 'scratch/saf/saf`
   ```

## Running the simulation

If you're familiar with ns-3, then you should know that the simulation is run
via the `waf` build tool. Arguments to this program *must* be part of the same
string that is passed to `./waf --run` (that's just how it works :shrug:).

Every parameter of the simulation is configurable. Run the following to see
all the configurable parameters. The default values are as described in the
SAF paper cited at the top of this document.

```
./waf --run 'scratch/saf/saf --printHelp'  # <-- mind the quotes!
```

You can view an animation of the simulation using `NetAnim`, which is included
with the ns-3 all-in-one distribution. To do so, run the following:

```
./waf --run 'scratch/saf/saf --animation-xml=path/to/saf.xml
```

This will generate an XML file at the specified path. You can then open this
file with `NetAnim` to view what happens during the simulation run.


## Code style

This project is formatted according to the `.clang-format` file included in this repository. It intentionally deviates from the code style used by the ns-3 library and simulator developers.

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
