#!/bin/sh

(which qtchooser > /dev/null) && qtchooser -run-tool=qmake -qt=5 && make
(which qtchooser > /dev/null) || (qmake && make)

