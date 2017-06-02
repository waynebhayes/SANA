#!/usr/bin/env python3

import argparse
import sys
import os
import subprocess
import glob
import multiprocessing
import re
import math

PARALLEL_EXEC = '/home/wayne/bin/bin.x86_64/parallel'

if (sys.version_info.major == 3 and sys.version_info.minor < 5) or sys.version_info == 2:
    print("Python version must >=3.5. If on UCI network, use the command:",
            file=sys.stderr)
    print("module load python/3.5.1", file=sys.stderr)
    sys.exit(8)

class TemperatureSchedule:
    ERROR_MSG='{shadow}-{network} has failed, check {shadow}-{network}.stderr or {shadow}-{network}.stdout'
    def __init__(self, args:list, shadow_file:str, out_dir:str):
        self.__t = dict()
        self.__tdecay = dict()
        m_dir = os.path.join(out_dir, 'dir-init')
        m_pattern = re.compile(r'(?:Initial Temperature|tdecay):\s([0-9]+(?:\.[0-9]+)?)')
        shadow_name = os.path.splitext(os.path.basename(shadow_file))[0]
        for n in args.networks:
            n_name = os.path.splitext(os.path.basename(network))[0]
            m_file = os.path.join(m_dir,'{}-{}.stderr'.format(shadow_name, n_name))
            with open(m_file, mode='r') as f:
                f_out = f.read()
            matches = m_pattern.findall(f_out)
            if len(matches) != 2:
                print('Something must have gone wrong!',file=sys.stderr)
                raise Exception(self.ERROR_MSG.format(shadow=shadow_file, network=n_name))
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
    out_dir_name = os.path.splitext(os.path.basename(OUT_DIR))[0]
    p_dir = os.path.join(out_dir, 'dir{}'.format(i-1))
    c_dir = os.path.join(out_dir, 'dir{}'.format(i))
    shadow_file = os.path.join(p_dir, '{}.shadow{}.el'.format(out_dir_name, i-1))
    n_name = os.path.splitext(os.path.basename(network))[0]
    output = os.path.join(c_dir, 'shadow-' + n_name)
    align = os.path.join(p_dir, 'shadow-' + n_name + '.out')
    c = ['./sana', '-fg1', network, '-fg2',  shadow_file, '-o', output,
            '-startalignment', align,'-t', str(args.time/args.iterations)] + \
                    args.command_line.split() 
    return c

def get_num_nodes(network) -> int:
    ext = os.path.splitext(network)[1]
    f = open(network, mode='r')
    num_nodes = None
    if ext == '.gw':
        for i in range(4):
            next(f)
        num_nodes = int(f.readline().strip())
    elif ext == '.el':
        nodes = set()
        for line in f:
            nodes.update(line.strip().split()[0:2])
        num_nodes = len(nodes)
    f.close()
    if num_nodes == None:
        raise Exception('Network "{}" is not .gw or .el format'.format(network))
    return num_nodes

if __name__ == '__main__':
    args = init_parser().parse_args()
    code = check_args(args)
    if code != 0:
        sys.exit(code)
    # TODO
    # print(args), stderr or stdout ?
    print(args)

    # Create output directory
    OUT_DIR = args.output_directory
    OUT_DIR_NAME = os.path.splitext(os.path.basename(OUT_DIR))[0]
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

    nodes = {get_num_nodes(network) : network for network in args.networks}
    biggest_network = nodes[max(nodes.keys())]
    largest_num_node = max(nodes.keys())

    # Generate initial shadow network
    c_shadow_args = ['python3','./scripts/create_shadow.py','-n'] + \
            args.networks + ['-s', str(largest_num_node)] + ['-a'] + \
            glob.glob(os.path.join(INIT_OUT_DIR,'*.align'))
    with open(os.path.join(INIT_OUT_DIR, '{}.shadow0.el'.format(OUT_DIR_NAME)),mode='w') as f:
         process = subprocess.run(c_shadow_args, stdout=f, stderr=subprocess.PIPE)
    if process.returncode != 0:
        print('ERROR. ./scripts/create_shadow.py has failed', file=sys.stderr)
        raise Exception(process.stderr)

    # Starting initial temperature schedule
    # --------------------------------------------------
    print('Initial temperature... ', end='', flush=True)
    p_dir = os.path.join(OUT_DIR, 'dir0')
    c_dir = os.path.join(OUT_DIR, 'dir-init')
    os.mkdir(c_dir)
    commands = []
    shadow_file = os.path.join(p_dir, biggest_network)
    for network in args.networks:
        shadow_name = os.path.splitext(os.path.basename(shadow_file))[0]
        n_name = os.path.splitext(os.path.basename(network))[0]
        output = os.path.join(c_dir, '{}-{}'.format(shadow_name, n_name))
        c = ['./sana', '-fg1', network, '-fg2',  shadow_file, '-o', output,
                '-t', '1']
        c += ['2>',output + '.stderr', '1>', output + '.stdout']
        commands.append(' '.join(c))
    str_commands = '\n'.join(commands)
    process = subprocess.run([PARALLEL_EXEC, str(args.processes)], 
                                input=str_commands.encode('ascii'))
    if process.returncode != 0:
        print('Something has gone wrong! Parallel script failed', file=sys.stderr)
        raise Exception(PARALLEL_EXEC + ' has failed.')
    # Reading/parsing initial temperature values
    my_tschedule = TemperatureSchedule(args, biggest_network, OUT_DIR)
    print('done')
    # --------------------------------------------------

    for i in range(1, 1+args.iterations):
        print('Iteration {} starting...  '.format(i), end=' ', flush=True)
        c_dir = os.path.join(OUT_DIR, 'dir{}'.format(i))
        os.mkdir(c_dir)
        commands = []
        for network in args.networks:
            c = create_cmd_line(args, network, i)
            tinit,tdecay = my_tschedule[network]
            tinit_i = tinit* math.exp(-tdecay*i/args.iterations)
            c += ['-tinitial', str(tinit_i), '-tdecay', str(tdecay)]
            n_name = os.path.splitext(os.path.basename(network))[0]
            output = os.path.join(c_dir, 'shadow-' + n_name)
            c += ['2>',output + '.stderr', '1>', output + '.stdout']
            commands.append(' '.join(c))

        str_commands = '\n'.join(commands)
        process = subprocess.run([PARALLEL_EXEC, str(args.processes)], 
                                    input=str_commands.encode('ascii'))
        if process.returncode != 0:
            print('Iteration {} has failed. Please check the error logs'.format(i),
                    file=sys.stderr)
            raise Exception('{} has given an error code'.format(PARALLEL_EXEC))
        print('done')

        print('\tCreating shadow{} file... '.format(i), end='', flush=True)
        c_shadow_args = ['python3','./scripts/create_shadow.py','-c','-n'] + \
                args.networks + ['-s', str(largest_num_node)] + ['-a'] + \
                glob.glob(os.path.join(c_dir,'*.out'))
        with open(os.path.join(c_dir,'{}.shadow{}.el'.format(OUT_DIR_NAME,i)), 
            mode='w') as f:
            process = subprocess.run(c_shadow_args, stdout=f, stderr=subprocess.PIPE)
        if process.returncode != 0:
            print('The following error has occurred in ./scripts/create_shadow.py',
                    file=sys.stderr)
            print(process.stderr, file=sys.stderr)
            sys.exit(1)
        print('done')
