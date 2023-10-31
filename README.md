# ATCN-Programs

The ATCN-Program is written to simulate the Congestion Control Process in TCP in NS-3. 

## Structure of the Whole project

```bash
.
├── README.md
├── sample
│   └── atcn-case-study.cc # sample code covered in class.
└── src
    └── ATCN-Program.cc # source code used to simulate TCP congestion control process.
```

## Overview of NS-3 Simulation Environment Setup through WSL2

* Download [Visual Studio Code](https://code.visualstudio.com/) and [WSL2](https://code.visualstudio.com/docs/remote/wsl).
* [Prerequisites](https://www.nsnam.org/wiki/Installation#Linux) Configuration.
* NS-3 installation package Download.
* Configure and build NS-3 Environment.
* Run a quick simulator.

**Prerequisites Configuration**
---

1. Switch to the home directory of your Ubuntu ISO file.
   + `$ cd /home/hostname`

2. Fetch the latest version of the package list from your distro's software repository.
   + `$ sudo apt update`
  
3. Download the prerequisites to start NS-3.
   + `$ sudo apt install gcc g++ python3 python tcpdump`


**NS-3 installation package Download**
---

1. Download the package from gitlab.

    + `$ git clone https://gitlab.com/nsnam/ns-3-dev.git`

2. Switch to the `ns-3-dev` directory.

    + `$ cd ns-3-dev/`

3. Switch to the `ns-3.35` branch.(The newer branch uses Cmake to build source codes instead of Waf)

    + `$ git checkout -b ns-3.35-branch ns-3.35`

**Configure and build NS-3 Environment**
---

Use `./waf` instruction to configure and build `NS-3` environment. Make sure to run these commands under the root of `ns-3-dev`.

```bash
./waf configure
./waf build
```

**Run a quick simulator**
---

Run the demo simulator `scratch-simulator.cc` file under the path of `scratch/`.

```bash
./waf --run scratch/scratch-simulator.cc
```

**How to run the ATCN-Program**
---

It is very strange that the source program can only run normally in the `scratch` directory. It may be the internal mechanism of NS-3.

```bash
cd /home/hostname/ns-3-dev
git clone https://github.com/ForgottenField/NS3-TCP-Congestion-Simulation.git
mv NS3-TCP-Congestion-Simulation/src/ATCN-Program.cc scratch/
./waf --run scratch/ATCN-Program.cc
```






