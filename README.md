Project Overview: Chain Reaction Simulation (Operating Systems 2023/24)

The project involves simulating a chain reaction using the following processes:

Master Process: Manages the simulation, initializes structures, creates child processes, updates statistics, and draws energy periodically. At the start, it creates atom, activator, and feeder processes and begins the simulation after all are initialized.

Atom Process: Each atom has a random atomic number and simulates fission through fork. Fission produces new atoms and releases energy calculated by a specific formula. Atoms with a small atomic number can terminate, becoming waste.

Activator Process: Periodically triggers fission in existing atoms but does not create new atoms.

Feeder Process: Periodically creates new atom processes as fuel.

Simulation Termination Conditions:

Timeout: Simulation duration reached.
Explosion: Energy exceeds a threshold.
Blackout: Energy demand exceeds supply.
Meltdown: Fork failures.
The system configuration is read dynamically (from files or environment variables). Outputs include statistics on activations, fissions, energy, and waste production. The implementation must use shared memory, semaphores, and inter-process communication, with modular design, concurrency, and efficient resource management.
