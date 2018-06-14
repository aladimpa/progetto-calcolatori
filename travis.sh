#!/usr/bin/bash
sudo apt-get update -qq
sudo apt-get install valgrind
make all
valgrind --error-exitcode=1 --leak-check=full ./simulator -i input_files/01_tasks.csv -on output-1-np.log -op output-1-pr.log
valgrind --error-exitcode=1 --leak-check=full ./simulator -i input_files/02_tasks.csv -on output-2-np.log -op output-2-pr.log
valgrind --error-exitcode=1 --leak-check=full ./simulator -i input_files/03_tasks.csv -on output-3-np.log -op output-3-pr.log
valgrind --error-exitcode=1 --leak-check=full ./simulator -i input_files/04_tasks.csv -on output-4-np.log -op output-4-pr.log
valgrind --error-exitcode=1 --leak-check=full ./simulator -i input_files/05_tasks.csv -on output-5-np.log -op output-5-pr.log