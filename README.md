# ATCN-Programs

The ATCN-Program is written to simulate the Congestion Control Process in TCP in NS-3. 

## Structure of the Whole project

```bash
.
├── README.md
├── experiment
│   ├── 01_result
│   │   ├── ATCN-Program-Tcprate.txt
│   │   └── ATCN-Program-Udprate.txt
│   ├── 02_result
│   │   ├── ATCN-Program-routerBufferSize.txt
│   │   └── ATCN-Program-tcpBufferSize.txt
│   ├── 03_result
│   │   └── ATCN-Program-MTU.txt
│   ├── 04_result
│   │   └── NewReno
│   │       ├── ATCN-Program-ascii
│   │       ├── ATCN-Program-cwnd.txt
│   │       ├── ATCN-Program-inflight.txt
│   │       ├── ATCN-Program-next-rx.txt
│   │       ├── ATCN-Program-next-tx.txt
│   │       ├── ATCN-Program-rto.txt
│   │       ├── ATCN-Program-rtt.txt
│   │       └── ATCN-Program-ssth.txt
│   └── ATCN_experiment_report.pptx
├── sample
│   └── atcn-case-study.cc
└── src
    └── ATCN-Program.cc
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

It is weird that the source program can only run normally in the `scratch` directory, which may be the internal mechanism of NS-3.

```bash
cd /home/hostname/ns-3-dev
git clone https://github.com/ForgottenField/NS3-TCP-Congestion-Simulation.git
cp NS3-TCP-Congestion-Simulation/src/ATCN-Program.cc scratch/
./waf --run scratch/ATCN-Program.cc
```

**Fail to build NS-3 environment**
---

If the command `./waf build` can not be executed successfully, please use the following command to check whether there is sufficient memory allocated for wsl.   

```bash
free -g
```

This command will show your memory size in gibibytes, and you can change the `-g` option to output in different forms.

After that, you can refer to [this answer](https://learn.microsoft.com/en-us/answers/questions/1296124/how-to-increase-memory-and-cpu-limits-for-wsl2-win) to increase your virtual memory in wsl.
