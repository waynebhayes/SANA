#!/usr/bin/env python3

import argparse
import sys
import os
import pathlib
import subprocess
import glob

if (sys.version_info.major == 3 and sys.version_info.minor < 5) or sys.version_info == 2:
    print("Python version must >=3.5. If you are on UCI network, use the command:",
            file=sys.stderr)
    print("module load python/3.5.1", file=sys.stderr)
    sys.exit(2)


class TemperatureSchedule:
    def __init__(self, networks:list, shadow:str, **kwargs):
        self.__t = dict()
        for g in networks:
            pass
            #self.__t[g],self.__tdecay[g] = [python exec]
                    #(sana -g1 g -g2 shadow... hudson stuff)

    def __getitem__(self, key):
        return (self.__t[key],self.__tdecay[key])

def init_parser():
    parser = argparse.ArgumentParser(description='Run multi-pairwise SANA')
    parser.add_argument('-n', '--networks', nargs='+', required=True)
    parser.add_argument('-t', '--time', default=60, type=float)
    parser.add_argument('-i', '--iterations', default=1, type=int)
    parser.add_argument('-c', '--command-line', type=str)
    parser.add_argument('-s', '--shadow-nodes', type=int, required=True)
    parser.add_argument('-o', '--output-directory', type=str, required=True)
    # TODO - Actually use this processes arg
    parser.add_argument('-p', '--processes', type=int, default=8)
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
    
def create_cmd_line(args, OUT_DIR, P_DIR, C_DIR, SHADOW_FILE):
    commands = [] 
    for network in args.networks:
        n_name = os.path.splitext(os.path.basename(network))[0]
        output = os.path.join(OUT_DIR, C_DIR, 'shadow-' + n_name)
        align = os.path.join(OUT_DIR, P_DIR, 'shadow-' + n_name + '.out')
        c = './sana -fg1 {} -fg2 {} {} -o {} -startalignment {} -t {}'.format(
            network, SHADOW_FILE, args.command_line, output, align, 
            args.time/args.iterations)
        commands.append(c)
    return '\n'.join(commands)
if __name__ == '__main__':
    args = init_parser().parse_args()
    code = check_args(args)
    # TODO
    # print(args), stderr or stdout ?
    if code != 0:
        sys.exit(code)
    print(args)

    OUT_DIR = args.output_directory
    os.mkdir(OUT_DIR)
    INIT_OUT_DIR = os.path.join(OUT_DIR,'dir0')
    os.mkdir(INIT_OUT_DIR)

    # Generate initial random alignments in TEMP_DIR/dir0/*.align
    process = subprocess.run(['./random-multi-alignment.sh',INIT_OUT_DIR] 
            + args.networks)
    if process.returncode != 0:
        print('The following error has occurred in ./random-multi-alignment:',
                file=sys.stderr)
        print(process.stderr, file=sys.stderr)

    # Generate initial shadow network
    c_shadow_args = ['python3','./scripts/create_shadow.py','-n'] + \
            args.networks + ['-s', str(args.shadow_nodes)] + ['-a'] + \
            glob.glob(os.path.join(INIT_OUT_DIR,'*.align'))
    with open(os.path.join(INIT_OUT_DIR,'shadow0.gw'),mode='w') as f:
         process = subprocess.run(c_shadow_args, stdout=f)

    # my_tschedule = TemperatureSchedule(self, args.networks, [shadow])

    for i in range(1, 1+args.iterations):
        print('Iteration {}'.format(i))
        P_DIR = 'dir{}'.format(i-1)
        C_DIR = 'dir{}'.format(i)
        os.mkdir(os.path.join(OUT_DIR,C_DIR))
        SHADOW_FILE = os.path.join(OUT_DIR, P_DIR,
                'shadow{iteration}.gw'.format(iteration=(i-1)))
        commands = create_cmd_line(args, OUT_DIR, P_DIR, C_DIR, SHADOW_FILE)
        with open(os.path.join(OUT_DIR,C_DIR,'run.output'),mode='w') as f:
            subprocess.run(['/home/wayne/bin/bin.x86_64/parallel', '8'],
                    input=commands.encode('ascii'), stderr=f)
            # t_initial,t_decay = my_tschedule[g]
            # starting = t_initial * e^(-t_decay * i/K)
        c_shadow_args = ['python3','./scripts/create_shadow.py','-c','-n'] + \
                args.networks + ['-s', str(args.shadow_nodes)] + ['-a'] + \
                glob.glob(os.path.join(OUT_DIR,C_DIR,'*.out'))
        with open(os.path.join(OUT_DIR,C_DIR,'shadow{}.gw'.format(i)), 
            mode='w') as f:
            process = subprocess.run(c_shadow_args, stdout=f)
        if process.returncode != 0:
            print('The following error has occurred in ./scripts/create_shadow.py',
                    file=sys.stderr)
            print(process.stderr, file=sys.stderr)
            sys.exit(1)
