# EnerPerf
wraps profilers and collects data from energy use monitors to profile the energy use of code to high resolution

# TODO

- [ ] standard energy data format that allows for arbitrary mesurments
- [ ] standard time format that all profilers included need to translate their output for
- [ ] library structure that takes a monitor, a profiler, and a front end. runs the experiment and outputs a constructed energy profile 
	- [ ] to the granularity of the profiler 
	- [ ] with the time resolution of the monitor
- [ ] 4 monitors: 
    - RAPL, 
    - an RAPL kernel moduel for low overhead, 
    - a hardware power monitor, 
    - and a regression model
- [ ] 3 profilers and a stub for processes: 
    - cprofile, 
    - scaline, 
    - node.js built in porfiler, 
    - a stub one to run a whole process should be trivial 
- [ ] 3 front ends: 
    - cli, 
    - webui, 
    - vscode extension
- [ ] graph tool for visualization

# primitive version
currently just a referance RAPL sensor in c