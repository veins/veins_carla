VEINS_CARLA
===============

Veins - The open source vehicular network simulation framework.

See the Veins website <[http://veins.car2x.org/](http://veins.car2x.org/)> for a tutorial, documentation,
and publications.

Veins is composed of many parts. See the version control log for a full list of
contributors and modifications. Each part is protected by its own, individual
copyright(s), but can be redistributed and/or modified under an open source
license. License terms are available at the top of each file. Parts that do not
explicitly include license text shall be assumed to be governed by the "GNU
General Public License" as published by the Free Software Foundation -- either
version 2 of the License, or (at your option) any later version
(SPDX-License-Identifier: GPL-2.0-or-later). Parts that are not source code and
do not include license text shall be assumed to allow the Creative Commons
"Attribution-ShareAlike 4.0 International License" as an additional option
(SPDX-License-Identifier: GPL-2.0-or-later OR CC-BY-SA-4.0). Full license texts
are available with the source distribution.

# Paper
Please cite our VNC 2023 paper when using *veins_carla*:

![](https://www.cms-labs.org/assets/bib2web/icons/IEEE-logo.gif "IEEE") [Tobias Hardes](https://www.cms-labs.org/people/hardes/), [Ion Turcanu](https://ion-turcanu.net/) and [Christoph Sommer](https://www.cms-labs.org/people/sommer/), "**Poster: A Case for Heterogenous Co-Simulation of Cooperative and Autonomous Driving**," Proceedings of 14th IEEE Vehicular Networking Conference (VNC 2023), Istanbul, Türkiye, April 2023.


```
@inproceedings{hardes2023cosimulation,
  author = {Hardes, Tobias and Turcanu, Ion and Sommer, Christoph},
  title = {{Poster: A Case for Heterogenous Co-Simulation of Cooperative and Autonomous Driving}},
  booktitle = {14th IEEE Vehicular Networking Conference (VNC 2023)},
  address = {Istanbul, Türkiye},
  month = {April},
  publisher = {IEEE},
  year = {2023},
}

```

# Prerequisites

*veins_carla* has so far only been tested using Linux (Ubuntu 18.04 - recommendation by CARLA).

Download and follow the instructions to install CARLA: [CARLA GitHub ](https://github.com/carla-simulator/)

Download and follow the instructions for installing [OMNeT++ 6 ](https://omnetpp.org/download/)

*veins_carla* requires several dependencies to run.
To install the Python3 requirements, run the following commands:

```
python3 -m pip install --user grpcio
python3 -m pip install --user grpcio-tools
python3 -m pip install --user conan==1.54.0
```

Then you need to configure ``conan``. You will need the following command:
```

conan profile new default --detect && conan profile update settings.compiler.libcxx=libstdc++11 default

```
Building veins_carla
--------------
Use `git clone` or download the project from this page. Note that the `main` branch contains the latest release of *veins_carla* with the latest fixes and features. *veins_carla* also requires [Veins](https://www.github.com/sommer/veins) to be downloaded and compiled.
You may use the following commands:
```
git clone git@github.com:sommer/veins.git
cd veins
./configure
make -j$(nproc)

cd ..
git clone git@github.com:veins/veins_carla.git
cd veins_carla
./configure
make -j$(nproc)
```


Run veins_carla
--------------
You need to start CARLA first. Depending on your local installation this can be done with the following command
```
cd carla_9.13
./CarlaUE4.sh
```
The simulation can be started either using the IDE (following the instructions in the [Veins documentation](http://veins.car2x.org/tutorial/)) or simply by using the following commands:
```
cd veins_carla/examples/veins_carla
./doRun.sh
```
