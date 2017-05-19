GG_FIG_DIR = gg_fig
FIG_DIR = fig

SRC = $(wildcard *.tex sty/*.tex)
GG_FIG_SRC = $(wildcard $(GG_FIG_DIR)/*.tex)
GG_FIG_DST = $(addsuffix .pdf, $(basename $(GG_FIG_SRC)))


main.pdf: $(SRC) $(GG_FIG_DST)
	pdflatex main.tex




.PHONY: gg_fig
gg_fig: $(GG_FIG_DST)

%.dvi: %.tex
	util/tex_correct $<
	latex -halt-on-error -shell-escape -output-directory=gg_fig $<

%.ps: %.dvi
	dvips -o $(addsuffix .ps, $(basename $<)) $<

%.pdf: %.ps
	ps2pdf $< $(addsuffix .pdf, $(basename $<))