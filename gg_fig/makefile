GG_FIG_SRC = $(wildcard *.tex)
GG_FIG_DST = $(addsuffix .pdf, $(basename $(GG_FIG_SRC)))

.PHONY: gg_fig
gg_fig: $(GG_FIG_DST)

%.dvi: %.tex
	# ../util/tex_correct $<
	../util/gg_tikz_export_correct.sh $<
	latex -halt-on-error -shell-escape $<

%.ps: %.dvi
	dvips $<

%.pdf: %.ps
	ps2pdf $<