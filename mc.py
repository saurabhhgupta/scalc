#! /usr/bin/python

from ctypes import CDLL, c_char_p, byref

testlib = CDLL('/home/mpaul/projects/scripts/calculator/libparseeqn.so')
calcMem = testlib.memoryInit()

answerString = c_char_p()

value = c_char_p('value=7.2')
eqn = c_char_p('4+5*value')
print eqn.value
print answerString.value
testlib.parseEquation(value,byref(answerString),calcMem)
print answerString.value
testlib.parseEquation(eqn,byref(answerString),calcMem)
print answerString.value
print "Hi"
