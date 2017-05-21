GG_FIG_SRC = $(wildcard *.tex)
GG_FIG_DST = $(addsuffix .pdf, $(basename $(GG_FIG_SRC)))

.PHONY: gg_fig
gg_fig: $(GG_FIG_DST)

%.dvi: %.tex
	../util/tex_correct $<
	latex -halt-on-error -shell-escape $<

%.ps: %.dvi
	dvips $<

%.pdf: %.ps
	ps2pdf $<