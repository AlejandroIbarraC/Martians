# Martians 👽

**Martians** is an interactive application that allows you to simulate a process scheduling system and try different workloads for different OS types, following simple scheduler theory. It features many operation modes, outlined below:

* **Sim Type:** In Manual mode, the user can create a martian at any given moment once the simulation is running, providing energy (process execution time) and period (for RTOS this means regeneration time and for interactive OS, arrival time) for each martian. In Auto mode, once the simulation starts, you can't add any more martians, which means the workload must be set in advance.
* **OS Type:** Interactive (common OS type) or RTOS (Real Time Operating System). For the RTOS analogy, martians are processses, their energy is execution time and their regeneration time symbolizes each process' period.
* **Scheduling:** For the Interactive OS, the FCFS, Priority and SRTN algorithms are included. For RTOS, you can choose between EDF and RM.
* **Priority:** If OS Type is Interactive and Priority scheduling is selected, this radio box allows you to choose between three different priority levels for each martian (process analogy). Each level is represented with a different color, where green is High priority, blue is Medium and red is Low.

The **Add Martian** button does exactly what it says: it adds a new martian to the current workload. Beware that the possibility to add new martians depends on Sim Type. Finally, the **Start Button** starts the simulation.

It's also possible to check current simulation time measured in seconds with the label below the Start button, as well as a graphical representation of each martian's energy updated in real time that reflects the system's general status. When the simulation ends or the application is closed, a report card is generated, which shows a timeline summary of how processes were scheduled according to simulated processor cycles.

The main application was written in C and the report card is generated using Python plotting libraries. A Makefile is included to simplify compilation endeavours. Both were developed in Ubuntu 20.04.

## Photos 📷

Main UI        |                Process end card report
:------------------------------:|:------------------------------:|
![](readme-images/m1.png)  | ![](readme-images/m2.png)

### Prerequisites 👓

Software you need to install to run this project:

```
C version 23
C libraries: SDL2, SDL2 Image, pthread, gtk+ 3.0
Python 3.8 or higher (recommended)
Python libraries: numpy, matplotlib, pandas
```

### Compiling 💻

Run the included **Makefile** with the default *make* command. Make sure to install all dependencies, especially SDL2 and SDL2 image, which can sometimes be tricky to configure. This creates an executable file called *main* in the src folder that can be opened with *./main*

## Authors 👨🏻‍💻

* **Brayan Alfaro** - *Report Developer* - [Braalfa](https://github.com/Braalfa)
* **Brayan Leon** - *Logic Developer* - [brayan156](https://github.com/brayan156)
* **Alejandro Ibarra** - *Developer & Designer* - [AlejandroIbarraC](https://github.com/AlejandroIbarraC)
* **Jesus Yamir Sandoval** - *Developer* - [shuzz22260427](https://github.com/shuzz22260427)

## License 📄

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments 📎

* Costa Rica Institute of Technology
* Kevin Cordero - [kevinscorzu](https://github.com/kevinscorzu)

<p align="center">This project was made with academical purposes. 2022</p
```

