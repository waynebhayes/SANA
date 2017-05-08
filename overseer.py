#!/usr/bin/env python3

import argparse
import sys
import os
import subprocess
import glob
import multiprocessing
import re
import math

if (sys.version_info.major == 3 and sys.version_info.minor < 5) or sys.version_info == 2:
    print("Python version must >=3.5. If on UCI network, use the command:",
            file=sys.stderr)
    print("module load python/3.5.1", file=sys.stderr)
    sys.exit(8)

class TemperatureSchedule:
    def __init__(self, args:list, processes:list, OUT_DIR:str):
        m_pattern = re.compile(r'(?:Initial Temperature|tdecay):\s([0-9]+\.[0-9]+)')
        self.__t = dict()
        self.__tdecay = dict()
        i = 0
        for n,p in zip(args.networks,processes):
            name = os.path.splitext(os.path.basename(n))[0]
            output, process = p
            with open(output+'.stdout',mode='w') as f:
                f.write(process.stdout.decode('utf-8'))
            with open(output+'.stderr',mode='w') as f:
                f.write(process.stderr.decode('utf-8'))
            if process.returncode != 0:
                print('Exception has occurred running shadow-{}'.format(name), file=sys.stderr)
                print(create_cmd_line(args,n,1), file=sys.stderr)
                raise Exception
            matches = m_pattern.findall(process.stderr.decode('utf-8'))
            if len(matches) != 2:
                print('Something must have gone wrong!',file=sys.stderr)
                print('Check init dir for shadow-{}'.format(name), file=sys.stderr)
                raise Exception
            self.__tdecay[n] = float(matches[0])
            self.__t[n] = float(matches[1])

    def __getitem__(self, key):
        return (self.__t[key],self.__tdecay[key])
    
def init_parser():
    parser = argparse.ArgumentParser(description='Run multi-pairwise SANA')
    parser.add_argument('-n', '--networks', nargs='+', required=True, 
                help='List of network files. Must be .gw or .el format')
    parser.add_argument('-t', '--time', default=60, type=float, 
                help='Total run time in minutes. Default is 60 minutes')
    parser.add_argument('-i', '--iterations', default=10, type=int, 
                help='Number of iterations. Default is 10')
    parser.add_argument('-c', '--command-line', type=str, 
                help='Additional arguments to pass to SANA. Must be enclosed with single or double quotes')
    parser.add_argument('-s', '--shadow-nodes', type=int, required=True, 
                help='Number of nodes in the largest network. Must be an integer')
    parser.add_argument('-o', '--output-directory', type=str, required=True, 
                help='Output directory. Must not exist')
    parser.add_argument('-p', '--processes', type=int, default=8, 
                help='Number of concurrent processes running. Default is 8')
    return parser

def check_args(args):
    code = 0
    if args.time <= 0:
        print('--time must be greater than 0', file=sys.stderr)
        code += 1
    if args.iterations <= 0:
        print('--iterations must be greater than 0', file=sys.stderr)
        code += 2
    if os.path.exists(args.output_directory):
        print('Directory "{}" exists. IMMINENT LOSS OF DATA'.format(
            args.output_directory), file=sys.stderr)
        code += 4
    return code

def create_cmd_line(args, network:str, i:int):
    out_dir = args.output_directory
    p_dir = os.path.join(out_dir, 'dir{}'.format(i-1))
    c_dir = os.path.join(out_dir, 'dir{}'.format(i))
    shadow_file = os.path.join(p_dir, 'shadow{}.gw'.format(i-1))
    commands = [] 
    n_name = os.path.splitext(os.path.basename(network))[0]
    output = os.path.join(c_dir, 'shadow-' + n_name)
    align = os.path.join(p_dir, 'shadow-' + n_name + '.out')
    c = ['./sana', '-fg1', network, '-fg2',  shadow_file, '-o', output,
            '-startalignment', align,'-t', str(args.time/args.iterations)] + \
                    args.command_line.split() 
    return (output,c)

def run_process(args, network:str, n_index:int, i:int, produce:list,
        semaphore, t_schedule=None):
    output, command = create_cmd_line(args, network, i)
    if t_schedule:
        tinit,tdecay = t_schedule[network]
        tinit_i = tinit* math.exp(tdecay*i/args.iterations)
        command += ['-tinitial', str(tinit_i), '-tdecay', str(tdecay)]
    semaphore.acquire()
    print(' '.join(command))
    produce[n_index] = (output,subprocess.run(command, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE))
    semaphore.release()
    return None

def iteration(args, i:int,t_schedule=None):
    processes = []
    manager = multiprocessing.Manager()
    m_list = manager.list(range(len(args.networks)))
    m_semaphore = manager.BoundedSemaphore(args.processes)
    for index,n in enumerate(args.networks):
        p = multiprocessing.Process(target=run_process, 
                args=(args, n, index, i, m_list, m_semaphore, t_schedule))
        processes.append(p)
        p.start()
    for p in processes:
        p.join()
    return m_list

if __name__ == '__main__':
    args = init_parser().parse_args()
    code = check_args(args)
    if code != 0:
        sys.exit(code)
    # TODO
    # print(args), stderr or stdout ?
    print(args)

    OUT_DIR = args.output_directory
    os.mkdir(OUT_DIR)
    INIT_OUT_DIR = os.path.join(OUT_DIR,'dir0')
    os.mkdir(INIT_OUT_DIR)

    # Generate initial random alignments in OUT_DIR/dir0/*.align
    r_process = subprocess.run(['./random-multi-alignment.sh',INIT_OUT_DIR] 
            + args.networks)
    if r_process.returncode != 0:
        print('The following error has occurred in ./random-multi-alignment:',
                file=sys.stderr)
        print(process.stderr, file=sys.stderr)
        sys.exit(1)

    # Generate initial shadow network
    c_shadow_args = ['python3','./scripts/create_shadow.py','-n'] + \
            args.networks + ['-s', str(args.shadow_nodes)] + ['-a'] + \
            glob.glob(os.path.join(INIT_OUT_DIR,'*.align'))
    with open(os.path.join(INIT_OUT_DIR,'shadow0.gw'),mode='w') as f:
         process = subprocess.run(c_shadow_args, stdout=f)

    # INIT TEMP
    os.mkdir(os.path.join(OUT_DIR,'dir1'))
    a = iteration(args,1)
    try:
        my_tschedule = TemperatureSchedule(args, a, OUT_DIR)
    except:
        raise
    os.rename(os.path.join(OUT_DIR,'dir1'), os.path.join(OUT_DIR,'dir-init'))

    for i in range(1, 1+args.iterations):
        print('Iteration {} starting...  '.format(i), end=' ')
        C_DIR = os.path.join(OUT_DIR, 'dir{}'.format(i))
        os.mkdir(C_DIR)
        a = iteration(args, i, my_tschedule)
        for j,item in enumerate(a):
            output, process = item
            with open(output+'.stdout',mode='w') as f:
                f.write(process.stdout.decode('utf-8'))
            with open(output+'.stderr',mode='w') as f:
                f.write(process.stderr.decode('utf-8'))
            if process.returncode != 0:
                network = args.networks[j]
                fail_name = os.path.splitext(os.path.basename(network))[0]
                print('shadow-{} has failed on iteration{}'.format(fail_name, i),
                        file=sys.stderr)
                sys.exit(1)
        print('done')

        print('Creating shadow{} file'.format(i))
        c_shadow_args = ['python3','./scripts/create_shadow.py','-c','-n'] + \
                args.networks + ['-s', str(args.shadow_nodes)] + ['-a'] + \
                glob.glob(os.path.join(C_DIR,'*.out'))
        with open(os.path.join(C_DIR,'shadow{}.gw'.format(i)), 
            mode='w') as f:
            process = subprocess.run(c_shadow_args, stdout=f, stderr=subprocess.PIPE)
        if process.returncode != 0:
            print('The following error has occurred in ./scripts/create_shadow.py',
                    file=sys.stderr)
            print(process.stderr, file=sys.stderr)
            sys.exit(1)
