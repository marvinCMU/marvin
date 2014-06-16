#!/bin/sh
export BIBINPUTS=./

pdflatex mmfps.tex
bibtex mmfps
pdflatex mmfps.tex
pdflatex mmfps.tex

