#!/usr/bin/python3

# Tests for project 2 from IOS course
# Author: Ondrej Mach
# Date: 17.04.2021

import subprocess
import sys
from enum import Enum
from time import perf_counter
from typing import Union
import argparse

class Arguments:
    def __init__(self, NE=5, NR=5, TE=100, TR=100):
        self.NE = NE
        self.NR = NR
        self.TE = TE
        self.TR = TR

class Santa:
    class State(Enum):
        NOT_STARTED = 0
        SLEEPING = 1
        HELPING_ELVES = 2
        HITCHING_RDS = 3
        GONE = 4

    def __init__(self):
        self.state = self.State.NOT_STARTED

class Elf:
    class State(Enum):
        NOT_STARTED = 0
        WORKING_ALONE = 1
        AWAITING_HELP = 2
        ON_VACATION = 3

    def __init__(self, ID=0):
        self.state = self.State.NOT_STARTED
        self.ID = ID

class Reindeer:
    class State(Enum):
        NOT_STARTED = 0
        ON_VACATION = 1
        BACK_HOME = 2
        HITCHED = 3

    def __init__(self, ID=0):
        self.state = self.State.NOT_STARTED
        self.ID = ID

class LineCounter:
    def __init__(self):
        self.expectedNumber = 1
        pass

    def read(self, num):
        if int(num) != self.expectedNumber:
            print(f'Expected line number {self.expectedNumber}')
            raise

        self.expectedNumber += 1


class Environment:
    def __init__(self, args, strict=True):
        self.lineCounter = LineCounter()
        self.santa = Santa()
        self.elves = [Elf(ID=i+1) for i in range(args.NE)]
        self.rds = [Reindeer(ID=i+1) for i in range(args.NR)]

        # shared variables for checking
        self.args = args
        self.strict = strict
        self.numElvesToHelp = 0
        self.reindeersHome = 0
        self.workshopOpen = True


    def end(self):
        # After all lines are read
        try:
            self.santaEnd(self.santa)
        except Exception as e:
            raise e

        for elf in self.elves:
            try:
                self.elfEnd(elf)
            except Exception as e:
                print(f'Elf {elf.ID} ended in wrong state')
                raise e

        for rd in self.rds:
            try:
                self.rdEnd(rd)
            except Exception as e:
                print(f'Reindeer {rd.ID} ended in wrong state')
                raise e

    def readLine(self, line):
        lineList = line.split(':')
        lineNum, actor, action = [item.strip() for item in lineList]

        self.lineCounter.read(lineNum)

        if actor == 'Santa':
            self.santaRead(self.santa, action)

        elif actor.startswith('Elf'):
            ID = int(actor.split()[1])
            self.elfRead(self.elves[ID-1], action)

        elif actor.startswith('RD'):
            ID = int(actor.split()[1])
            self.rdRead(self.rds[ID-1], action)

        else:
            print(f'Wrong actor identifier: {actor}')
            raise

    def santaEnd(self, santa):
        if santa.state != Santa.State.GONE:
            print(f'Santa ended in state {santa.state}')
            raise

    def santaRead(self, santa, text):
        if santa.state == Santa.State.NOT_STARTED:
            if text == 'going to sleep':
                if self.numElvesToHelp != 0:
                    print(f'There are still {self.numElvesToHelp} elves in workshop, that didn\'t get help')
                    raise
                santa.state = Santa.State.SLEEPING
            else:
                print(f'Santa in state {santa.state} cannot do {text}')
                raise

        elif santa.state == Santa.State.SLEEPING:
            if text == 'helping elves':
                if self.numElvesToHelp != 0:
                    print('Santa did not yet help all the elves in previous helping cycle (or helped more than 3)')
                    raise
                if self.strict and self.reindeersHome == self.args.NR:
                    print('Santa cannot help elves, when all reindeers are home')
                    raise
                self.numElvesToHelp = 3
                santa.state = Santa.State.HELPING_ELVES
            elif text == 'closing workshop':
                if self.reindeersHome != self.args.NR:
                    print('Santa is closing workshop before all reindeers are home')
                    raise
                self.workshopOpen = False
                santa.state = Santa.State.HITCHING_RDS
            else:
                print(f'Santa in state {santa.state} cannot do {text}')
                raise

        elif santa.state == Santa.State.HELPING_ELVES:
            if text == 'going to sleep':
                if self.numElvesToHelp != 0:
                    print(f'Santa went to sleep, he still has {self.numElvesToHelp} elves in his workshop')
                    raise
                santa.state = Santa.State.SLEEPING
            else:
                print(f'Santa in state {santa.state} cannot do {text}')
                raise

        elif santa.state == Santa.State.HITCHING_RDS:
            if text == 'Christmas started':
                santa.state = Santa.State.GONE
            else:
                print(f'Santa in state {santa.state} cannot do {text}')
                raise

        elif santa.state == Santa.State.GONE:
            print(f'Santa in state {santa.state} cannot do {text}')
            raise


    def elfEnd(self, elf):
        if elf.state != Elf.State.ON_VACATION:
            print(f'Elf {elf.ID} ended in state {elf.state}')
            raise

    def elfRead(self, elf, text):
        if elf.state == Elf.State.NOT_STARTED:
            if text == 'started':
                elf.state = Elf.State.WORKING_ALONE
            else:
                print(f'Elf in state {elf.state} cannot do {text}')
                raise

        elif elf.state == Elf.State.WORKING_ALONE:
            if text == 'need help':
                elf.state = Elf.State.AWAITING_HELP
            else:
                print(f'Elf in state {elf.state} cannot do {text}')
                raise

        elif elf.state == Elf.State.AWAITING_HELP:
            if text == 'get help':
                if not self.workshopOpen:
                    print('Elf cannot get help after the workshop is closed')
                    raise
                if self.santa.state != Santa.State.HELPING_ELVES:
                    print(f'Santa cannot help an elf in state {self.santa.state}')
                    raise

                self.numElvesToHelp -= 1
                elf.state = Elf.State.WORKING_ALONE
            elif text == 'taking holidays':
                if self.workshopOpen:
                    print('Elf cannot go on vacation before the workshop closes')
                    raise
                elf.state = Elf.State.ON_VACATION
            else:
                print(f'Elf in state {elf.state} cannot do {text}')
                raise

        elif elf.state == Elf.State.ON_VACATION:
            print(f'Elf in state {elf.state} cannot do {text}')
            raise



    def rdEnd(self, rd):
        if rd.state != Reindeer.State.HITCHED:
            print(f'Reindeer {rd.ID} ended in state {rd.state}')
            raise

    def rdRead(self, rd, text):
        if rd.state == Reindeer.State.NOT_STARTED:
            if text == 'rstarted':
                rd.state = Reindeer.State.ON_VACATION
            else:
                print(f'Reindeer in state {rd.state} cannot do {text}')
                raise

        elif rd.state == Reindeer.State.ON_VACATION:
            if text == 'return home':
                self.reindeersHome += 1
                rd.state = Reindeer.State.BACK_HOME
            else:
                print(f'Reindeer in state {rd.state} cannot do {text}')
                raise

        elif rd.state == Reindeer.State.BACK_HOME:
            if text == 'get hitched':
                if self.workshopOpen:
                    print('Workshop must be closed, when a reindeer gets hitched')
                    raise
                if self.santa.state != Santa.State.HITCHING_RDS:
                    print(f'Santa cannot hitch a reindeer in state {self.santa.state}')
                    raise
                rd.state = Reindeer.State.HITCHED
            else:
                print(f'Reindeer in state {rd.state} cannot do {text}')
                raise

        elif rd.state == Reindeer.State.HITCHED:
            print(f'Reindeer in state {rd.state} cannot do {text}')
            raise


# things used for formatting
class fmt:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[0;33m'
    DIM = '\e[2m'
    NOCOLOR = '\033[0m'
    TICK = '\u2713'
    CROSS = '\u2717'


def runSubject(args, timeout:Union[None, float]=5):
    process = subprocess.Popen(["./proj2", str(args.NE), str(args.NR), str(args.TE), str(args.TR)])

    try:
        process.wait(timeout)

    except subprocess.TimeoutExpired:
        print(f'The program took longer than {timeout} seconds and has been terminated')
        process.terminate()
        raise

    except KeyboardInterrupt as e:
        print(f'The testing has been cancelled by the user')
        process.terminate()
        raise e

    if process.returncode != 0:
        print('The tested program returned error')
        raise


def analyzeFile(file, args, strict=True):
    # initialize the test environment with all the creatures
    env = Environment(args, strict)

    for lineNumber, line in enumerate(file.readlines()):
        try:
            env.readLine(line)

        except Exception as e:
            print(f'Illegal operation on line {lineNumber}:')
            print(line)
            raise e

    env.end()


class Controller:
    def __init__(self, testedArguments=None, timeToRun=30):
        if testedArguments is None:
            testedArguments = [Arguments(20, 5, 0, 50)]

        self.testedArguments = testedArguments
        self.timeToRun = timeToRun
        self.startTime = perf_counter()
        self.testsRun = 0
        # list of arguments, that will be tested
        self.args = Arguments()

    def nextRun(self):
        finished_part = (perf_counter() - self.startTime) / self.timeToRun
        if finished_part >= 1:
            return False

        index = int(finished_part * len(self.testedArguments))

        if self.args != self.testedArguments[index]:
            self.args = self.testedArguments[index]
            print(f'Status: {int(finished_part * 100)}% done. {self.testsRun} tests have run. ' +
                  f'Testing: ./proj2 {self.args.NE} {self.args.NR} {self.args.TE} {self.args.TR}')

        self.testsRun += 1
        return True

def main():
    parser = argparse.ArgumentParser(description="Tester for IOS project2 2020/2021")
    parser.add_argument("-t", "--time", type=float, default=30,
                        help="how long the test should run in seconds (default: 30)")
    parser.add_argument("-s", "--strict", action="store_true",
                        help="add some extra rules, that should not be necessary")
    parser.add_argument("-T", "--timeout", type=float, default=None,
                        help="set timeout in seconds for detecting deadlock (default: None - no timeout)")
    parser.add_argument("-F", "--full", action="store_true", help="adds a few test cases with more extreme arguments")

    args = parser.parse_args()

    # list of arguments, that will be tested
    testedArguments = [
        Arguments(20, 5, 0, 50),
        Arguments(3, 5, 0, 0),
        Arguments(10, 10, 10, 10),
        Arguments(5, 4, 100, 100),
        Arguments(8, 2, 50, 0),
        Arguments(2, 1, 0, 10)
    ]

    if args.full:
        testedArguments.extend([
            Arguments(999, 19, 0, 0),
            Arguments(100, 10, 10, 200),
            Arguments(80, 15, 100, 0),
            Arguments(999, 1, 0, 10),
            Arguments(2, 19, 0, 100),
        ])

    cont = Controller(testedArguments=testedArguments, timeToRun=args.time)

    # comment this line if you want to see the python exception
    sys.stderr = open('/dev/null', 'w')

    try:
        while cont.nextRun():
            runSubject(cont.args, args.timeout)
            with open('proj2.out', 'r') as file:
                analyzeFile(file, cont.args, strict=args.strict)

    except KeyboardInterrupt:
        print(fmt.YELLOW + fmt.CROSS + ' Test has been cancelled by the user' + fmt.NOCOLOR)
        return 1

    except Exception:
        print(fmt.RED + fmt.CROSS + ' Tests failed' + fmt.NOCOLOR)
        return 1

    print(fmt.GREEN + fmt.TICK + ' Tests passed' + fmt.NOCOLOR)
    return 0

if __name__ == '__main__':
    sys.exit(main())
