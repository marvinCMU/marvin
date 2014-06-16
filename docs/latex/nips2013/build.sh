#!/bin/sh
export BIBINPUTS=./

pdflatex nips2012.tex
bibtex nips2012
pdflatex nips2012.tex
pdflatex nips2012.tex

